#pragma once

#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <mutex>

#include "thread_pool.h"
#include "epoller.h"
#include "timer.h"

class Server {
public:
    Server(int port, const std::string& html_dir);
    ~Server();
    void start();
    void stop();

private:
    void run();
    void handle_events(int num_events); // 处理 epoll 事件
    void accept_connection();          // 接受新连接
    void handle_client(int fd);        // 处理客户端数据

    void handle_timeout();

    int port;
    std::string html_dir;
    std::atomic<bool> is_running;
    int server_fd;
    ThreadPool thread_pool;
    Epoller epoller_;

    Timer timeout_timer_;
    std::unordered_map<int, time_t> conn_last_active_;
    std::mutex conn_mutex_;
};
