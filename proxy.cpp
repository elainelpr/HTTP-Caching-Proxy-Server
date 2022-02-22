//
//  proxy.cpp
//  hw2
//
//  Created by Elaine on 2022/2/20.
//

#include "proxy.hpp"
#include<pthread/pthread.h>

char *addr_client;
int clientProxy_connectfd;
std::string proxy_send_server;
std::vector<char> proxy_recv_mssg;

socket_info create_socket(const char *hostname, const char *port){
    struct addrinfo addr_info, *addr_info_list;
    int sockfd;
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
    }
    
    socket_info sock_info;
    sock_info.socket_fd = sockfd;
    sock_info.info_list = addr_info_list;
    return sock_info;
}


void Proxy::proxy_bindListen_accept(){
    socket_info clientServer_info = create_socket("192.168.1.142", "12345");
    int opt = 1;
    if(setsockopt(clientServer_info.socket_fd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt))==-1){
        perror("Fail to use port again\n");
        exit(EXIT_FAILURE);
    }
    
    if(bind(clientServer_info.socket_fd, clientServer_info.info_list->ai_addr, clientServer_info.info_list->ai_addrlen) == -1){
        perror("Fail to bind to server IP to the socket");
        exit(EXIT_FAILURE);
    }

    if(listen(clientServer_info.socket_fd, 1000) == -1){
        perror("Fail to convert socket_fd into listen_fd");
        exit(EXIT_FAILURE);
    }
    //Accept the request from client
    while(1){
        struct sockaddr_in clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);
        clientProxy_connectfd = accept(clientServer_info.socket_fd, (struct sockaddr*)&clientaddr, &clientaddr_len);
        std::cout<<clientProxy_connectfd<<std::endl;
        
        if(clientProxy_connectfd == -1){
            perror("Fail to accept client's connection");
            exit(EXIT_FAILURE);
        }
        
        //pthread_create
        pthread_t tid;
        void *a = NULL;
        pthread_create(&tid,NULL, Proxy::proxyServer, &a);
        //线程分离
        pthread_detach(tid);

        //use the port again
        int connect_opt = 1;
        if(setsockopt(clientProxy_connectfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&connect_opt, sizeof(connect_opt))==-1){
            perror("Fail to use port again\n");
            exit(EXIT_FAILURE);
        }
    }
    
}


void *Proxy::proxyServer(void *){
    //vector char
    char buffer[65536]="";
    ssize_t _recv = recv(clientProxy_connectfd, buffer, 65536, 0);
    if(_recv==-1){
        perror("Fail to receive the message from client\n");
        exit(EXIT_FAILURE);
    }
    
    parse_request(buffer);
    proxyServer_serverClient();
    return NULL;
}

void proxyServer_serverClient(){
    //Send the message from the client to server through proxy
    socket_info serverClient_info = create_socket(addr_client,"80");
    
    int _connect = connect(serverClient_info.socket_fd, serverClient_info.info_list->ai_addr, serverClient_info.info_list->ai_addrlen);
    if(_connect==-1){
        perror("Fail to connet with the server\n");
        exit(EXIT_FAILURE);
    }
    
    if(send(serverClient_info.socket_fd, proxy_send_server.c_str(), proxy_send_server.length(),0)==-1){
        perror("Fail to send the message/n");
        exit(EXIT_FAILURE);
    }
    //send message from server to client through proxy
    char proxy_recv[65536] = "";
    std::memset(&proxy_recv_mssg,0, proxy_recv_mssg.size());
    while(1){
        //receive message from the server
        size_t recv_resp = recv(serverClient_info.socket_fd, proxy_recv, 65536,0);
        if(recv_resp==-1){
            perror("Fail to receive the message from the server\n");
            exit(EXIT_FAILURE);
        }
        if(recv_resp==0){
            break;
        }
        proxy_recv_mssg.insert(proxy_recv_mssg.end(), proxy_recv, proxy_recv+recv_resp);
        //send message to the client
        if(send(clientProxy_connectfd, proxy_recv, recv_resp, 0)==-1){
            perror("Proxy fail to send message to the client\n");
            exit(EXIT_FAILURE);
        }
    }
    std::cout<<proxy_recv_mssg.data()<<std::endl;
}



void parse_request(char *buffer){
    std::string parse_req = buffer;
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
    std::cout<<"************"<<std::endl;
    std::cout<<argument<<std::endl;
    if(method_part == "GET" || method_part=="POST"){
        //parse the http://www.....
        std::string arg_part = GET_method(argument);
        std::string GET_method = method_part+" "+arg_part+version_part;
        std::string req_data = parse_req.substr(get);
        proxy_send_server = GET_method  + req_data;
        std::cout<<"Receive from the client: "<<"\n"<<proxy_send_server<<std::endl;
    }

    if(method_part == "CONNECT"){
        
    }

}

std::string GET_method(std::string argument){
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




