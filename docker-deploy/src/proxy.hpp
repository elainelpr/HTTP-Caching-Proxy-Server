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

#include <fstream>

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
    static void *proxyServer(void *);
    
};
std::string getCurrentTime();
socket_info create_socket(const char *hostname, const char *port);
void parse_request(char *buffer);
std::vector<char> proxyServer_serverClient();
socket_info proxy_CONNECT_server(char *hostname, const char *port);
std::string GET_method(std::string argument);
void proxy_client(std::string cache_content);
void process_connect(int accept_fd, int proxyClient_fd);
#endif /* proxy_hpp */
