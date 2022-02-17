//
//  server.cpp
//  proxy
//
//  Created by Elaine on 2022/2/11.
//


//
// Created on 2/11/22.
//


//改动： 初始化时Socket的NULL改了
//int connectfd;作为成员变量放在server class中。因为后面receive需要用到
//加了析构函数
#include "server.hpp"
#include <string>
#include <arpa/inet.h>

Server::Server(const char *_hostname, const char *_port) : Socket(_hostname, _port){
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt))==-1){
        perror("Fail to use port again\n");
        exit(EXIT_FAILURE);
    }
    
    if(bind(sockfd, addr_info_list->ai_addr, addr_info_list->ai_addrlen) == -1){
        perror("Fail to bind to server IP to the socket");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, 1000) == -1){
        perror("Fail to convert socket_fd into listen_fd");
        exit(EXIT_FAILURE);
    }
}

void Server::Accept() {
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    connectfd = accept(sockfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
    std::cout<<connectfd<<std::endl;
    
    if(connectfd == -1){
        perror("Fail to accept client's connection");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if(setsockopt(connectfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt))==-1){
        perror("Fail to use port again\n");
        exit(EXIT_FAILURE);
    }
    connectfd_list.push_back(connectfd);
}

void Server::Receive_test(){
    //vector char
    char buffer[65536]="";
    //char header[10]="";
    std::cout<<"**********"<<std::endl;
    ssize_t _recv = recv(connectfd, buffer, 65536, 0);
    if(_recv==-1){
        perror("Fail to receive the message from client\n");
        exit(EXIT_FAILURE);
        
    }
    std::string parse_req = buffer;
    /*size_t length = parse_req.find("Content-Length");
    if(length!=std::string::npos){
        //find the content-length position
        std::string contentLength = parse_req.substr(length);
        //extract the line of content-length
        size_t length_line = contentLength.find("\r\n");
        std::string content_length = contentLength.substr(0, length_line);
        std::cout<<"8********************"<<std::endl;
        std::cout<<content_length<<std::endl;
        //the content_length is Content-Length: 81.
        //extract the 81 from content_length, find " "first
        size_t len = content_length.find(" ");
        //Length is 81
        std::string Length = content_length.substr(len);
        //convert the string "81" to number 81
        ssize_t _len = stoi(Length);
        std::cout<<_len<<std::endl;
        _len = _len - _recv;
        while(_len>0){
            _recv = recv(connectfd, buffer, 1024, 0);
            if(_recv==-1){
                perror("Fail to receive the message\n");
                exit(EXIT_FAILURE);
            }
            std::string buf = buffer;
            parse_req+=buf;
            _len = _len - _recv;
            std::cout<<"The rest len is"<<std::endl;
        }
    }*/
    //parse the request, extract the first line
    std::string line = "\r\n";
    size_t get = parse_req.find(line);
    //extract the first line of the client message(ex: GET www...... HTTP/1.1)
    std::string req_head = parse_req.substr(0, get);
    //extract the first part(GET, POST..)
    size_t method = req_head.find_first_of(" ");
    std::string method_part = req_head.substr(0, method);
    //extract the last part(HTTP/1.1)
    size_t version = req_head.find_last_of(" ");
    std::string version_part = req_head.substr(version+1);
    //extract the middle part(www.cmu.edu)
    std::string argument = req_head.substr(method+1, version-method);
    if(method_part == "GET" || method_part=="POST"){
        //parse the http://www.....
        std::string arg_part = GET_method(argument);
        std::string GET_method = method_part+" "+arg_part+version_part;
        std::string req_data = parse_req.substr(get);
        proxy_send_server = GET_method  + req_data;
        std::cout<<"Receive from the client: "<<"\n"<<proxy_send_server<<std::endl;
    }
}

std::string Server::GET_method(std::string argument){
    size_t first_slash = argument.find("/");
    std::string first = argument.substr(first_slash+1);
    size_t second_slash = first.find("/");
    std::string second = first.substr(second_slash+1);
    size_t res_part = second.find("/");
    std::string argument_part = second.substr(res_part);
    
    //extract the domain name
    std::string domain_name = argument.substr(first_slash+2, res_part);
    struct hostent *host_name = gethostbyname(domain_name.c_str());
    if(!host_name){
        perror("The domain name is wrong\n");
        exit(EXIT_FAILURE);
    }
    //Convert the domain name to IP address
    addr_client = inet_ntoa(*(struct in_addr*)host_name->h_addr_list[0]);
    return argument_part;

}

int Server::get_connectfd(){
    return connectfd;
}

std::string Server::get_proxySendSerevr(){
    return proxy_send_server;
}

Server::~Server(){
    close(connectfd);
}



