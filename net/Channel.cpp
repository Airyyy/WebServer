//
// Created by airy on 2022/1/3.
//
#include "Channel.h"

#include <utility>
#include "socketUtils.h"

int Channel::channel_cnt = 0;
int Channel::channel_destroy = 0;
Mutex Channel::channel_mutex;

Channel::Channel(int fd)
    : m_fd(fd),
      m_key(fd),
      m_fd_in_use(true),
      m_events(EPOLLIN|EPOLLET|EPOLLONESHOT),
      m_revents(0),
      m_long_link(false),
      m_expire_time(0),
      m_http_event(nullptr) { }

Channel::Channel(int fd, std::shared_ptr<HttpEvent> http_event)
    : m_fd(fd),
      m_key(fd),
      m_fd_in_use(true),
      m_events(EPOLLIN|EPOLLET|EPOLLONESHOT),
      m_revents(0),
      m_long_link(false),
      m_expire_time(SHORT_LINK_EXPIRE_TIME),
      m_http_event(std::move(http_event)){

    m_handle_read = [&]()->CHANNEL_HANDLE_RESULT {
        HTTP_HANDLE_RESULT http_handle_res = m_http_event->handleRead();
        m_long_link = m_http_event->isLongLink();
        if (m_long_link) {
            m_expire_time = LONG_LINK_EXPIRE_TIME;
        }
        // else return this->handleClose();
        // printf("%s\n", m_long_link ? "Long Link" : "Not Long Link");

        switch (http_handle_res) {
            case HTTP_NORMAL: {
                if (m_long_link) {
                    return CHANNEL_EDIT;
                } else {
                    return this->handleClose();
                    // return CHANNEL_DEL;
                }
            }
            case HTTP_READ_AGAIN: {
                // short-link can also be handled once more.
                return CHANNEL_EDIT;
            }
            case HTTP_WRITE_AGAIN: {
                m_events |= EPOLLOUT;
                return CHANNEL_EDIT;
            }
            case HTTP_ERROR: {
                // if (m_long_link) return CHANNEL_EDIT;
                return this->handleClose();
            }
            case HTTP_CLOSE: {
                return this->handleClose();
            }
            default:
                return this->handleClose();
        }
    };
    m_handle_write = [&]()->CHANNEL_HANDLE_RESULT {
        HTTP_HANDLE_RESULT http_handle_res = m_http_event->handleWrite();
        switch (http_handle_res) {
            case HTTP_NORMAL: {
                if (m_long_link) {
                    m_events = EPOLLIN|EPOLLET|EPOLLONESHOT;  // do not monitor EPOLLOUT event next time.
                    return CHANNEL_EDIT;
                } else {
                    return handleClose();
                }
            }
            case HTTP_WRITE_AGAIN: {
                m_events |= EPOLLOUT;
                return CHANNEL_EDIT;
            }
            case HTTP_CLOSE: {
                return handleClose();
            }
            default:
                return handleClose();
        }
    };

    LockGuard locker(channel_mutex);
    ++channel_cnt;
}

Channel::~Channel() {
    {
        LockGuard locker(channel_mutex);
        ++channel_destroy;
    }
    handleClose();
}

int Channel::getFd() const { return m_fd; }
int Channel::getKey() const { return m_key; }

__uint32_t Channel::getEvent() const { return m_events; }
__uint32_t Channel::getRevent() const { return m_revents; }
void Channel::setEvents(__uint32_t events) { m_events = events; }
void Channel::setRevents(__uint32_t revents) { m_revents = revents; }

void Channel::setReadHandle(Callback&& callback) { m_handle_read = callback; }
void Channel::setWriteHandle(Callback&& callback) { m_handle_write = callback; }
void Channel::setErrorHandle(Callback&& callback) { m_handle_error = callback; }

void Channel::setCloseHandle(Callback&& callback) { m_handle_close = callback; }
void Channel::setConnectionHandle(Callback&& callback) { m_handle_connection = callback; }

CHANNEL_HANDLE_RESULT Channel::handleEvent() {
    // printf("Channel::handleEvent()\n");
    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
        // means connection pair closed this connection.
        // printf("Channel: %ld Channel::handleEvent(): connection pair closed connection.\n", this->getFd());
        return handleClose();
    }

    if (m_revents & EPOLLERR) {
        // printf("Channel: %ld Channel::handleEvent(): error happens, maybe pair disconnected without notifying.\n", this->getFd());
        return handleClose();
    }

    if (m_revents & (EPOLLIN | EPOLLRDHUP)) {
        // printf("Channel: %ld is Channel::handleEvent::read\n", this->getFd());
        if (m_handle_read) {
            CHANNEL_HANDLE_RESULT read_res = m_handle_read();
            if (read_res != CHANNEL_NORMAL) return read_res;
        }
    }
    if (m_revents & EPOLLOUT) {
        // printf("Channel: %ld is Channel::handleEvent::write\n", this->getFd());
        if (m_handle_write) {
            CHANNEL_HANDLE_RESULT write_res = m_handle_write();
            if (write_res != CHANNEL_NORMAL) return write_res;
        }
    }
    // printf("Channel::handleEvent() end.\n");
    return CHANNEL_NORMAL;
}

CHANNEL_HANDLE_RESULT Channel::handleWait() {
    if (m_handle_connection) {
        return m_handle_connection();  // m_handle_connection may return CHANNEL_ADD;
    }

    return CHANNEL_ADD;
}

// always return CHANNEL_DEL
CHANNEL_HANDLE_RESULT Channel::handleClose() {
    // printf("%d, Channel::handleClose()\n", this->getFd());
    if (m_handle_close) {
        m_handle_close();
    }
    seperateTimer();
    if (m_fd_in_use) {
        close(m_fd);
        m_fd_in_use = false;
    }
    return CHANNEL_DEL;
}

bool Channel::isLongLink() const {
    return m_long_link;
}

time_t Channel::getExpireTime() const {
    return m_expire_time;
}

void Channel::linkTimer(const std::shared_ptr<TimeNode<SPChannel>>& timer) {
    m_timer = timer;
}

void Channel::seperateTimer() {
    if (m_timer.lock()) {
        std::shared_ptr<TimeNode<SPChannel>> timer(m_timer.lock());
        timer->clearTask();
        m_timer.reset();
    }
}
