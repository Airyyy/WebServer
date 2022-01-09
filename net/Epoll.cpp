//
// Created by airy on 2022/1/3.
//

#include "Epoll.h"

typedef std::shared_ptr<Channel> SPChannel;

void Epoll::getInfo() {
    int cnt = 0;
    for (auto & m_channel : m_channels) {
        if (m_channel) ++cnt;
    }
    printf("Epoll::getInfo(): remain channels: %d\n", cnt);
}

Epoll::Epoll()
    : m_epoll_fd(epoll_create(MAX_EVENT_SIZE)),
      m_events(MAX_EVENT_SIZE){ }

Epoll::~Epoll() {
    if (m_epoll_fd != -1) {
        close(m_epoll_fd);
        m_epoll_fd = -1;
    }
}

void Epoll::addToEpoll(const SPChannel& channel) {
    int fd = channel->getFd();
//    already set in Server.
//    setNoBlocking(fd);
//    setNoDelay(fd);
    epoll_event event{};
    event.events = channel->getEvent();
    event.events |= EPOLLET;  // use ET mode by default
    event.data.fd = fd;

    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);
    m_channels[fd] = channel;
}

void Epoll::editEpoll(const SPChannel& channel) {
    int fd = channel->getFd();
    setNoBlocking(fd);
    setNoDelay(fd);
    epoll_event event{};
    event.events = channel->getEvent();
    event.events |= (EPOLLET|EPOLLONESHOT);  // use ET mode by default
    event.data.fd = fd;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void Epoll::deleteFromEpoll(const SPChannel& channel) {
    int fd = channel->getFd();

    if (fd == -1 || !m_channels[fd]) return;
//    printf("Delete fd: %d\n", fd);
//    printf("m_channels[fd]: use_count: %d\n", m_channels[fd].use_count());
    epoll_event event{};
    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &event);
    m_channels[fd].reset();
}

std::vector<SPChannel> Epoll::poll() {
    int event_num = epoll_wait(m_epoll_fd, &*m_events.begin(), m_events.size(), EPOLL_TIME_OUT);
    // printf("Epoll: %d - event_num: %d\n", m_epoll_fd, event_num);
    std::vector<SPChannel> res;
    if (event_num > 0) res.resize(event_num);
    for (int i = 0; i < event_num; ++i) {
        epoll_event event = m_events[i];
        res[i] = m_channels[event.data.fd];
        res[i]->setRevents(event.events);
    }
    return res;
}
