#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Server Main

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

    std::byte len_buf[4];
    for (;;) {
        constexpr int prefix_bytes = 4;
        int bytes_read = 0;
        ssize_t val_read = 0;
        // ensures that entire prefix is read, if not then leaves trailing characters in front of json
        while (bytes_read < prefix_bytes) {
            val_read = read(new_socket, len_buf + bytes_read, prefix_bytes - bytes_read);
            if (val_read <= 0) {
                if (val_read < 0) {
                    perror("read");
                }
                break;
            }
            bytes_read += val_read;
        }
        if (bytes_read != prefix_bytes) {
            perror("didn't read prefix");
            break;
        }

        // ensures that the prefix says the same byte size as the payload
        // ex: client sent 76 byte payload so prefix should be 00 00 00 4C
        uint32_t net_len = 0;
        std::memcpy(&net_len, len_buf, sizeof(net_len));
        uint32_t len = ntohl(net_len);
        if (len == 0 || len > 1'048'576) break;

        // retrieve rest of bytes from client request, loop ensures doesn't use previous byte count1
        std::string payload(len, '\0');
        size_t got = 0;
        while (got < len) {
            ssize_t r = read(new_socket, payload.data() + got, len - got);
            if (r == 0) break;
            if (r < 0) {
                if (errno == EINTR) continue;
                break;
            }
            got += static_cast<size_t>(r);
        }
        if (got != len) break;
        std::cout << "Received: " << payload << std::endl;
        send(new_socket, payload.data(), payload.size(), 0);
    }

    // close socket
    close(new_socket);
    close(server_fd);
    return 0;
}