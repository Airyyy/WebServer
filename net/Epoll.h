//
// Created by airy on 2022/1/3.
//

#ifndef WEB_SERVER_EPOLL_H
#define WEB_SERVER_EPOLL_H
#include "Channel.h"
#include "socketUtils.h"
#include <sys/epoll.h>
#include <bits/stdc++.h>
#include <unistd.h>

const int MAX_FD_SIZE = 100000;
const int MAX_EVENT_SIZE = 4096;
const int EPOLL_TIME_OUT = 100000;

class Epoll {
public:
    typedef std::shared_ptr<Channel> SPChannel;
    explicit Epoll();
    ~Epoll();
    void addToEpoll(const SPChannel& channel);
    void editEpoll(const SPChannel& channel);
    void deleteFromEpoll(const SPChannel& channel);

    std::vector<SPChannel> poll();

    void getInfo();
private:
    int m_epoll_fd;
    SPChannel m_channels[MAX_FD_SIZE];
    std::vector<epoll_event> m_events;

};

#endif //WEB_SERVER_EPOLL_H
