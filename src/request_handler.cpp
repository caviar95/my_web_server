#include "request_handler.h"

#include <fstream>
#include <sstream>

std::string RequestHandler::handle_request(const std::string& request, const std::string& html_dir) {
    size_t path_start = request.find(' ') + 1;
    size_t path_end = request.find(' ', path_start);
    std::string path = request.substr(path_start, path_end - path_start);

    std::string file_path = html_dir + path;
    if (file_path.back() == '/') {
        file_path += "index.html";
    }

    std::ifstream file(file_path);
    if (file) {
        std::ostringstream content;
        content << file.rdbuf();
        return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + content.str();
    } else {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
    }
}