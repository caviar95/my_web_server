#include "server.h"
#include "request_handler.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

Server::Server(int port, const std::string& html_dir)
    : port(port), html_dir(html_dir), is_running(false), thread_pool(4) {}

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
}

void Server::stop() {
    if (is_running) {
        is_running = false;
        close(server_fd);
        std::cout << "Server stopped.\n";
    }
}

void Server::run() {
    while (is_running) {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }

        thread_pool.enqueue([this, client_socket]() {
            handle_client(client_socket);
        });
    }
}

void Server::handle_client(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));

    std::string request(buffer);
    std::string response = RequestHandler::handle_request(request, html_dir);

    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}