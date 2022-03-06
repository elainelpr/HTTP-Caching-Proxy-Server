//
// Created by
//

#ifndef ECE568_HW2_MASTER_CACHE_H
#define ECE568_HW2_MASTER_CACHE_H


#include <list>
#include <string>
#include <unordered_map>
#include <fstream>
#include <atomic>
#include "response.h"
#include "request.h"

#define MAX_CACHE_CAPACITY 20


class Cache {
private:
    // key: url, value: response_list::iterator
    std::unordered_map<std::string, std::list<Response>::iterator> map;
    std::list<Response> doubly_list; // list of Response
    std::atomic_int available_capacity;
    pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

    void moveToFirst(std::string url, Response res);
    void deleteLast();
    void revalidateToServer(Request revalidate_request, Response cached_response); // change to server side
    void handleValidationResponse(Response res);
    std::vector<char> sendRevalidationRequest(Request &req);

public:
    Cache();
    bool isValidResponseStoredInCache(Request http_request);
    void putResponse(std::string url, Response http_response);
    Response getResponse(std::string url);
    void printMap();
};




#endif //ECE568_HW2_MASTER_CACHE_H
