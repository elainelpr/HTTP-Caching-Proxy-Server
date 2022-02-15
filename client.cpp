//
//  client.cpp
//  proxy
//
//  Created by Elaine on 2022/2/12.
//

#include "client.hpp"
#include "socket.hpp"
#include <arpa/inet.h>
#include <vector>
#include <iostream>
void Client::proxy_connet_server(std::string proxy_send_server){
    std::cout<<"client: "<<sockfd<<std::endl;
    int _connect = connect(sockfd, addr_info_list->ai_addr, addr_info_list->ai_addrlen);
    if(_connect==-1){
        perror("Fail to connet with the server\n");
        exit(EXIT_FAILURE);
    }
    
    if(send(sockfd, proxy_send_server.c_str(), proxy_send_server.length(),0)==-1){
        perror("Fail to send the message/n");
        exit(EXIT_FAILURE);
    }
    
    //char proxy_recv[10000] = "";
    recv_resp = recv(sockfd, proxy_recv_mssg.data(), proxy_recv_mssg.size(), 0);
    if(recv_resp==-1){
        perror("Fail to receive the message from the server\n");
        exit(EXIT_FAILURE);
    }
    
    std::cout<<proxy_recv_mssg.data()<<std::endl;

}

void Client::proxy_client(int accept_fd){
    
    if(send(accept_fd, proxy_recv_mssg.data(), recv_resp, 0)==-1){
        perror("Proxy fail to send message to the client\n");
        exit(EXIT_FAILURE);
    }
}

Client::~Client(){}



