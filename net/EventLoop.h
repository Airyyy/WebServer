//
// Created by airy on 2022/1/3.
//

#ifndef WEB_SERVER_EVENTLOOP_H
#define WEB_SERVER_EVENTLOOP_H
#include "./../base/common.h"
#include "Epoll.h"
#include "Channel.h"
#include "Timer.h"
#include "./../base/MutexLock.h"
#include <bits/stdc++.h>
#include <unistd.h>

class EventLoop : public BaseThreadEvent{
public:
    typedef std::shared_ptr<Channel> SPChannel;
    EventLoop();
    ~EventLoop();
    void threadFunc(const std::string& msg);

    void addToEpoll(const SPChannel& channel);
    void addToWaitingQueue(SPChannel channel);
    void addToTimer(SPChannel channel, time_t expire_time);
    void dealWaitingTask();
    void dealExpiredTask();
    void loop();

    void getInfo();
private:
    void wakeup();
private:
    bool m_running;
    Epoll m_epoll;
    int m_wakeup_fd;
    std::vector<SPChannel> m_waiting_queue;
    TimeHeap<SPChannel> m_time_heap;
    Mutex m_mutex;
    std::shared_ptr<Channel> m_wakeup_channel;
};

#endif //WEB_SERVER_EVENTLOOP_H
