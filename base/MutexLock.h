//
// Created by airy on 2021/12/30.
//
#ifndef WEB_SERVER_MUTEXLOCK_H
#define WEB_SERVER_MUTEXLOCK_H
#include "common.h"
#include <pthread.h>

class Mutex : public NoCopyable {
public:
    explicit Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    Mutex(Mutex&& other)  noexcept : m_mutex(other.m_mutex){ };

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t* get() {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

class LockGuard : public NoCopyable{
public:
    explicit LockGuard(Mutex& locker) : m_locker(locker) {
        // printf("LockGuard::LockGuard()\n");
        m_locker.lock();
    }
    ~LockGuard() {
        // printf("LockGuard::~LockGuard()\n");
        m_locker.unlock();
    }
private:
    Mutex& m_locker;
};

#endif //WEB_SERVER_MUTEXLOCK_H