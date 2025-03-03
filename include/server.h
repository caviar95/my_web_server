#pragma once

#include <string>
#include <thread>
#include <vector>
#include <atomic>

#include "thread_pool.h"

class Server {
public:
    Server(int port, const std::string& html_dir);
    ~Server();
    void start();
    void stop();

private:
    void run();
    void handle_client(int client_socket);

    int port;
    std::string html_dir;
    std::atomic<bool> is_running;
    int server_fd;
    ThreadPool thread_pool;
};
