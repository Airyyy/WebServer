//
// Created by airy on 2022/1/3.
//

#ifndef WEB_SERVER_CHANNEL_H
#define WEB_SERVER_CHANNEL_H

#include "HttpEvent.h"
#include "Timer.h"
#include <bits/stdc++.h>
#include <functional>
#include <unistd.h>
#include <sys/epoll.h>
const time_t LONG_LINK_EXPIRE_TIME = 60;  // seconds
const time_t SHORT_LINK_EXPIRE_TIME = 2;
enum CHANNEL_HANDLE_RESULT { CHANNEL_NORMAL = 0, CHANNEL_ADD, CHANNEL_EDIT, CHANNEL_DEL };

class Channel {
public:
    static int channel_cnt;
    static int channel_destroy;
    static Mutex channel_mutex;
    typedef std::function<CHANNEL_HANDLE_RESULT()> Callback;
    typedef std::shared_ptr<Channel> SPChannel;

    explicit Channel(int fd);
    Channel(int fd, std::shared_ptr<HttpEvent> http_event);
    ~Channel();

    int getFd() const;
    int getKey() const;
    bool isLongLink() const;
    time_t getExpireTime() const;
    __uint32_t getEvent() const;
    __uint32_t getRevent() const;
    void setEvents(__uint32_t events);
    void setRevents(__uint32_t revents);

    CHANNEL_HANDLE_RESULT handleEvent();
    CHANNEL_HANDLE_RESULT handleWait();
    CHANNEL_HANDLE_RESULT handleClose();

    void setReadHandle(Callback&& callback);
    void setWriteHandle(Callback&& callback);
    void setErrorHandle(Callback&& callback);

    void setConnectionHandle(Callback&& callback);
    void setCloseHandle(Callback&& callback);

    void linkTimer(const std::shared_ptr<TimeNode<SPChannel>>& timer);
    void seperateTimer();

private:
    int m_fd;
    int m_key;
    bool m_fd_in_use;
    __uint32_t m_events;
    __uint32_t m_revents;  // after epoll_wait, channel has to deal m_revents

    bool m_long_link;
    time_t m_expire_time;

    std::shared_ptr<HttpEvent> m_http_event;
    std::weak_ptr<TimeNode<SPChannel>> m_timer;

    // TODO - bind this callback with bind(Epoll::func, epollInstance) ?
    Callback m_handle_connection;  // return CHANNEL_ADD or CHANNEL_EDIT or CHANNEL_DEL
    Callback m_handle_read;  // return CHANNEL_DEL or CHANNEL_NORMAL
    Callback m_handle_write;  // return CHANNEL_DEL or CHANNEL_NORMAL
    Callback m_handle_close;  // return CHANNEL_DEL
    Callback m_handle_error;  // return CHANNEL_DEL

};

#endif //WEB_SERVER_CHANNEL_H
