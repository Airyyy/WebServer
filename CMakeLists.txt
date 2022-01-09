cmake_minimum_required(VERSION 3.10)
project(Web_Server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(
        Web_Server
        main.cpp
        base/MutexLock.h base/common.h base/Condition.h
        threadUtils/Thread.h threadUtils/ThreadPool.h
        log/File.cpp log/File.h log/LogFile.cpp log/LogFile.h log/AsyncLogging.cpp log/AsyncLogging.h log/Logging.cpp log/Logging.h log/Buffer.h log/Buffer.cpp log/LogStream.cpp log/LogStream.h
        net/socketUtils.h net/Timer.h net/Epoll.cpp net/Epoll.h net/Channel.cpp net/Channel.h net/EventLoop.cpp net/EventLoop.h net/Server.cpp net/Server.h net/socketUtils.cpp net/HttpEvent.cpp net/HttpEvent.h net/Timer.cpp)
