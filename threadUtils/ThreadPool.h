//
// Created by airy on 2021/12/31.
//
#ifndef WEB_SERVER_THREADPOOL_H
#define WEB_SERVER_THREADPOOL_H
#include "Thread.h"
#include <memory>
#include <vector>

template<typename ThreadEvent>
class ThreadPool {
public:
    typedef std::unique_ptr<Thread<ThreadEvent>> ValueType;
    // typedef Thread<ThreadEvent> ValueType;
    explicit ThreadPool(int thread_nums)
        : m_thread_nums(thread_nums),
          m_curr_idx(0) {
    m_threads.reserve(thread_nums);
}

    void start();
    ValueType& getNextThread();
    int getCurrThreadIdx() const;

    void getInfo();

private:
    int m_thread_nums;
    int m_curr_idx;
    std::vector<ValueType> m_threads;
};

template <typename ThreadEvent>
void ThreadPool<ThreadEvent>::start() {
    m_threads.reserve(m_thread_nums);
    for (int i = 0; i < m_thread_nums; ++i) {
        m_threads.emplace_back(std::unique_ptr<Thread<ThreadEvent>>(new Thread<ThreadEvent>(i)));
        // m_threads.push_back(Thread<ThreadEvent>(i));
    }

    for (int i = 0; i < m_thread_nums; ++i) {
        m_threads[i]->start();
    }
}

template <typename ThreadEvent>
std::unique_ptr<Thread<ThreadEvent>>& ThreadPool<ThreadEvent>::getNextThread() {
    std::unique_ptr<Thread<ThreadEvent>>& next = m_threads[m_curr_idx];
    m_curr_idx = (m_curr_idx + 1) % m_thread_nums;
    // printf("curr thread: %d\n", m_curr_idx);
    return next;
}

template <typename ThreadEvent>
int ThreadPool<ThreadEvent>::getCurrThreadIdx() const {
    return m_curr_idx;
}

//template <typename ThreadEvent>
//ThreadEvent* ThreadPool<ThreadEvent>::getNextThread() {
//    ThreadEvent* next = m_threads[m_curr_idx].getEvent();
//    m_curr_idx = (m_curr_idx + 1) % m_thread_nums;
//    return next;
//}

template <typename ThreadEvent>
void ThreadPool<ThreadEvent>::getInfo() {
    for (int i = 0; i < m_thread_nums; ++i) {
        printf("Thread %d: ", i);
        m_threads[i]->getEvent()->getInfo();
    }
}

#endif  // WEB_SERVER_THREADPOOL_H
