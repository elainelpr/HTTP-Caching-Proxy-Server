//
// Created by Xinyu
//

#include "cache.h"
#include "global.h"

void Cache::moveToFirst(std::string url, Response res){
    pthread_mutex_lock(&map_mutex);
    auto& resp_it = map[url];
    pthread_mutex_unlock(&map_mutex);

    pthread_mutex_lock(&list_mutex);
    // delete url from doubly_list
    doubly_list.erase(resp_it);
    // add response to the beginning of the list
    doubly_list.push_front(res);
    auto list_begin_it = doubly_list.begin();
    pthread_mutex_unlock(&list_mutex);

    // update its iterator in the map
    pthread_mutex_lock(&map_mutex);
    map[url] = list_begin_it;
    pthread_mutex_unlock(&map_mutex);
}

int buildClientSocket(const char *hostname, const char *port){
    int sockfd;
    struct addrinfo addr_info;
    struct addrinfo *addr_info_list;

    memset(&addr_info, 0, sizeof(addr_info));
    addr_info.ai_family = AF_INET; // IPv4
    addr_info.ai_socktype = SOCK_STREAM; // TCP

    //convert addr_info into socket_addr_info
    if(getaddrinfo(hostname, port, &addr_info, &addr_info_list) != 0){
        perror("Fail to get socket address information.\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(addr_info_list->ai_family,
                    addr_info_list->ai_socktype,
                    addr_info_list->ai_protocol);
    if(sockfd == -1){
        perror("Fail to create the socket.");
        exit(EXIT_FAILURE);
    }

    int status = connect(sockfd, addr_info_list->ai_addr, addr_info_list->ai_addrlen);
    if(status == -1){
        perror("Fail to connect to the server\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(addr_info_list);
    return sockfd;
}

void Cache::deleteLast() {
    pthread_mutex_lock(&list_mutex);
    auto list_back_it = doubly_list.back();
    doubly_list.pop_back();
    pthread_mutex_unlock(&list_mutex);

    pthread_mutex_lock(&map_mutex);
    map.erase(list_back_it.getRequestUrl());
    pthread_mutex_unlock(&map_mutex);

    available_capacity++;
}

std::vector<char> Cache::sendRevalidationRequest(Request &req) {
    int sockfd = buildClientSocket(req.getRequestHostInfo().first.c_str(), req.getRequestHostInfo().second.c_str());

    if(send(sockfd, req.getRevalidationRequest().c_str(), req.getRevalidationRequest().length(), 0) == -1){
        perror("Fail to send the message/n");
        exit(EXIT_FAILURE);
    }

    std::vector<char> received_response;
    char buffer[65536] = "";
    while(1){
        memset(buffer, 0, sizeof(buffer));
        size_t received_size = recv(sockfd, buffer, 65536, 0);
        if(received_size == -1){
            perror("Fail to receive the message from the server\n");
            exit(EXIT_FAILURE);
        }
        else if(received_size == 0){
            break;
        }
        else {
            received_response.insert(received_response.end(), buffer, buffer+received_size);
        }
    }
    close(sockfd);
    return received_response;
}

void Cache::revalidateToServer(Request revalidate_request, Response cached_response){
    //insert if_non_match and if_modified_since into request header
    revalidate_request.addRevalidationInfo(cached_response.getResponseFieldValue("etag"), cached_response.getResponseFieldValue("last-modified"));

    // returned revalidation response
    std::vector<char> returned_raw_data = sendRevalidationRequest(revalidate_request);
    Response returned_response (returned_raw_data);

    /// print out returned revalidation response
    std::cout << ">--------------revalidation response--------------<" <<std::endl;
    std::cout << returned_response.getRawResponse() <<std::endl;
    std::cout << ">-------------------------------------------------<" <<std::endl;

    /// the cached response is valid
    if(returned_response.getStatusCode() == 304){
        // set Age to zero
        cached_response.resetAge();
        // update date if it has
        if(returned_response.getResponseFieldValue("date") != ""){
            cached_response.updateDate(returned_response.getResponseHeaderFields("date"));
        }
        // update expire if it has
        if(returned_response.getResponseFieldValue("expire") != ""){
            cached_response.updateDate(returned_response.getResponseHeaderFields("expire"));
        }
        // update cache-control if it has
        if(returned_response.getResponseFieldValue("cache-control") != ""){
            cached_response.updateDate(returned_response.getResponseHeaderFields("cache-control"));
        }
        moveToFirst(revalidate_request.getRequestUrl(), cached_response);
    }

    /// the cached response is invalid
    // update it with new received response
    if(returned_response.getStatusCode() == 200){
        putResponse(revalidate_request.getRequestUrl(), returned_response);
    }
}

/********************************************
 *               API for Cache              *
 *******************************************/

Cache::Cache() : available_capacity(MAX_CACHE_CAPACITY){
}

/// call for cacheable Response
void Cache::putResponse(std::string url, Response http_response) {
    // if valid response is stored in the cache and valid
    if(map.find(url) != map.end()){
        // update if required
        *map[url] = http_response;
        moveToFirst(url, http_response);
    }
    // else add it and http_response to the cache
    else{
        http_response.setUrl(url);
        doubly_list.push_front(http_response);
        map[url] = doubly_list.begin();
        available_capacity --;
    }
    // if available capacity of cache < 0, delete the last one
    if (available_capacity < 0){
        deleteLast();
    }
}

/// Call for GET Request
bool Cache::isValidResponseStoredInCache(Request http_request) {

    pthread_mutex_lock(&map_mutex);
    bool in_cache = map.count(http_request.getRequestUrl()) > 0;
    pthread_mutex_unlock(&map_mutex);

    // if the required response has been stored in cache
    if(in_cache){
        pthread_mutex_lock(&map_mutex);
        Response stored_response = *map[http_request.getRequestUrl()];
        pthread_mutex_unlock(&map_mutex);
        // if require revalidation
        if(stored_response.requireValidation()){
            /// log
            pthread_mutex_lock(&logfile_mutex);
            LOG_FILE << http_request.getRequestId() << ": in cache, requires validation" << std::endl;
            pthread_mutex_unlock(&logfile_mutex);
            ///
            revalidateToServer(http_request, stored_response);
        }
        else{
            /// log
            pthread_mutex_lock(&logfile_mutex);
            LOG_FILE << http_request.getRequestId() <<  std::endl;
            pthread_mutex_unlock(&logfile_mutex);
            ///
        }
        return true;
    }
    return false;
}

// Call for isValidResponseStoredInCache(url) == true
Response Cache::getResponse(std::string url) {
    std::cout << "-> get response from cache" << std::endl;
    pthread_mutex_lock(&map_mutex);
    Response res = *map[url];
    pthread_mutex_unlock(&map_mutex);

    moveToFirst(url, res);
    return res;
}

void Cache::printMap(){
   std::cout << "<------------cache map layout---------" << std::endl;
    for (auto const& item : map){
        std::cout << "request url: " << (*item.second).getRequestUrl() << std::endl;
        std::cout << "response line: " << (*item.second).getResponseLine() << std::endl;
        std::cout << "---------" << std::endl;
    }
    std::cout << "-------------------------------------->" << std::endl;
}
