//
// Created by airy on 2022/1/3.
//

#ifndef WEB_SERVER_TIMER_H
#define WEB_SERVER_TIMER_H
#include <ctime>
#include <sys/time.h>
#include <bits/stdc++.h>

/* Task should be shared_ptr type */
template <typename Task>
class TimeNode {
public:
    TimeNode(Task task, time_t expire_time);
    ~TimeNode();

    void update(time_t time);
    bool expired() const;
    bool expired(time_t now) const;
    time_t getExpireTime() const;
    Task& getTask();
    void clearTask();
    int getKey() const;

private:
    Task m_task;
    int m_key;
    time_t m_expire_time;
};

template <typename Node>
struct TimeCmp {
    bool operator()(Node& a, Node& b) const;
};

template <typename Task>
class TimeHeap {
public:
    // use shared_ptr as node's type, otherwise TimeNode may copy-init several times and call task->handleClose
    // several times when deconstructing.
    typedef std::shared_ptr<TimeNode<Task>> NodeType;
    TimeHeap() = default;
    ~TimeHeap() = default;

    void push(Task& task, time_t expire_time_sec);
    // void pop();
    bool empty();
    size_t size() const;
    void swap(TimeHeap<Task>& other);
    std::vector<Task> handleExpiredTask();

    void getInfo();

private:
    std::priority_queue<NodeType, std::vector<NodeType>> m_time_heap;
    std::map<int, time_t> m_key_expire_time;
};

time_t getCurrTime();

/* ******************************************************************* */
/* ************************** implement ****************************** */
/* ******************************************************************* */
template <typename Node>
bool TimeCmp<Node>::operator()(Node& a, Node& b) const {
    return a->getExpireTime() > b->getExpireTime();
}

template <typename Task>
TimeNode<Task>::TimeNode(Task task, time_t expire_time)
        : m_task(task), m_key(task->getKey()), m_expire_time(expire_time) {
    // m_task->linkTimer(this);
}

template <typename Task>
TimeNode<Task>::~TimeNode() = default;

template <typename Task>
void TimeNode<Task>::update(time_t time) {
    m_expire_time = time;
}

template <typename Task>
void TimeNode<Task>::clearTask() {
    // if (m_task) {
    m_task.reset();
    // }
}

template <typename Task>
int TimeNode<Task>::getKey() const {
    return m_key;
}

template <typename Task>
bool TimeNode<Task>::expired() const{
    time_t now_time = getCurrTime();
    return now_time >= m_expire_time;
}

template <typename Task>
bool TimeNode<Task>::expired(time_t now) const {
    return now >= m_expire_time;
}

template <typename Task>
bool operator<(const std::shared_ptr<TimeNode<Task>>& a, const std::shared_ptr<TimeNode<Task>>& b) {
    return a->getExpireTime() > b->getExpireTime();
}

template <typename Task>
time_t TimeNode<Task>::getExpireTime() const{
    return m_expire_time;
}

template <typename Task>
Task& TimeNode<Task>::getTask() {
    return m_task;
}

template <typename Task>
void TimeHeap<Task>::push(Task& task, time_t expire_time_sec) {
    int key = task->getKey();
    time_t expire_time = getCurrTime() + (expire_time_sec * 1000);
    if (m_key_expire_time.find(key) == m_key_expire_time.end()) {
        std::shared_ptr<TimeNode<Task>> time_node(new TimeNode<Task>(task, expire_time));
        m_time_heap.push(time_node);
        task->linkTimer(time_node);
    }
    m_key_expire_time[key] = expire_time;
}

template <typename Task>
bool TimeHeap<Task>::empty() {
    return m_time_heap.empty();
}

template <typename Task>
size_t TimeHeap<Task>::size() const {
    return m_time_heap.size();
}

template <typename Task>
void TimeHeap<Task>::swap(TimeHeap<Task>& other) {
    this->m_time_heap.swap(other.m_time_heap);
}

template <typename Task>
std::vector<Task> TimeHeap<Task>::handleExpiredTask() {
    std::vector<Task> res;
    time_t now = getCurrTime();  // not very accurate, but reduce some invokes.
    while (!m_time_heap.empty() && m_time_heap.top()->expired(now)) {
        auto& task = m_time_heap.top()->getTask();
        auto time_node = m_time_heap.top();
        int key = time_node->getKey();
        m_time_heap.pop();

        if (task) {  // channel still exists.
            // task->seperateTimer();
            time_t real_expire_time = m_key_expire_time[key];

            if (real_expire_time <= now) {
                res.emplace_back(task);
                m_key_expire_time.erase(key);
            } else {
                time_node->update(real_expire_time);
                // task->linkTimer(time_node);
                m_time_heap.push(time_node);
            }
        } else {
            m_key_expire_time.erase(key);
        }
    }
    return res;
}

template <typename Task>
void TimeHeap<Task>::getInfo() {
    int size = m_time_heap.size();
    int empty_element = 0;
    while (!m_time_heap.empty()) {
        auto time_node = m_time_heap.top();
        m_time_heap.pop();
        if (time_node->getTask()) {

        } else {
            ++empty_element;
        }
    }
    printf("TimeHeap::getInfo(): size: %d, empty size: %d\n", size, empty_element);
}

#endif //WEB_SERVER_TIMER_H
