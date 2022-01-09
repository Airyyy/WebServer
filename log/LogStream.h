//
// Created by airy on 2022/1/2.
//

#ifndef WEB_SERVER_LOGSTREAM_H
#define WEB_SERVER_LOGSTREAM_H
#include "./../base/common.h"
#include "Buffer.h"
#include <memory>
#include <cstring>

std::string log_tab();
std::string log_end();

class LogStream : public NoCopyable {
public:
    typedef LogStream& reference;
    typedef std::string(*Func)(void);
    LogStream();
    ~LogStream();
    Buffer& getBuffer();

    /* overload << operator for possible log info. */
    reference operator<<(int);
    reference operator<<(unsigned short);
    reference operator<<(const char*);
    reference operator<<(char*);
    reference operator<<(const std::string&);
    reference operator<<(std::string&);

    reference operator<<(Func);

private:
    Buffer m_buffer;
};
#endif //WEB_SERVER_LOGSTREAM_H
