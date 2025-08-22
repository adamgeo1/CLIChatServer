#include <iostream>
#include <string>
#include <cstring>
#include <format>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <json.hpp>
#include <sstream>
#include <vector>
#include <fstream>

// Client Main

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

using namespace std;
using namespace nlohmann;

vector<string> split(const string& str, const char delim) {
    istringstream iss(str);
    string temp;
    vector<string> tokens;

    while (getline(iss, temp, delim)) {
        if (!temp.empty()) {
            tokens.push_back(temp);
        }
    }
    return tokens;
}

json msg_to_json(const int req_id, const string& msg) {
    json j;
    j["req_id"] = req_id;
    vector<string> split_msg = split(msg, ' ');
    string cmd = split_msg[0].substr(1);
    j["command"] = cmd;
    if (cmd == "signup" or cmd == "login") {
        j["args"] = {
            {"username", split_msg[1]},
            {"password", split_msg[2]}
        };
    }
    else if (cmd == "join") {
        j["args"] = {
            {"room", split_msg[1]}
        };
    }
    else if (cmd == "msg") {
        ostringstream oss;
        for (size_t i = 2; i < split_msg.size(); ++i) {
            if (i > 2) oss << ' ';
            oss << split_msg[i];
        }
        j["args"] = {
            {"room", split_msg[1]},
            {"text", oss.str()}
        };
    }
    else if (cmd == "dm") {
        ostringstream oss;
        for (size_t i = 2; i < split_msg.size(); ++i) {
            if (i > 2) oss << ' ';
            oss << split_msg[i];
        }
        j["args"] = {
            {"user", split_msg[1]},
            {"text", oss.str()}
        };
    }
    else if (cmd == "history") {
        j["args"] = {
            {"room", split_msg[1]},
            {"limit", split_msg[2]}
        };
    }
    else if (cmd == "history_dm") {
        j["args"] = {
            {"user", split_msg[1]},
            {"limit", split_msg[2]}
        };
    }
    /*
    string dump = j.dump(2);
    ofstream out(format("{}json.txt", req_id));
    out << dump;
    */
    return j;
}

int main() {
    int sock = 0;
    sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // convert ipv4 and ipv6 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        return -1;
    }

    // connect to server
    if (connect(sock, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed" << endl;
        return -1;
    }

    int req_id = 1;

    for (;;) {
        string msg;
        cout << "Enter message: ";
        getline(cin, msg);
        json req = msg_to_json(req_id, msg);
        if (req["command"] == "exit") break;
        string payload = req.dump();
        uint32_t len = htonl(payload.size());
        string frame;
        frame.append(reinterpret_cast<char*>(&len), sizeof(len));
        frame.append(payload);
        req_id++;
        send(sock, frame.data(), frame.size(), 0);
    }

    // close the socket
    close(sock);
    return 0;
}