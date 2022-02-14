//
//  socket.cpp
//  proxy
//
//  Created by Elaine on 2022/2/11.
//

//
// Created on 2/11/22.
//

#include "socket.hpp"
#include <arpa/inet.h>
Socket::Socket(const char *_hostname, const char  *_port) : hostname(_hostname), port(_port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1){
        perror("Fail to create the socket\n");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port = htons(atoi(port));
}

Socket::~Socket() {
    close(sockfd);
    //freeaddrinfo(addr_info_list);
}
