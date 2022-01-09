//
// Created by airy on 2022/1/1.
//
#ifndef WEB_SERVER_LOGFILE_H
#define WEB_SERVER_LOGFILE_H
#include "./../base/common.h"
#include "./../base/MutexLock.h"
#include "File.h"

#include <string>
#include <memory>

class LogFile : NoCopyable {
public:
    LogFile(const std::string& file_name, size_t flush_every_n);
    ~LogFile() {};

    void append(const char* info, size_t len);
    void flush();

private:
    void append_unlocked(const char* info, size_t len);
    const size_t m_flush_interval;
    size_t m_cnt;
    std::unique_ptr<File> m_file;
    Mutex m_mutex;
};
#endif  // WEB_SERVER_LOGFILE_H
