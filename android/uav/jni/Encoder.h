#ifndef ENCODER_H_
#define ENCODER_H_

#include "libmp3lame/lame.h"

namespace uav {
    class Encoder {
    public:
        Encoder() : initialized(false) {};
        ~Encoder();
        void init();
        int encode(short* pcm_src, unsigned long pcm_frames, unsigned char* mp3_dest, unsigned long mp3_size);
    private:
        lame_t lame;
        bool initialized;
    };
}

#endif
