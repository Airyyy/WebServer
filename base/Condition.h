//
// Created by airy on 2021/12/30.
//
#ifndef WEB_SERVER_CONDITION_H
#define WEB_SERVER_CONDITION_H
#include "common.h"
#include "MutexLock.h"
#include <pthread.h>

class Condition : public NoCopyable {
public:
    explicit Condition(Mutex& mutex) : m_mutex(mutex){
        // linux implementation supports no attributions for conditions
        pthread_cond_init(&m_cond, nullptr);
    }

    Condition(Condition&& other)  noexcept : m_mutex(other.m_mutex), m_cond(other.m_cond) {};

    ~Condition() {
        pthread_cond_destroy(&m_cond);
    }

    void wait() {
        // m_mutex.lock();
        pthread_cond_wait(&m_cond, m_mutex.get());
        // m_mutex.unlock();
    }

    bool waitForSeconds(time_t seconds) {
        timespec abs_time;
        clock_gettime(CLOCK_REALTIME, &abs_time);
        abs_time.tv_sec += seconds;
        // m_mutex.lock();
        return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mutex.get(), &abs_time);
        // m_mutex.unlock();
        // return res == ETIMEDOUT;
    }

    void notify() {
        pthread_cond_signal(&m_cond);
    }

    void notifyAll() {
        pthread_cond_broadcast(&m_cond);
    }
private:
    Mutex& m_mutex;
    pthread_cond_t m_cond;
};

#endif  // WEB_SERVER_CONDITION_H