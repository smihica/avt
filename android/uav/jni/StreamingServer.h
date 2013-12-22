#ifndef STREAMING_SERVER_H_
#define STREAMING_SERVER_H_

#include <pthread.h>

namespace uav {
    class StreamingServer {
    public:
        StreamingServer() : user_num(0), current_buff_idx(0), current_buff_len(0), epfd(-1) {};
        ~StreamingServer() {};

        int open(int port,
                 void (*on_user_callback)(int user_num),
                 unsigned char** circular_buffer,
                 unsigned char buffer_num,
                 unsigned long buffer_len);

        int close();

        void filledBuffer(unsigned char buffer_idx, unsigned int size);
        void open_http_server();

    private:
        struct StreamingSock {
            StreamingSock(int fd, int s = 0, unsigned char cbi = 0, unsigned int cbl = 0)
            { sock = fd; state = s; current_buff_idx = cbi; current_buff_len = cbl; }
            int sock;
            int state;
            unsigned char current_buff_idx;
            unsigned int  current_buff_len;
        };
        enum StreamingSockState {
            READING   = 0,
            NOT_FOUND = 1,
            BEFORE_STREAMING = 2,
            STREAMING = 3,
        };

        void do_accept(int fd);
        void do_read(StreamingSock* ss);
        void do_write(StreamingSock* ss, unsigned char cbi, unsigned int cbl);
        void report_user_adding();
        void report_user_removing();

        void (*on_user)(int user_num);
        unsigned char** c_buff;
        unsigned char   c_buff_num;
        unsigned long   c_buff_size;
        int listening_socket;
        int accepted_socket;
        pthread_t server_thread;
        pthread_mutex_t server_thread_mutex;
        int port;
        unsigned int user_num;

        unsigned char current_buff_idx;
        unsigned int  current_buff_len;

        int epfd;
    };
}

#endif
