//
//  client.cpp
//  proxy
//
//  Created by Elaine on 2022/2/12.
//

#include "client.hpp"
#include "socket.hpp"
#include <arpa/inet.h>

#define N 1000
void Client::proxy_connet_server(std::string proxy_send_server){
    std::cout<<"client: "<<sockfd<<std::endl;
    if(connect(sockfd, addr_info_list->ai_addr, addr_info_list->ai_addrlen)==-1){
        perror("Fail to connet with the server\n");
        exit(EXIT_FAILURE);
    }
    std::cout<<"Send to server"<<"\n"<<proxy_send_server<<std::endl;
    if(send(sockfd, proxy_send_server.c_str(), sizeof(proxy_send_server),0)==-1){
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
