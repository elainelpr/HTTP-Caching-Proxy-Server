//
// Created by Xinyu
//

#include "cache.h"

Cache::Cache() : available_capacity(MAX_CACHE_CAPACITY), read_count(0){
}

void Cache::moveToFirst(std::string url) {
    // delete url from doubly_list
    doubly_list.erase(map[url].first);
    // add url to the beginning of the list
    doubly_list.push_front(url);
    // update its iterator in the map
    map[url].first = doubly_list.begin();
}

void Cache::deleteLast() {
    Response http_response = map[doubly_list.back()].second;
    map.erase(doubly_list.back());
    available_capacity += (int)http_response.content_len;
    doubly_list.pop_back();
}

void Cache::revalidateToServer(Request req){
    //insert if_non_match and if_modified_since into request header

}
/********************************************
 *               API for Cache              *
 *******************************************/

// Call for 200 OK Response
void Cache::putResponse(std::string url, Response http_response) {
    // if valid response is stored in the cache and valid
    if(map.find(url) != map.end()){
        // update if required
        map[url].second = http_response;
        moveToFirst(url);
    }
    // else add it and http_response to the cache
    else{
        doubly_list.push_front(url);
        map[url] = {doubly_list.begin(), http_response};
        available_capacity -= (int)http_response.content_len;
    }

    // if available capacity of cache < 0
    while (available_capacity < 0){
        deleteLast();
    }
}

// Call for GET Request
bool Cache::isValidResponseStoredInCache(Request http_request) {
    // if response stored in cache
    if(map.find(http_request.url) != map.end()){
        Response &stored_response = map[http_request.url].second;
        // if require revalidation
        if(stored_response.requireValidation()){
            revalidateToServer(http_request);
        }
        return true;
    }
    return false;
}

// Call for isValidResponseStoredInCache(url) == true
Response Cache::getResponse(std::string url) {
    moveToFirst(url);
    return map[url].second;
}
