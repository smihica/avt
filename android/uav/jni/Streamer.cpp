#include <stdlib.h>
#include "Streamer.h"
#include "Encoder.h"
#include "Recorder.h"
#include "StreamingServer.h"
#include "log.h"

using namespace uav;

void Streamer::init(
    int port,
    void (*on_recorded)(short* buf, unsigned long len, void* context),
    void (*on_user)(int user_num))
{
    recorder = new Recorder(on_recorded);
    PLOG("created recorder.\n");
    recorder->init();
    PLOG("initialized recorder.\n");
    buff_encoded_capacity = recorder->recorderBufferSize;
    encoder = new Encoder();
    encoder->init();
    buff_encoded = (unsigned char**)malloc(buff_encoded_num * sizeof(unsigned char*));
    for (int i=0; i < buff_encoded_num; i++) {
        (*(buff_encoded+i)) = (unsigned char*)malloc(buff_encoded_capacity * sizeof(unsigned char));
    }
    server = new StreamingServer();
    server->open(port, on_user, buff_encoded, buff_encoded_num, buff_encoded_capacity);
}

void Streamer::exit()
{
    if (streaming) stopRecording();
    server->close();
    for (int i=0; i < buff_encoded_num; i++) {
        free(*(buff_encoded+i));
    }
    free(buff_encoded);
    delete encoder;
    delete recorder;
}

void Streamer::on_recorded(short* buf, unsigned long frames, void* context)
{
    if (!streaming) return;
    int buff_encoded_size = encoder->encode(buf, frames, buff_encoded[target_buffer_idx], buff_encoded_capacity);
    // PLOG("frames: %lu   buff capacity: %lu   encoded size: %d\n", frames, buff_encoded_capacity, buff_encoded_size);
    server->filledBuffer(target_buffer_idx, buff_encoded_size);
    target_buffer_idx = (target_buffer_idx+1) % buff_encoded_num;
}

void Streamer::on_user(int user_num)
{
    if (0 < user_num) {
        if (!streaming) startRecording();
    } else {
        if (streaming) stopRecording();
    }
}

void Streamer::startRecording()
{
    if (streaming) return;
    streaming = true;
    recorder->startRecording();
    PLOG("start.\n");
}

void Streamer::stopRecording()
{
    if (!streaming) return;
    streaming = false;
    recorder->stopRecording();
    PLOG("stop.\n");
}
