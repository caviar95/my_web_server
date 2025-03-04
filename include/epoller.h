#pragma once

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

class Epoller {
public:
    explicit Epoller(int max_events = 1024);
    ~Epoller();

    // 添加文件描述符到 epoll 监听
    bool add_fd(int fd, uint32_t events);
    // 修改 epoll 监听的事件
    bool mod_fd(int fd, uint32_t events);
    // 从 epoll 移除文件描述符
    bool del_fd(int fd);
    // 等待事件发生（返回就绪的事件数量）
    int wait(int timeout_ms = -1);
    // 获取就绪的事件
    struct epoll_event* get_event(size_t i);

private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
};