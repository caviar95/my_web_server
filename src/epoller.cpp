#include "epoller.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>

Epoller::Epoller(int max_events) : events_(max_events) {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        std::cerr << "epoll_create1 error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

Epoller::~Epoller() {
    close(epoll_fd_);
}

bool Epoller::add_fd(int fd, uint32_t events) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::mod_fd(int fd, uint32_t events) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::del_fd(int fd) {
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

int Epoller::wait(int timeout_ms) {
    return epoll_wait(epoll_fd_, &events_[0], static_cast<int>(events_.size()), timeout_ms);
}

struct epoll_event* Epoller::get_event(size_t i) {
    assert(i < events_.size());
    return &events_[i];
}
