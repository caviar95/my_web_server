#include "server.h"
#include "request_handler.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>

Server::Server(int port, const std::string& html_dir)
    : port(port), html_dir(html_dir), is_running(false), thread_pool(4), epoller_(1024) {}

Server::~Server() {
    stop();
}

void Server::start() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        return;
    }

    is_running = true;
    std::cout << "Server started on port " << port << "...\n";
    run();

    timeout_timer_.start(5);
    epoller_.add_fd(timeout_timer_.get_fd(), EPOLLIN);
}

void Server::stop() {
    if (is_running) {
        is_running = false;
        close(server_fd);
        std::cout << "Server stopped.\n";
    }
}

void Server::run() {
    // while (is_running) {
    //     sockaddr_in client_addr{};
    //     socklen_t client_addr_len = sizeof(client_addr);
    //     int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    //     if (client_socket < 0) {
    //         std::cerr << "Accept failed\n";
    //         continue;
    //     }

    //     thread_pool.enqueue([this, client_socket]() {
    //         handle_client(client_socket);
    //     });
    // }

    // 将 server_fd 添加到 epoll 监听
    epoller_.add_fd(server_fd, EPOLLIN | EPOLLET); // 边缘触发模式

    while (is_running) {
        int num_events = epoller_.wait(1000); // 等待1秒
        if (num_events < 0) {
            std::cerr << "epoll_wait error\n";
            continue;
        }
        handle_events(num_events);
    }
}

// 处理所有就绪的事件
void Server::handle_events(int num_events) {
    for (int i = 0; i < num_events; ++i) {
        int fd = epoller_.get_event(i)->data.fd;
        if (fd == server_fd) {
            accept_connection(); // 处理新连接
        } else if (fd == timeout_timer_.get_fd()) {
            handle_timeout();
        } else {
            // 更新客户端活动时间
            {
                std::lock_guard<std::mutex> lock(conn_mutex_);
                conn_last_active_[fd] = time(nullptr);
            }

            // 处理客户端数据
            thread_pool.enqueue([this, fd]() {
                handle_client(fd);
            });
        }
    }
}

// 接受新连接并设置为非阻塞
void Server::accept_connection() {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        std::cerr << "accept error\n";
        return;
    }

    // 设置非阻塞模式
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    // 将客户端 fd 加入 epoll 监听
    epoller_.add_fd(client_fd, EPOLLIN | EPOLLET);
}

// void Server::handle_client(int client_socket) {
//     char buffer[1024] = {0};
//     read(client_socket, buffer, sizeof(buffer));

//     std::string request(buffer);
//     std::string response = RequestHandler::handle_request(request, html_dir);

//     send(client_socket, response.c_str(), response.size(), 0);
//     close(client_socket);
// }

// 处理客户端请求（非阻塞读取）
void Server::handle_client(int fd) {
    char buffer[4096];
    ssize_t bytes_read;

    while (true) { // 边缘触发需要循环读取
        bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 数据已读完
            } else {
                std::cerr << "read error\n";
                close(fd);
                return;
            }
        } else if (bytes_read == 0) { // 客户端关闭连接
            close(fd);
            epoller_.del_fd(fd);
            return;
        }

        // 处理请求并发送响应
        std::string request(buffer, bytes_read);
        std::string response = RequestHandler::handle_request(request, html_dir);
        send(fd, response.c_str(), response.size(), 0);
    }

    // 客户端关闭连接时清理数据
    if (bytes_read == 0) {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        conn_last_active_.erase(fd);
    }
}

void Server::handle_timeout() {
    uint64_t exp;
    read(timeout_timer_.get_fd(), &exp, sizeof(exp));

    std::lock_guard<std::mutex> lock(conn_mutex_);
    time_t now = time(nullptr);
    auto it = conn_last_active_.begin();
    while (it != conn_last_active_.end()) {
        if (now - it->second > 60) { // 超过时间 60秒
            std::cout << "Timeout: closing connection: " << it->first << std::endl;
            close(it->first);
            epoller_.del_fd(it->first);
            it = conn_last_active_.erase(it);
        } else {
            ++it;
        }
    }
}