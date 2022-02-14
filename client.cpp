//
//  client.cpp
//  proxy
//
//  Created by Elaine on 2022/2/12.
//

#include "client.hpp"
#include "socket.hpp"
#include <arpa/inet.h>

#define N 1024000
void Client::proxy_connet_server(std::string proxy_send_server){
    std::cout<<"client: "<<sockfd<<std::endl;
    int _connect = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(_connect==-1){
        perror("Fail to connet with the server\n");
        exit(EXIT_FAILURE);
    }
    
    
    if(send(sockfd, proxy_send_server.c_str(), proxy_send_server.length(),0)==-1){
        perror("Fail to send the message/n");
        exit(EXIT_FAILURE);
    }
    
    char proxy_recv_mssg[N] = "";
    ssize_t recv_resp = recv(sockfd, proxy_recv_mssg, N, 0);
    if(recv_resp==-1){
        perror("Fail to receive the message from the server\n");
        exit(EXIT_FAILURE);
    }
    std::cout<<recv_resp<<std::endl;
    std::cout<<proxy_recv_mssg<<std::endl;
}


Client::~Client(){}



