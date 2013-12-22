#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <time.h>
#include <sys/epoll.h>

#include "StreamingServer.h"
#include "log.h"

using namespace uav;

static const char *DAY_NAMES[] =
{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *MONTH_NAMES[] =
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

char* RFC1123_DateTimeNow()
{
    const int RFC1123_TIME_LEN = 29;
    time_t t;
    struct tm tm;
    char* buf = (char*)malloc(RFC1123_TIME_LEN+1);

    time(&t);
    gmtime_r(&t, &tm);

    strftime(buf, RFC1123_TIME_LEN+1, "---, %d --- %Y %H:%M:%S GMT", &tm);
    memcpy(buf, DAY_NAMES[tm.tm_wday], 3);
    memcpy(buf+8, MONTH_NAMES[tm.tm_mon], 3);

    return buf;
}

void gen_random_str(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

#define RES_HEAD_OK_FMT                                                 \
    "HTTP/1.1 200 OK\r\n"                                               \
    "Server: uav:StreamingServer.cpp v0.0.1\r\n"                        \
    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
    "Pragma: no-cache\r\n"                                              \
    "Expires: -1\r\n"                                                   \
    "Access-Control-Allow-Origin: *\r\n"                                \
    "Content-Type: application/octet-stream\r\n"                        \
    "Date: %s\r\n"                                                      \
    "Transfer-Encoding: chunked\r\n"                                    \
    "\r\n"

#define RES_NOT_FOUND                                                   \
    "HTTP/1.1 404 NotFound\r\n"                                         \
    "Content-Type: text/html\r\n"                                       \
    "Connection: close\r\n"                                             \
    "Content-Length: 38\r\n\r\n<html><body>404 NotFound</body></html>"

#define STREAMING_PATH "/stream.mp3"
#define MAX_EVENTS 20

void* open_http_server_static(void* inst) {
    StreamingServer* instance = (StreamingServer*)inst;
    instance->open_http_server();
    return NULL;
}

int StreamingServer::open(
    int _port,
    void (*on_user_callback)(int user_num),
    unsigned char** circular_buffer,
    unsigned char buffer_num,
    unsigned long buffer_size)
{
    srand((unsigned)time(NULL));

    on_user     = on_user_callback;
    c_buff      = circular_buffer;
    c_buff_num  = buffer_num;
    c_buff_size = buffer_size;
    port = _port;

    pthread_mutex_init(&server_thread_mutex, NULL);
    pthread_create(&server_thread, NULL, open_http_server_static, (void*)this);

    return 0;
}

// !!! This method will be run by other thread.
void StreamingServer::open_http_server()
{
    int i = 1;
    struct sockaddr_in my_sin;
    struct epoll_event ev;

    memset(&ev, 0, sizeof(ev));

    if((epfd = epoll_create(MAX_EVENTS)) < 0) {
        PLOG("epoll_create(): errno=%d\n", errno);
        goto error;
    }

    if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        PLOG("socket: errno=%d\n", errno);
        goto error;
    }

    memset(&my_sin, 0, sizeof(my_sin));
    my_sin.sin_family      = AF_INET;
    my_sin.sin_port        = htons(port);
    my_sin.sin_addr.s_addr = INADDR_ANY;
    setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i));

    if (bind(listening_socket, (struct sockaddr*)&my_sin, sizeof(my_sin)) < 0) {
        PLOG("bind: errno=%d\n", errno);
        goto error;
    }

    PLOG("HTTP server listen port %d", port);

    if (listen(listening_socket, 10) < 0) {
        PLOG("listen: errno=%d\n", errno);
        goto error;
    }

    {
        StreamingSock* ss = new StreamingSock(listening_socket);
        ev.events  = EPOLLIN | EPOLLHUP | EPOLLERR; // | EPOLLRDHUP;
        ev.data.ptr = (void*)ss;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, listening_socket, &ev) < 0) {
            delete ss;
            PLOG("epoll_ctl(): errno=%d\n", errno);
            goto error;
        }
    }

    struct epoll_event evs[MAX_EVENTS];
    unsigned char cbi;
    unsigned int  cbl;
    while (1) {
        int nfd = epoll_wait(epfd, evs, MAX_EVENTS, 2); // wait 2ms
        if (nfd < 0) {
            PLOG("epoll_wait(epfd): errno=%d", nfd);
            goto error;
        }
        if (nfd == 0) continue; // timeouted continue;

        { // renew pointers of streaming buffer.
            pthread_mutex_lock(&server_thread_mutex);
            cbi = current_buff_idx;
            cbl = current_buff_len;
            pthread_mutex_unlock(&server_thread_mutex);
        }

        // PLOG("nfd: %d", nfd);

        for (int i = 0; i < nfd; i++) {
            StreamingSock* ss = (StreamingSock*)evs[i].data.ptr;
            int sock = ss->sock;
            int events = evs[i].events;
            if (sock == listening_socket) {
                if (// events & EPOLLRDHUP ||
                    events & EPOLLHUP ||
                    events & EPOLLERR) {
                    delete ss;
                    PLOG("Error occured in listening socket.");
                    goto error;
                }
                do_accept(sock);

            } else {
                if (// events & EPOLLRDHUP ||
                    events & EPOLLHUP ||
                    events & EPOLLERR) {
                    PLOG("Error occured or connection closed by peer fd=%d", sock);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
                    shutdown(sock, 2);
                    ::close(sock);
                    if (ss->state == BEFORE_STREAMING ||
                        ss->state == STREAMING) {
                        report_user_removing();
                    }
                    delete ss;

                } else {
                    if (events & EPOLLIN) {
                        PLOG("New request received");
                        do_read(ss);
                    } else if (events & EPOLLOUT) {
                        do_write(ss, cbi, cbl);
                    }
                }
            }
        }
        usleep(1000); // fource wait 1ms
    }

