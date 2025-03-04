#include "timer.h"

Timer::Timer() {
    timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd_ == -1) {
        throw std::runtime_error("timerfd_create error");
    }
}

Timer::~Timer() {
    if (timer_fd_ != -1) {
        close(timer_fd_);
    }
}

void Timer::start(int interval_ms) {
    struct itimerspec new_value;
    new_value.it_interval.tv_sec = interval_ms / 1000;
    // new_value.it_interval.tv_nsec = (interval_ms % 1000) * 1000000;
    new_value.it_value.tv_sec = new_value.it_interval.tv_sec;
    // new_value.it_value.tv_nsec = new_value.it_interval.tv_nsec
    if (timerfd_settime(timer_fd_, 0, &new_value, nullptr) < 0) {
        throw std::runtime_error("timerfd_settime error");
    }
}

void Timer::stop() {
    struct itimerspec its{};
    memset(&its, 0, sizeof(its));
    timerfd_settime(timer_fd_, 0, &its, nullptr);
}
