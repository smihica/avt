#include <algorithm>
#include <unistd.h>
#include "Recorder.h"
#include "log.h"

// to avoid unused warning.
#define _unused(x) ((void)x)

// recorder interfaces

// 5 seconds of recorded audio at 16 kHz mono, 16-bit signed little endian
// pointer and size of the next player buffer to enqueue, and number of remaining buffers

using namespace uav;

// this callback handler is called every time a buffer finishes recording
void recorderCallbackFunction(SLAndroidSimpleBufferQueueItf bq, void* context)
{
    Recorder* recorder = reinterpret_cast<Recorder*>(context);

    // SLAndroidSimpleBufferQueueState state;
    // SLRecordItf r = recorder->recorderRecord;

    SLresult result = (*bq)->Enqueue(
        bq,
        recorder->recorderBufferPair[recorder->nextBufferIndex],
        recorder->recorderBufferSize);
    assert(SL_RESULT_SUCCESS == result);

    short* this_buf = recorder->recorderBufferPair[recorder->nextBufferIndex];

    recorder->recorderCallback(
        this_buf, //recorder->recorderBufferReal
        recorder->recorderFrames,
        context);

    recorder->nextBufferIndex = (!recorder->nextBufferIndex);

    _unused(result);
}

Recorder::Recorder(
    void (*callbackfn)(short* buf, unsigned long len, void* context),
    uint32_t sampling_rate, uint16_t bit_depth, unsigned long frames)
{
    SLresult result;

    recorderCallback = callbackfn;

    engineObject = NULL;
    recorderObject = NULL;

    recorderSamplingRate = sampling_rate;
    recorderBitDepth     = bit_depth;
    recorderFrames       = frames;

    PLOG("samplingRate: %u\nframes: %lu\nbitDepth: %u\n", sampling_rate, recorderFrames, bit_depth);

    recorderBufferSize = sizeof(short)*recorderFrames;
    recorderBufferPair[0] = (short*)malloc(recorderBufferSize);
    recorderBufferPair[1] = (short*)malloc(recorderBufferSize);

    recorderBufferReal = (uint32_t*)malloc(sizeof(uint32_t)*recorderFrames);

    nextBufferIndex = 0;
    memset(recorderBufferPair[0], 0, recorderBufferSize);
    memset(recorderBufferPair[1], 0, recorderBufferSize);

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);

    _unused(result);
}

Recorder::~Recorder()
{
    free(recorderBufferPair[0]);
    free(recorderBufferPair[1]);
    free(recorderBufferReal);
    // destroy audio recorder object, and invalidate all associated interfaces
    if (recorderObject != NULL) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBufferQueue = NULL;
    }
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
}

void Recorder::init()
{
    SLresult result;

    // configure audio source
    SLDataLocator_IODevice loc_dev = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        2
    };

/*
    typedef struct SLDataFormat_PCM_ {
        SLuint32   formatType;
        SLuint32   numChannels;
        SLuint32   samplesPerSec;
        SLuint32   bitsPerSample;
        SLuint32   containerSize;
        SLuint32   channelMask;
        SLuint32   endianness;
    } SLDataFormat_PCM;
*/

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        1,
        //SL_SAMPLINGRATE_44_1,SL_PCMSAMPLEFORMAT_FIXED_16,SL_PCMSAMPLEFORMAT_FIXED_16,
        recorderSamplingRate, //
        SL_PCMSAMPLEFORMAT_FIXED_16, // recorderBitDepth
        SL_PCMSAMPLEFORMAT_FIXED_16, // recorderBitDepth
        SL_SPEAKER_FRONT_CENTER,
        SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioRecorder(
        engineEngine,
        &recorderObject,
        &audioSrc,
        &audioSnk,
        1,
        id,
        req);

    // PLOG("here1");

    assert(SL_RESULT_SUCCESS == result);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // PLOG("here2");

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // PLOG("here3");
    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    // void (*callback)(SLAndroidSimpleBufferQueueItf caller, void *pContext);
    // callback = reinterpret_cast<void (*)(SLAndroidSimpleBufferQueueItf, void*)>(recorderCallback);

    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, recorderCallbackFunction, reinterpret_cast<void*>(this));
    assert(SL_RESULT_SUCCESS == result);

    // PLOG("here4");

    _unused(result);

    return;
}

void Recorder::startRecording()
{
    SLresult result;

    stopRecording();

    nextBufferIndex = 0;
    memset(recorderBufferPair[0], 0, recorderBufferSize);
    memset(recorderBufferPair[1], 0, recorderBufferSize);

    // enqueue an empty buffer to be filled by the recorder
    // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
    result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBufferPair[0], recorderBufferSize);
    assert(SL_RESULT_SUCCESS == result);

    result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBufferPair[1], recorderBufferSize);
    assert(SL_RESULT_SUCCESS == result);

    SLAndroidSimpleBufferQueueState state;
    (*recorderBufferQueue)->GetState(recorderBufferQueue, &state);
    // PLOG("QUEUE_STATE -> count: %u, index: %u", state.count, state.index);

    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
    // which for this code example would indicate a programming error

    // start recording
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
    assert(SL_RESULT_SUCCESS == result);

    _unused(result);
}


void Recorder::stopRecording()
{
    SLresult result;

    // in case already recording, stop recording and clear buffer queue
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    _unused(result);
}
