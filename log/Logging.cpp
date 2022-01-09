//
// Created by airy on 2022/1/2.
//

#include "Logging.h"

void emptyOut(const char*, size_t) {
    // printf("Logging::empty(): No output function is defined.\n");
}
typedef std::function<void(const char*, size_t)> Functor;  // function that write logs to file
Functor Logger::m_output = emptyOut;

Logger::Logger(const char *file_name, int line)
    : m_file_name(file_name),
      m_line(line),
      m_stream() {
    // printf("Logger::Logger()\n");
}

Logger::~Logger() {
    // printf("Logger::~Logger()\n");

    m_stream << log_tab << m_file_name << ":" << m_line << log_end;
    // m_stream << m_file_name << ":" << "160" << "!";
    // printf("here\n");
    Buffer& buffer = m_stream.getBuffer();
    m_output(buffer.getBuff(), buffer.size());
    buffer.reset();
    // printf("Logger::~Logger() end\n");
}

LogStream& Logger::getStream() {
    // printf("Logger::getStream()");
    formatTime();
    return m_stream;
}

void Logger::setOutputFunc(Functor& func) {
    m_output = func;
}

void Logger::setOutputFunc(Functor&& func) {
    m_output = func;
}

void Logger::formatTime() {
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday (&tv, nullptr);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S", p_time);
    m_stream << str_t << log_tab;
    // m_stream << str_t;
}