//
// Created by airy on 2021/12/30.
//
#ifndef WEB_SERVER_COMMON_H
#define WEB_SERVER_COMMON_H
#include <string>

class NoCopyable {
public:
    NoCopyable() = default;
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable& operator=(const NoCopyable&) = delete;
};

class BaseThreadEvent {
protected:
    virtual void threadFunc(const std::string& msg) = 0;
};
#endif  // WEB_SERVER_COMMON_H
