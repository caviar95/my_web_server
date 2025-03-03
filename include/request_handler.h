#pragma once

#include <string>

class RequestHandler {
public:
    static std::string handle_request(const std::string& request, const std::string &html_dir);
};