error:
    if (listening_socket != -1) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, listening_socket, NULL);
        ::close(listening_socket);
    }
    if (epfd != -1)
        ::close(epfd);

    return;
}

// !!! This method will be run by other thread.
void StreamingServer::do_accept(int fd)
{
    struct epoll_event ev;
    struct sockaddr_in peer_sin;
    int len = sizeof(peer_sin);
    int sock = accept(fd, (struct sockaddr *)&peer_sin, &len);
    if (sock == -1) {
        PLOG("accept: errno=%d", errno);
        return;
    }
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR; // | EPOLLRDHUP;
    ev.data.ptr = (void*)(new StreamingSock(sock));
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
}

// !!! This method will be run by other thread.
void StreamingServer::do_read(StreamingSock* ss)
{
    int len;
    char buf[512];
    struct epoll_event ev;
    int sock = ss->sock;

    len = recv(sock, buf, sizeof(buf)-1, 0);
    if (len <= 0) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
        shutdown(sock, 2);
        ::close(sock);
        delete ss;
        return;
    }

    buf[len] = '\0';
    ev.events = EPOLLOUT | EPOLLHUP | EPOLLERR; // | EPOLLRDHUP;
    ev.data.ptr = (void*)ss;
    epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev);

    if (memcmp(buf, "GET " STREAMING_PATH, 15) == 0) {
        report_user_adding();
        ss->state = BEFORE_STREAMING;
    } else {
        ss->state = NOT_FOUND;
    }
}

// !!! This method will be run by other thread.
void StreamingServer::do_write(
    StreamingSock* ss,
    unsigned char cbi, unsigned int cbl)
{
    int sock  = ss->sock;
    int state = ss->state;

    switch (state) {
    case NOT_FOUND:
    {
        char res_head[512];
        snprintf(res_head, sizeof(res_head), RES_NOT_FOUND);
        send(sock, res_head, strlen(res_head), 0);
        epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
        shutdown(sock, 2);
        ::close(sock);
        break;
    }
    case BEFORE_STREAMING:
    {
        char res_head[512];
        char buf[32];
        char* timenow = RFC1123_DateTimeNow();
        gen_random_str(buf, 32);
        snprintf(res_head, sizeof(res_head), RES_HEAD_OK_FMT, timenow);
        free(timenow);
        send(sock, res_head, strlen(res_head), 0);

        ss->state = STREAMING;
        // NOT BREAK
    }
    case STREAMING:
    {
        if (ss->current_buff_idx != cbi ||
            ss->current_buff_len != cbl) {

            // PLOG("sending mp3 -- idx: %d len: %d", cbi, cbl);
            send(sock, c_buff[cbi], cbl, 0);

            ss->current_buff_idx = cbi;
            ss->current_buff_len = cbl;
        }
        break;
    }
    }
}

// !!! This method will be run by other thread.
void StreamingServer::report_user_adding()
{
    user_num++;
    PLOG("new user comming !! %d", user_num);
    on_user(user_num);
}

// !!! This method will be run by other thread.
void StreamingServer::report_user_removing()
{
    user_num--;
    PLOG("user removing !! %d", user_num);
    on_user(user_num);
}

int StreamingServer::close()
{
    int res = pthread_kill(server_thread, SIGTERM);
    if (res < 0) {
        PLOG("pthread kill failed errno=%d", errno);
    }
    PLOG("Close.");
    return 0;
}

void StreamingServer::filledBuffer(unsigned char buffer_idx, unsigned int size)
{
    // PLOG("buffer filled called idx: %d size: %d/%lu", buffer_idx, size, c_buff_size);

    pthread_mutex_lock(&server_thread_mutex);
    {
        current_buff_idx = buffer_idx;
        current_buff_len = size;
    }
    pthread_mutex_unlock(&server_thread_mutex);

}
