//
//  proxy.cpp
//  hw2
//
//  Created by Elaine on 2022/2/20.
//

#include "proxy.hpp"
#include "request.h"
#include "cache.h"
#include "response.h"
#include <pthread.h>
#include <ctime>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "proxyexception.h"
#include "global.h"

Cache cache;
char *addr_client;
int request_id = 0;
std::unordered_map<pthread_t, int> threads_fd;
pthread_mutex_t threadsClientFd_mutex = PTHREAD_MUTEX_INITIALIZER;
std::string proxy_send_server;
socket_info clientServer_info;
pthread_mutex_t logfile_mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream LOG_FILE("/home/xg77/568/HW2_1/proxy.log", std::ofstream::out);

void insertThreadsClientFd(pthread_t th, int client_fd){
    pthread_mutex_lock(&threadsClientFd_mutex);
    threads_fd[th] = client_fd;
    pthread_mutex_unlock(&threadsClientFd_mutex);
}

int getThreadsClientFd(pthread_t th){
    pthread_mutex_lock(&threadsClientFd_mutex);
    if(threads_fd.count(th) == 0){
        std::cout << "ERROR: can't find client fd" << std::endl;
    }
    int res = threads_fd[th];
    pthread_mutex_unlock(&threadsClientFd_mutex);
    return res;
}

void removeThreadsClientFd(pthread_t th){
    pthread_mutex_lock(&threadsClientFd_mutex);
    threads_fd.erase(th);
    pthread_mutex_unlock(&threadsClientFd_mutex);
}

// create socket
socket_info create_socket(const char *hostname, const char *port){
    struct addrinfo addr_info, *addr_info_list;
    int sockfd;
    memset(&addr_info, 0, sizeof(addr_info));
    addr_info.ai_family = AF_INET; // IPv4
    addr_info.ai_socktype = SOCK_STREAM; // TCP

    //convert addr_info into socket_addr_info
    if(getaddrinfo(hostname, port, &addr_info, &addr_info_list) != 0){
        throw ProxyException("Fail to get socket address information.");
    }

    sockfd = socket(addr_info_list->ai_family,
                    addr_info_list->ai_socktype,
                    addr_info_list->ai_protocol);
    if(sockfd == -1){
        throw ProxyException("Fail to create the socket.");
    }
    
    socket_info sock_info;
    sock_info.socket_fd = sockfd;
    sock_info.info_list = addr_info_list;
    return sock_info;
}

// bind port and get listening fd
// accept client connection
void Proxy::proxy_bindListen_accept(){
    clientServer_info = create_socket("0.0.0.0", "12345");
    int opt = 1;
    if(setsockopt(clientServer_info.socket_fd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt))==-1){
        throw ProxyException("Fail to use port again\n");
    }
    
    if(bind(clientServer_info.socket_fd, clientServer_info.info_list->ai_addr, clientServer_info.info_list->ai_addrlen) == -1){
        throw ProxyException("Fail to bind to server IP to the socket");
    }

    if(listen(clientServer_info.socket_fd, 1000) == -1){
        throw ProxyException("Fail to convert socket_fd into listen_fd");
    }
    //Accept the request from client
    while(1){
        struct sockaddr_in clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);
        int temp_fd = accept(clientServer_info.socket_fd, (struct sockaddr*)&clientaddr, &clientaddr_len);

        //std::cout<<clientProxy_connectfd<<std::endl;
        
        if(temp_fd == -1){
            throw ProxyException("Fail to accept client's connection");
        }

        //use the port again
        int connect_opt = 1;
        if(setsockopt(temp_fd, SOL_SOCKET,SO_REUSEADDR, (const void *)&connect_opt, sizeof(connect_opt)) == -1){
            throw ProxyException("Fail to use port again\n");
        }

        //pthread_create
        pthread_t tid;
        void *a = NULL;
        int *fdPtr = new int;
        *fdPtr = temp_fd;
        pthread_create(&tid,NULL, Proxy::proxyServer, fdPtr);
        //线程分离
        pthread_detach(tid);
    }
    
}


void proxy_client(std::vector<char> cache_content){
    if(send(getThreadsClientFd(pthread_self()), cache_content.data(), cache_content.size(),0)==-1){
        throw ProxyException("Fail to send the cache content to the client");
    }
}

