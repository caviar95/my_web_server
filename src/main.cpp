#include "server.h"
#include <unistd.h>
#include <iostream>

int main()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    Server server(8080, "../html/");
    server.start();
    return 0;
}