//
// Created by airy on 2022/1/1.
//
#include "LogFile.h"

LogFile::LogFile(const std::string &file_name, size_t flush_every_n)
    : m_flush_interval(flush_every_n),
      m_cnt(0),
      m_file(new File(file_name)),
      m_mutex() {}

void LogFile::append_unlocked(const char* info, size_t len) {
    m_file->append(info, len);
    ++m_cnt;
    if (m_cnt >= m_flush_interval) {
        m_cnt = 0;
        m_file->flush();
    }
}

void LogFile::append(const char *info, size_t len) {
    LockGuard lock_guard(m_mutex);
    this->append_unlocked(info, len);
}

void LogFile::flush() {
    LockGuard lock_guard(m_mutex);
    m_file->flush();
}