#pragma once

#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <functional>

class Timer {
public:
    Timer();
    ~Timer();

    void start(int interval_sec);
    void stop();
    bool get_fd() const {
        return timer_fd_;
    }

private:
    int timer_fd_;
};
