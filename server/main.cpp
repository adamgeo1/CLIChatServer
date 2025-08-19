#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

int main() {
    int server_fd, new_socket;
    sockaddr_in address{};
    int opt = 1;
    socklen_t addr_len = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // forcefully attaching socket to port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
    #ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }
    #endif
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind socket to network address and port
    if (bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    // accept incoming connection
    if ((new_socket = accept(server_fd, reinterpret_cast<sockaddr *>(&address), &addr_len)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    // read and echo message

    for (;;) {
        ssize_t val_read = read(new_socket, buffer, BUFFER_SIZE);
        if (val_read <= 0) {
            if (val_read < 0) {
                perror("read");
            }
            break;
        }
        std::string msg(buffer, buffer + val_read);
        std::cout << "Received: " << msg << std::endl;
        if (msg == "exit") {
            std::cout << "Client exit, shutting down server";
            break;
        }
        send(new_socket, buffer, BUFFER_SIZE, 0);
    }

    // close socket
    close(new_socket);
    close(server_fd);
    return 0;
}