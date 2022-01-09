//
// Created by airy on 2021/12/30.
//
/**
 * manage thread.
 */
#ifndef WEB_SERVER_THREAD_H
#define WEB_SERVER_THREAD_H
#include <pthread.h>
#include <functional>
#include <memory>
#include <sys/syscall.h>

template <typename Event>
class Thread {
public:
    Thread();
    explicit Thread(Event* event);
    explicit Thread(int idx);
    Thread(int idx, Event* event);
    ~Thread();

    // call Event.threadFunc()
    void start();
    void join();
    void detach();

    std::unique_ptr<Event>& operator->() {
        return m_event;
    }
//    Event* operator->() {
//        return m_event;
//    }

    std::unique_ptr<Event>& operator*() {
        return m_event;
    }
//    Event& operator*() {
//        return m_event;
//    }

    inline int getTid() {
        return m_tid;
    }

    inline int getIdx() {
        return m_idx;
    }

    std::unique_ptr<Event>& getEvent() {
        return m_event;
    }

    template <typename __Event>
    friend void* threadFunc(void*);

private:
    bool m_started;  // show if thread has started
    bool m_joinable;  // show if thread has joined or detached
    int m_idx;  // index of thread in thread pool
    pthread_t m_tid;  // thread id managed by Thread
    std::unique_ptr<Event> m_event; // a class that has function threadFunc()
    // Event m_event;
};

template <typename Event>
void* threadFunc(void* arg) {
    pid_t curr_pid = syscall(SYS_gettid);
    auto* _thread = static_cast<Thread<Event>*>(arg);
    std::string msg("Thread idx: " + std::to_string(_thread->getIdx()));
    _thread->m_event->threadFunc(msg);
    return nullptr;
}

template <typename Event>
Thread<Event>::Thread()
    : m_started(false), m_joinable(true), m_idx(0), m_tid(0){
    m_event = std::unique_ptr<Event>(new Event());
}

template <typename Event>
Thread<Event>::Thread(int idx)
    : m_started(false), m_joinable(true), m_idx(idx), m_tid(0){
    m_event = std::unique_ptr<Event>(new Event());
}

template <typename Event>
Thread<Event>::Thread(Event* event)
    : m_started(false), m_joinable(true), m_idx(0), m_tid(0), m_event(event){ }

template <typename Event>
Thread<Event>::Thread(int idx, Event* event)
        : m_started(false), m_joinable(true), m_idx(idx), m_tid(0), m_event(event) { }

template <typename Event>
Thread<Event>::~Thread() {
    // std::cout << "Thread::~Thread()" << std::endl;
    if (m_started && m_joinable) {
        // std::cout << "Thread::~Thread(): not joined" << std::endl;
        pthread_join(m_tid, nullptr);
    }
}

template <typename Event>
void Thread<Event>::join() {
    if (m_started && m_joinable) {
        pthread_join(m_tid, nullptr);
        m_joinable = false;
    }
}

template <typename Event>
void Thread<Event>::detach() {
    if (m_started && m_joinable) {
        pthread_detach(m_tid);
        m_joinable = false;
    }
}

template <typename Event>
void Thread<Event>::start() {
    pthread_create(&m_tid, nullptr, threadFunc<Event>, (void*)this);
    m_started = true;
}

#endif  // WEB_SERVER_THREAD_H
