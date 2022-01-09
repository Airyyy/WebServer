//
// Created by airy on 2022/1/1.
//

#include "AsyncLogging.h"

AsyncLogging::AsyncLogging(const std::string& log_file, time_t flush_every_n)
    : m_running(false),
      m_log_file(log_file),
      m_flush_interval(flush_every_n),
      m_curr_buff(new Buffer()),
      m_next_buff(new Buffer()),
      m_buff_vec(),
      m_mutex(),
      m_cond(m_mutex) {
    m_curr_buff->zeroFill();
    m_next_buff->zeroFill();
    m_buff_vec.reserve(2);
}

AsyncLogging::~AsyncLogging() {
    if (m_running) stop();
}

void AsyncLogging::append(const char *info, size_t len) {
    LockGuard lock_guard(m_mutex);
    // printf("AsyncLogging::append(): %s\n", info);
    if (m_curr_buff->available(len)) {
        m_curr_buff->append(info, len);
    } else {
        m_buff_vec.emplace_back(std::move(m_curr_buff));
        if (m_next_buff) {
            m_curr_buff = std::move(m_next_buff);
        } else {
            m_curr_buff.reset(new Buffer());
        }
        m_curr_buff->append(info, len);
        m_cond.notify();
    }
    // printf("AsyncLogging::append(): end.\n");
}

void AsyncLogging::threadFunc(const std::string& msg) {
    start();
    LogFile output(m_log_file, AsyncLogging::m_flush_interval);
    BufferPtr new_buffer1(new Buffer());
    BufferPtr new_buffer2(new Buffer());
    new_buffer1->zeroFill();
    new_buffer2->zeroFill();
    BufferVec write_buffers;

    while (m_running) {
        {
            LockGuard lock_guard(m_mutex);
            if (m_buff_vec.empty()) {
                m_cond.waitForSeconds(m_flush_interval);
            }
            m_buff_vec.emplace_back(std::move(m_curr_buff));
            m_curr_buff = std::move(new_buffer1);
            m_curr_buff->reset();
            write_buffers.swap(m_buff_vec);
            if (!m_next_buff) {
                m_next_buff = std::move(new_buffer2);
                m_next_buff->reset();
            }
        }

        if (write_buffers.size() > WRITE_BUFF_UPPER) {
            // too many logs to write, simply discard them.
            write_buffers.resize(2);
        }
        for (auto& buff : write_buffers) {
            output.append(buff->getBuff(), buff->size());
        }

        if (write_buffers.size() > 2) write_buffers.resize(2);

        if (!new_buffer1) {
            new_buffer1 = std::move(write_buffers.back());
            write_buffers.pop_back();
            new_buffer1->zeroFill();
            new_buffer1->reset();
        }

        if (!new_buffer2) {
            new_buffer2 = std::move(write_buffers.back());
            write_buffers.pop_back();
            new_buffer2->zeroFill();
            new_buffer2->reset();
        }

        write_buffers.clear();
        output.flush();
    }
    output.flush();
}

void AsyncLogging::start() {
    m_running = true;
}

void AsyncLogging::stop() {
    m_running = false;
    m_cond.notify();
}