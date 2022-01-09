//
// Created by airy on 2022/1/2.
//

#include "LogStream.h"

typedef LogStream& reference;
typedef std::string(*Func)();

std::string log_tab() {
    std::string info("\t");
    return info;
}

std::string log_end() {
    std::string info("\n");
    return info;
}

LogStream::LogStream()
    : m_buffer(BUFF_SIZE_LOWER){
    // printf("LogStream::LogStream()\n");
}

LogStream::~LogStream() = default;

Buffer& LogStream::getBuffer() {
    return m_buffer;
}

reference LogStream::operator<<(const char* info) {
    if (info) {
        m_buffer.append(info, strlen(info));
    } else {
        m_buffer.append("NULL", 4);
    }
    return *this;
}

reference LogStream::operator<<(char* info) {
    return operator<<(reinterpret_cast<const char*>(info));
}

reference LogStream::operator<<(const std::string& info) {
    if (info.empty()) {
        m_buffer.append("NULL", 4);
    } else {
        m_buffer.append(info.c_str(), info.size());
    }
    return *this;
}

reference LogStream::operator<<(std::string& info) {
    if (info.empty()) {
        m_buffer.append("NULL", 4);
    } else {
        m_buffer.append(info.c_str(), info.size());
    }
    return *this;
}

reference LogStream::operator<<(int val) {
    std::string info(std::to_string(val));
    // printf("val = %s, size = %ld\n", info.c_str(), info.size());
    m_buffer.append(info.c_str(), info.size());
    return *this;
}

reference LogStream::operator<<(unsigned short val) {
    int i_val = int(val);
    return operator<<(i_val);
}


reference LogStream::operator<<(Func func) {
    std::string info = func();
    return operator<<(reinterpret_cast<const std::string&>(info));
}