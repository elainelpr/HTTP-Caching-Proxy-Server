//
//  proxy.hpp
//  hw2
//
//  Created by Elaine on 2022/2/20.
//

#ifndef proxy_hpp
#define proxy_hpp

#include <stdio.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
struct socket_info{
    int socket_fd;
    struct addrinfo *info_list;
};

class Proxy{
public:
    
public:
    Proxy(){}
    void proxy_bindListen_accept();
    void Accept();
    static void *proxyServer_serverClient(void *);
};
socket_info create_socket(const char *hostname, const char *port);
void parse_request(char *buffer);
std::string GET_method(std::string argument);
#endif /* proxy_hpp */
