//
// Created by airy on 2022/1/3.
//

#include "EventLoop.h"
#include "./../log/Logging.h"
#include "socketUtils.h"
#include <sys/eventfd.h>

typedef std::shared_ptr<Channel> SPChannel;

int createEventFd() {
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0) {
        // LOG << "Failed in create event fd";
        abort();
    }
    return event_fd;
}

void EventLoop::getInfo() {
    m_epoll.getInfo();
    m_time_heap.getInfo();
    printf("EventLoop::getInfo(): waiting size: %d, time_heap size: %d\n", m_waiting_queue.size(), m_time_heap.size());
}

EventLoop::EventLoop()
    : m_running(false),
      m_epoll(),
      m_wakeup_fd(createEventFd()),
      m_waiting_queue(),
      m_time_heap(),
      m_mutex(),
      m_wakeup_channel(new Channel(m_wakeup_fd)) {
    m_wakeup_channel->setEvents(EPOLLIN | EPOLLET);
    m_wakeup_channel->setReadHandle([&]()->CHANNEL_HANDLE_RESULT{
        uint64_t one = 1;
        ssize_t n = readAll(m_wakeup_fd, (char*)&one, sizeof(one));
        if (n != sizeof(one)) {
            // LOG << "EventLoop::handleRead() reads " << (int)n << " bytes instead of 8";
            // printf("EventLoop::handleRead() reads %ld bytes instead of 8 and one is: %d\n", n, one);
        }
        return CHANNEL_NORMAL;
    });

    m_epoll.addToEpoll(m_wakeup_channel);
}

EventLoop::~EventLoop() = default;

void EventLoop::threadFunc(const std::string &msg) {
    // printf("%s EventLoop::threadFunc()\n", msg.c_str());
    this->loop();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    // printf("sizeof one: %d\n", sizeof(one));
    ssize_t n = writeAll(m_wakeup_fd, (char*)(&one), sizeof(one));
    if (n != sizeof one) {
        // LOG << "EventLoop::wakeup() writes " << (int)n << " bytes instead of 8";
    }
}

void EventLoop::addToEpoll(const SPChannel& channel) {
    m_epoll.addToEpoll(channel);
}

void EventLoop::addToWaitingQueue(SPChannel channel) {
    // printf("EventLoop::addToWaitingQueue(): lock\n");
    {
        LockGuard locker(m_mutex);
        m_waiting_queue.emplace_back(std::move(channel));
    }
    wakeup();
}

void EventLoop::addToTimer(SPChannel channel, time_t expire_time) {
    LockGuard locker(m_mutex);
    m_time_heap.push(channel, expire_time);
}

void EventLoop::dealWaitingTask() {
    std::vector<SPChannel> temp;
    {
        LockGuard locker(m_mutex);
        temp.swap(m_waiting_queue);
    }

    for (auto& task : temp) {
        CHANNEL_HANDLE_RESULT res = task->handleWait();
        if (res == CHANNEL_ADD) {
            m_epoll.addToEpoll(task);
            if (task->getExpireTime() != 0) {
                m_time_heap.push(task, task->getExpireTime());
            }
        }
    }
}

// TODO - make TimeHeap a member of Server and use a single thread to deal with expired tasks.
void EventLoop::dealExpiredTask() {
    std::vector<SPChannel> expired_tasks = m_time_heap.handleExpiredTask();
//    if (!expired_tasks.empty()) {
//        printf("EventLoop::dealExpiredTask() size: %ld remain: %ld\n", expired_tasks.size(), m_time_heap.size());
//    }
    for (auto& task : expired_tasks) {
        m_epoll.deleteFromEpoll(task);
        task->handleClose();
    }
}

void EventLoop::loop() {
    m_running = true;

    while (m_running) {
        std::vector<SPChannel> tasks = m_epoll.poll();
        if (!tasks.empty()) {
            for (auto& task : tasks) {
                CHANNEL_HANDLE_RESULT handle_res = task->handleEvent();
                switch (handle_res) {
                    case CHANNEL_DEL: {
                        // printf("EventLoop::loop(): received\n");
                        m_epoll.deleteFromEpoll(task);
                        // printf("EventLoop::loop(): delete function called.\n");
                        break;
                    }
                    case CHANNEL_EDIT: {
                        m_epoll.editEpoll(task);
                        // if (task->isLongLink() && (task->getExpireTime() != 0))
                        if (task->getExpireTime() != 0)
                            m_time_heap.push(task, task->getExpireTime());
                        // printf("EventLoop::loop(): edit function called.\n");
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }

        dealWaitingTask();
        dealExpiredTask();
    }
}
