//
//  server.hpp
//  proxy
//
//  Created by Elaine on 2022/2/11.
//

//
// Created on 2/11/22.
//

#ifndef HW2_HTTP_PROXY_SERVER_H
#define HW2_HTTP_PROXY_SERVER_H

#include "socket.hpp"
#include <vector>

class Server : public Socket{
//private:
protected:
    //int connectfd;
    std::vector<int> connectfd_list;
    int connectfd;
    std::string proxy_send_server;
public:
    Server(const char *_hostname, const char *_port);
    void Accept();
    void Receive_test();
    std::string GET_method(std::string argument);
    int get_connectfd();
    std::string get_proxySendSerevr();
    ~Server();
};


#endif //HW2_HTTP_PROXY_SERVER_H