// thread: receive client request
//         parse request
//         send parsed request to web server
//         send back returned response to the client
void *Proxy::proxyServer(void *clientfd_ptr){
    try{
        int clientfd = *(int*)clientfd_ptr;
        delete (int*)clientfd_ptr;
        insertThreadsClientFd(pthread_self(), clientfd);
        //vector char
        char buffer[65536]="";
        ssize_t _recv = recv(getThreadsClientFd(pthread_self()), buffer, 65536, 0);
        if(_recv==-1){
            throw ProxyException("Fail to receive the message from client\n");
        }

        parse_request(buffer);
    }
    catch(CorruptedResponseException & e){
        std::cerr << "ERROR: "<< e.what() << std::endl;
        /// log: cached but need revalidation
        pthread_mutex_lock(&logfile_mutex);
        LOG_FILE << request_id << ": ERROR 502 Bad GateWay" << std::endl;
        pthread_mutex_unlock(&logfile_mutex);
        ///
        char error_message[100] = "HTTP/1.1 502 Bad GateWay\r\n\r\n";
        send(getThreadsClientFd(pthread_self()), error_message, 100, 0);
    }
    catch(NonFoundRequestMethodException & e){
        std::cerr << "ERROR: "<< e.what() << std::endl;
        /// log: cached but need revalidation
        pthread_mutex_lock(&logfile_mutex);
        LOG_FILE << request_id << ": ERROR 400 Bad Request" << std::endl;
        pthread_mutex_unlock(&logfile_mutex);
        ///
        char error_message[100] = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(getThreadsClientFd(pthread_self()), error_message, 100, 0);
    }
    catch(MalformedRequestException & e){
        std::cerr << "ERROR: "<< e.what() << std::endl;
    }
    catch(ProxyException & e){
        std::cerr <<"ERROR: " << e.what() << std::endl;
        /// close fd and host addrinfo
        close(getThreadsClientFd(pthread_self()));
        removeThreadsClientFd(pthread_self());
        exit(EXIT_FAILURE);
    }
    close(getThreadsClientFd(pthread_self()));
    removeThreadsClientFd(pthread_self());
    return NULL;
}


std::vector<char> proxyServer_serverClient(){
    //Send the message from the client to server through proxy
    socket_info serverClient_info = create_socket(addr_client,"80");
    
    int _connect = connect(serverClient_info.socket_fd, serverClient_info.info_list->ai_addr, serverClient_info.info_list->ai_addrlen);
    if(_connect==-1){
        throw ProxyException("Fail to connet with the server\n");
    }
    
    if(send(serverClient_info.socket_fd, proxy_send_server.c_str(), proxy_send_server.length(),0)==-1){
        throw ProxyException("Fail to send the message/n");
    }
    //send message from server to client through proxy
    char proxy_recv[65536] = "";
    std::vector<char> proxy_recv_mssg;


    while(1){
        //receive message from the server
        ssize_t recv_resp = recv(serverClient_info.socket_fd, proxy_recv, 65536,0);
        if(recv_resp == -1){
            throw  ProxyException("Fail to receive the message from the server\n");
        }
        if(recv_resp==0){
            break;
        }
        proxy_recv_mssg.insert(proxy_recv_mssg.end(), proxy_recv, proxy_recv+recv_resp);
        //send message to the client
        ssize_t sent_size = send(getThreadsClientFd(pthread_self()), proxy_recv, recv_resp, 0);
        if( sent_size == -1){
            std::cout << errno << ": "<< getThreadsClientFd(pthread_self()) << std::endl;
            throw  ProxyException("Proxy fail to send message to the client\n");
        }
    }
    return proxy_recv_mssg;
}


std::string parseHostName(std::string argument){
    size_t first_slash = argument.find("/");
    std::string first = argument.substr(first_slash+1);
    size_t second_slash = first.find("/");
    std::string second = first.substr(second_slash+1);
    size_t res_part = second.find("/");
    std::string argument_part = second.substr(res_part);

    //extract the domain name
    std::string domain_name = argument.substr(first_slash+2, res_part);
    return domain_name;
}

socket_info proxy_CONNECT_server(char *hostname, const char *port){
    socket_info proxyServer_info = create_socket(hostname, port);
    int _connect = connect(proxyServer_info.socket_fd, proxyServer_info.info_list->ai_addr, proxyServer_info.info_list->ai_addrlen);
    if(_connect==-1){
        throw ProxyException("Fail to connet with the server\n");
    }

    return proxyServer_info;
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
        std::cout << "domain name: " << domain_name << std::endl;
        //throw ProxyException("The domain name is wrong\n");
    }

    //Convert the domain name to IP address
    addr_client = inet_ntoa(*(struct in_addr*)host_name->h_addr_list[0]);
    return argument_part;
}


