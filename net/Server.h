//
// Created by airy on 2022/1/4.
//

#ifndef WEB_SERVER_SERVER_H
#define WEB_SERVER_SERVER_H

#include "socketUtils.h"
#include "EventLoop.h"
#include "./../threadUtils/ThreadPool.h"
#include "./../log/Logging.h"

#include <unistd.h>

const int MAX_CONNECTION_NUM = 100000;

class Server {
public:
    Server(int port, int thread_num);
    ~Server();
    void start();

    void getInfo();
private:
    CHANNEL_HANDLE_RESULT handleNewConnection();
private:
    int m_listen_fd;
    bool m_start;
    EventLoop m_main_loop;
    ThreadPool<EventLoop> m_thread_pool;
};

#endif //WEB_SERVER_SERVER_H
