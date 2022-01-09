//
// Created by airy on 2022/1/2.
//

#ifndef WEB_SERVER_LOGGING_H
#define WEB_SERVER_LOGGING_H
#include "./../base/common.h"
#include "AsyncLogging.h"
#include "LogStream.h"
#include <string>
#include <functional>
#include <sys/time.h>

class Logger : public NoCopyable {
public:
    typedef std::function<void(const char*, size_t)> Functor;  // function that write logs to file
    static Functor m_output;
    static void setOutputFunc(Functor&);
    static void setOutputFunc(Functor&&);

    Logger(const char* file_name, int line);
    ~Logger();

    LogStream& getStream();
    void formatTime();

private:
    std::string m_file_name;  // file that calls LOG
    int m_line;
    LogStream m_stream;
};

#define LOG Logger(__FILE__, __LINE__).getStream()
#endif  //WEB_SERVER_LOGGING_H
