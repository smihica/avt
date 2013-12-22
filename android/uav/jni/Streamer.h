#ifndef STREAMER_H_
#define STREAMER_H_

namespace uav {
    class Encoder;
    class Recorder;
    class StreamingServer;

    class Streamer {
    public:

        Streamer() : buff_encoded_num(5), target_buffer_idx(0), streaming(false) {};
        ~Streamer() {};

        void init(int port,
                  void (*on_recorded)(short* buf, unsigned long len, void* context),
                  void (*on_user)(int user_num));

        void exit();
        void on_recorded(short* buf, unsigned long len, void* context);
        void on_user(int user_num);

    private:
        void startRecording();
        void stopRecording();

        Encoder*           encoder;
        Recorder*          recorder;
        StreamingServer*   server;
        unsigned char**    buff_encoded;
        unsigned long      buff_encoded_capacity;
        unsigned char      buff_encoded_num;
        unsigned char      target_buffer_idx;
        bool               streaming;
   };
}

#endif
