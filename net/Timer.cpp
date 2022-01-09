//
// Created by airy on 2022/1/8.
//
#include "Timer.h"

time_t getCurrTime() {
    struct timeval now{};
    gettimeofday(&now, nullptr);
    time_t now_time = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    return now_time;
}
