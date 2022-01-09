//
// Created by airy on 2022/1/1.
//
#ifndef WEB_SERVER_ASYNCLOGGING_H
#define WEB_SERVER_ASYNCLOGGING_H
#include "./../base/common.h"
#include "./../base/MutexLock.h"
#include "./../base/Condition.h"
#include "LogFile.h"
#include "Buffer.h"
#include <vector>
#include <memory>
#include <cstring>
#include <algorithm>

class AsyncLogging : public NoCopyable, BaseThreadEvent {
public:
    typedef std::shared_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVec;

    AsyncLogging(const std::string& log_file, time_t flush_every_n = 3);
    ~AsyncLogging();

    void append(const char* info, size_t len);
    void threadFunc(const std::string& msg) override;
    void start();
    void stop();
private:
    bool m_running;
    std::string m_log_file;
    time_t m_flush_interval;
    BufferPtr m_curr_buff;  // front-end write to this buffer
    BufferPtr m_next_buff;  // when front-end is full, use this as a replacement
    BufferVec m_buff_vec;  // used to store BufferPtrs which need to be written to file.
    Mutex m_mutex;
    Condition m_cond;
};
#endif  // WEB_SERVER_ASYNCLOGGING_H