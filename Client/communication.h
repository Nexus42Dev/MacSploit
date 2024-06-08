//
// Created by Nexus Pancakes on 13/11/2022.
//

#ifndef CLIENT_COMMUNICATION_H
#define CLIENT_COMMUNICATION_H

#include <iostream>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <errno.h>
#include <sstream>

#include "exploit.h"

struct recv_msg {
    uint8_t type;
    size_t size;
};

enum recv_type {
    IPC_EXECUTE,
    IPC_SETTING,
    IPC_PING
};

std::vector<std::string> script_queue;
int ports_start = 5553;
int pong = 0x10;

int setup_socket(int port, sockaddr_in* addr, size_t addr_len) {
    const int reuseaddr = 1;
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = INADDR_ANY;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cout << "[IPC] Failed to create socket.\n";
        return false;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) < 0) {
        std::cout << "[IPC] Failed to bypass cleanup state.\n";
    }

    /*
    int nonblocking = 1;
    if (ioctl(sockfd, FIONBIO, (char*)&nonblocking) < 0) {
        std::cout << "[IPC] Attempt to Enabled Async Socket Failed.\n";
        close(sockfd);
    }

    */

    if (bind(sockfd, (struct sockaddr*)addr, addr_len) < 0) {
        return -1;
    }

    return sockfd;
}

void init_socket() {
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    int active_port = 0;
    int clientfd;
    int sockfd;

    while (true) {
        bool interrupt = false;
        for (int i = 0; i < 10; i++) {
            int sel_port = ports_start + i;
            if ((sockfd = setup_socket(sel_port, &addr, sizeof(addr))) != -1) {
                active_port = sel_port;
                interrupt = true;
                break;
            }

            close(sockfd);
        }

        if (interrupt) break;
        std::cout << "[IPC] Failed to bind socket information, trying again...\n";
        sleep(1);
    }

    if (listen(sockfd, 2) < 0) {
        std::cout << "[IPC] Failed to listen for clients.\n";
        close(sockfd);
        return;
    }

    while (true) {
        std::cout << "[IPC] Listening to TCP " << std::to_string(active_port) << ".\n";
        current_port = active_port;
        
        clientfd = accept(sockfd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
        int bytes_read;

        std::cout << "[IPC] UI Attached.\n";
        while (true) {
            recv_msg msg_data;
            bytes_read = recv(clientfd, &msg_data, sizeof(recv_msg), 0);
            
            if (bytes_read < 1) {
                std::cout << "[IPC] Disconnected.\n";
                break;
            }

            if (bytes_read != sizeof(recv_msg)) {
                std::cout << "Unknown Bytes Read: 0x" << bytes_read << "\n";
                continue;
            }

            if (msg_data.type == IPC_PING) {
                send(clientfd, &pong, 1, 0);
                continue;
            }

            size_t buffer_len = msg_data.size; char msg_body[buffer_len];
            bytes_read = recv(clientfd, &msg_body, buffer_len, MSG_WAITALL);

            if (msg_data.type == IPC_SETTING) {
                std::istringstream data(msg_body);
                std::string key; std::getline(data, key, ' ');
                std::string value; std::getline(data, value, ' ');
                std::cout << "[IPC] Received Setting Event. " << key << ": " << value << "\n";
                settings::handle_setting(key, value);
                continue;
            }

            if (msg_data.type != IPC_EXECUTE) continue;
            if (!whitelisted || !roblox_thread) continue;
            execute_script(roblox_thread, std::string(msg_body, buffer_len));
        }
    }
}

#endif //CLIENT_COMMUNICATION_H
