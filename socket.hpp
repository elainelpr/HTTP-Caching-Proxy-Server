//
//  socket.hpp
//  proxy
//
//  Created by Elaine on 2022/2/11.
//

//
// Created on 2/11/22.
//

#ifndef HW2_HTTP_PROXY_SOCKET_H
#define HW2_HTTP_PROXY_SOCKET_H

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

class Socket
{
//这里改了，原来是protected
protected:
    int sockfd;
    struct addrinfo addr_info, *addr_info_list;
    const char *hostname;
    const char *port;
    char *addr_client;
    struct sockaddr_in server_addr;
    
public:
    Socket(const char * _hostname, const char * _port);
    char *get_addrClient();
    ~Socket();
};
#endif //HW2_HTTP_PROXY_SOCKET_H