void parse_request(char *buffer){
    std::string parse_req = buffer;
    if(parse_req.find("\r\n\r\n") == std::string::npos){
        throw MalformedRequestException();
    }

    //parse the request, extract the first line
    std::string line = "\r\n";
    if(parse_req.find(line) == std::string::npos){
        throw MalformedRequestException();
    }
    size_t get = parse_req.find(line);

    //extract the first line of the client message(ex: GET www...... HTTP/1.1)
    std::string req_head = parse_req.substr(0, get);

    //extract the first part(GET, POST..)
    if(req_head.find_first_of(" ") == std::string::npos){
        throw MalformedRequestException();
    }
    size_t method = req_head.find_first_of(" ");

    std::string method_part = req_head.substr(0, method);
    //extract the last part(HTTP/1.1)
    if(req_head.find_last_of(" ") == std::string::npos){
        throw MalformedRequestException();
    }
    size_t version = req_head.find_last_of(" ");

    if(version == std::string::npos){
        throw MalformedRequestException();
    }

    std::string version_part = req_head.substr(version+1);
    //extract the middle part(www.cmu.edu)
    std::string argument = req_head.substr(method+1, version-method);

    if(method_part != "GET" && method_part != "POST" && method_part != "CONNECT"){
        request_id++;
        /// log: cached but need revalidation
            pthread_mutex_lock(&logfile_mutex);
            LOG_FILE << request_id << ": ERROR 400 bad request" << std::endl;
            pthread_mutex_unlock(&logfile_mutex);
        ///
        throw NonFoundRequestMethodException();
    }

    // hand different request method
    if(method_part == "GET"){
        ///parse the http://www.....
        std::string arg_part = GET_method(argument);
        std::string GET_method = method_part+" "+arg_part+version_part;
        std::string req_data = parse_req.substr(get);

        // raw request
        proxy_send_server = GET_method  + req_data;

        request_id++;
        Request request_obj(request_id, proxy_send_server, parseHostName(argument));

        /// log request info
        pthread_mutex_lock(&logfile_mutex);
        LOG_FILE << request_obj.getRequestId() << ": \"" << request_obj.getRequestLine()<< "\" from "
                << request_obj.getRequestHostInfo().first << " @ " << getCurrentTime() << std::endl;
        pthread_mutex_unlock(&logfile_mutex);
        ///
        std::cout << "||-> start check cache..." << std::endl;

             /************** get response from the cache **************/
        if(cache.isValidResponseStoredInCache(request_obj)){
            std::cout << "[In Cache]" << std::endl;

            Response cached_response = cache.getResponse(request_obj.getRequestUrl());
            proxy_client(cached_response.getReturnedResponse());
        }
            /************** get response from the server **************/
        else {
            /// log: request not in cache
            pthread_mutex_lock(&logfile_mutex);
            LOG_FILE << request_obj.getRequestId() << ": not in cache" << std::endl;
            pthread_mutex_unlock(&logfile_mutex);
            ///
            std::cout << "[Not In Cache]" << std::endl;

            // get the response from web server and sent it to the client;
            /// log: requesting from web server
            pthread_mutex_lock(&logfile_mutex);
            LOG_FILE << request_obj.getRequestId() << ": Requesting " << request_obj.getRequestLine() << " from " << request_obj.getRequestHostInfo().first << std::endl;
            pthread_mutex_unlock(&logfile_mutex);
            ///

            std::vector<char> raw_response = proxyServer_serverClient();
            Response response_obj(raw_response);

            /// log: received response from web server
            pthread_mutex_lock(&logfile_mutex);
            LOG_FILE << request_obj.getRequestId() << ": Received " << response_obj.getResponseLine() << " from " << request_obj.getRequestHostInfo().first << std::endl;
            pthread_mutex_unlock(&logfile_mutex);
            ///

            std::cout << "-successfully got response from web server" << std::endl;
            std::string not_cacheable_reason;
            if(response_obj.isCacheable(not_cacheable_reason)){
                cache.putResponse(request_obj.getRequestUrl(), response_obj);
                std::cout << "-save cacheable response into cache" << std::endl;

                /// log: cached with expire time
                if(!response_obj.getExpiredTime().empty()){
                    pthread_mutex_lock(&logfile_mutex);
                    LOG_FILE << request_obj.getRequestId() << ": cached, expires at " << response_obj.getExpiredTime() << std::endl;
                    pthread_mutex_unlock(&logfile_mutex);
                }
                ///
                /// log: cached without expire time
                else{
                    pthread_mutex_lock(&logfile_mutex);
                    LOG_FILE << request_obj.getRequestId() << ": cached" << std::endl;
                    pthread_mutex_unlock(&logfile_mutex);
                }
                ///

                /// log: cached but need revalidation
                if(response_obj.requireValidation()){
                    pthread_mutex_lock(&logfile_mutex);
                    LOG_FILE << request_obj.getRequestId() << ": cached, but requires re-validation" << std::endl;
                    pthread_mutex_unlock(&logfile_mutex);
                }
                ///
            }
            else{
                /// log: received response from web server
                pthread_mutex_lock(&logfile_mutex);
                LOG_FILE << request_obj.getRequestId() << "not cacheable because " << not_cacheable_reason << std::endl;
                pthread_mutex_unlock(&logfile_mutex);
                ///
            }
        }
        cache.printMap();
    }

    if(method_part == "POST"){
        //parse the http://www.....
        std::string arg_part = GET_method(argument);
        std::string GET_method = method_part+" "+arg_part+version_part;
        std::string req_data = parse_req.substr(get);
        proxy_send_server = GET_method  + req_data;

        request_id++;
        Request request_obj(request_id, proxy_send_server);
        /// log request info
        pthread_mutex_lock(&logfile_mutex);
        LOG_FILE << request_obj.getRequestId() << ": \"" << request_obj.getRequestLine() << "\" from "
                 << argument << " @ " << getCurrentTime() << std::endl;
        pthread_mutex_unlock(&logfile_mutex);
        ///

        std::cout<<"Receive from the client: "<<"\n"<<proxy_send_server<<std::endl;
        proxyServer_serverClient();
    }

    if(method_part == "CONNECT"){
        //parse the domain name and the port number
        //www.youtube.com:443
        size_t domain = argument.find(":");
        std::string domainName;
        if(domain!=std::string::npos){
            //extract the domain name
            domainName = argument.substr(0,domain);
            std::string portNumber = argument.substr(domain+1);
        }
        else{
            //there is no ":", there is port number
            domainName = argument;
        }
        request_id++;
        Request request_obj(request_id, parse_req);
        /// log request info
        pthread_mutex_lock(&logfile_mutex);
        LOG_FILE << request_obj.getRequestId() << ": \"" << request_obj.getRequestLine() << "\" from "
                 << argument << " @ " << getCurrentTime() << std::endl;
        pthread_mutex_unlock(&logfile_mutex);
        ///

        struct hostent *host_name = gethostbyname(domainName.c_str());
        if(!host_name){
            std::cout << "domain name: " << domainName << std::endl;
            //throw ProxyException("The domain name is wrong\n");
        }
        //Convert the domain name to IP address
        char *server_addr = inet_ntoa(*(struct in_addr*)host_name->h_addr_list[0]);

        socket_info proxyServer = proxy_CONNECT_server(server_addr, "443");
        process_connect(getThreadsClientFd(pthread_self()), proxyServer.socket_fd);

        /// log: tunnel close
        pthread_mutex_lock(&logfile_mutex);
        LOG_FILE << request_id << ": Tunnel closed" << std::endl;
        pthread_mutex_unlock(&logfile_mutex);
        ///
    }

}

std::string getCurrentTime(){
    time_t rawTime = time(0);
    struct tm * current_time = gmtime(&rawTime);
    std::string current_time_str(asctime(current_time));
    // check and delete the new line character
    size_t pos = current_time_str.find("\n");
    if (pos != std::string::npos){
        return current_time_str.substr(0, pos);
    }
    return current_time_str;
}

void process_connect(int client_fd, int server_fd) {
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    fd_set readfds;
    int nfds = std::max(server_fd, client_fd) + 1;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        FD_SET(client_fd, &readfds);

        select(nfds, &readfds, NULL, NULL, NULL);
        int fd[2] = {server_fd, client_fd};
        int len;
        for (int i = 0; i < 2; i++) {
            char message[65536] = {0};
            if (FD_ISSET(fd[i], &readfds)) {
                len = recv(fd[i], message, sizeof(message), 0);
                if (len <= 0) {
                    return;
                }
                else {
                    if (send(fd[1 - i], message, len, 0) <= 0) {
                        return;
                    }
                }
            }
        }
    }
}

