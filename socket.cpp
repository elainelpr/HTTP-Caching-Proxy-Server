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

Socket::Socket(const char *_hostname, const char  *_port) : hostname(_hostname), port(_port) {
    memset(&addr_info, 0, sizeof(addr_info));
    addr_info.ai_family = AF_INET; // IPv4
    addr_info.ai_socktype = SOCK_STREAM; // TCP

    //convert addr_info into socket_addr_info
    if(getaddrinfo(hostname, port, &addr_info, &addr_info_list) != 0){
        perror("Fail to get socket address information.");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(addr_info_list->ai_family,
                    addr_info_list->ai_socktype,
                    addr_info_list->ai_protocol);
    if(sockfd == -1){
        perror("Fail to create the socket.");
        exit(EXIT_FAILURE);
    }
}

Socket::~Socket() {
    //close(sockfd);
    //freeaddrinfo(addr_info_list);
}
