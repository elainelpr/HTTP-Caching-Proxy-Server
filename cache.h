//
// Created by
//

#ifndef ECE568_HW2_MASTER_CACHE_H
#define ECE568_HW2_MASTER_CACHE_H


#include <list>
#include <string>
#include <unordered_map>
#include "response.h"
#include "request.h"

#define MAX_CACHE_CAPACITY 1048576
#define MAX_CACHE_ITEM_SIZE 102400

class Cache {
private:
    // key: url, value: (url_list::iterator, Response)
    std::unordered_map<std::string, std::pair<std::list<url>::iterator, Response >> map;
    std::list<std::string> doubly_list; // list of url
    int available_capacity;
    size_t read_count; //Add read and write mutex later

    void moveToFirst(std::string url);
    void deleteLast();
    void revalidateToServer(Request req); // change to server side
    void sendRevalidationRequest(Request req);
    void handleValidationResponse();

public:
    Cache();
    bool isValidResponseStoredInCache(Request http_request);
    void putResponse(std::string url, Response http_response);
    Response getResponse(std::string url);
};


#endif //ECE568_HW2_MASTER_CACHE_H
