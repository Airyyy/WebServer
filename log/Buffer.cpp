//
// Created by airy on 2022/1/2.
//
#include "Buffer.h"

Buffer::Buffer(size_t buff_size)
        : m_curr_idx(0),
          m_buff(nullptr) {
    buff_size = std::max(buff_size, BUFF_SIZE_LOWER);
    buff_size = std::min(buff_size, BUFF_SIZE_UPPER);
    m_buff_size = buff_size;
    m_buff = new char[m_buff_size];
}

Buffer::~Buffer() {
    delete [] m_buff;
    m_buff = nullptr;
}

size_t Buffer::append(const char* info, size_t len) {
    if (available(len)) {
        strncpy(m_buff+m_curr_idx, info, len);
        m_curr_idx += len;
        return len;
    } else {
        return 0;
    }
}

char* Buffer::getBuff() {
    return m_buff;
}

size_t Buffer::size() {
    return m_curr_idx;
}

bool Buffer::available(size_t required_size) {
    return (m_buff_size - m_curr_idx) > required_size;
}

void Buffer::reset() {
    m_curr_idx = 0;
}

void Buffer::zeroFill() {
    memset(m_buff, 0, m_buff_size);
}
