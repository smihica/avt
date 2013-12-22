#ifndef RECORDER_H_
#define RECORDER_H_

#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>

// for native audio
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

// for native asset manager
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace uav {
    class Recorder {

    private:
        // engine interfaces
        SLObjectItf engineObject;
        SLEngineItf engineEngine;

        // recorder interfaces
        //SLObjectItf recorderObject;
        //SLRecordItf recorderRecord;
        //SLAndroidSimpleBufferQueueItf recorderBufferQueue;

        // recorder sources
        uint32_t        recorderSamplingRate;
        uint32_t        recorderBitDepth;
        SLmilliHertz    recorderSR;

    public:
        Recorder(
            void (*callbackfn)(short* buf, unsigned long len, void* context),
            uint32_t sampling_rate=SL_SAMPLINGRATE_44_1,
            uint16_t bit_depth=SL_PCMSAMPLEFORMAT_FIXED_16,
            unsigned long flames=2048); // 1024

        ~Recorder();

        void init();
        void startRecording();
        void stopRecording();

        short*          recorderBufferPair[2];
        uint32_t*       recorderBufferReal;
        int             nextBufferIndex;
        unsigned long   recorderFrames;
        unsigned long   recorderBufferSize;

        SLObjectItf recorderObject;
        SLRecordItf recorderRecord;
        SLAndroidSimpleBufferQueueItf recorderBufferQueue;

        void (*recorderCallback)(short* buf, unsigned long len, void* context);

    };
}


#endif // RECORDER_H_
