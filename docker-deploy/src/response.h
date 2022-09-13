//
// Created by Peiru Liu
//

#ifndef ECE568_HW2_MASTER_RESPONSE_H
#define ECE568_HW2_MASTER_RESPONSE_H

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "global.h"
#include "proxyexception.h"


class Response {
private:
    std::string raw_response;
    std::vector<char> returned_response;
    std::string response_line; //RESPONSE
    std::string response_header;
    std::string response_body;
    size_t content_len;

    size_t status_code;
    // response header fields
    /// Do case-insensitive finding !
    std::unordered_map<std::string, std::pair<std::string, std::string>> header_map;
    std::string header_Cache_Control;
    std::string header_ETag;
    std::string header_Expire;
    std::string header_Date;
    std::string header_Last_Modified;
    std::string header_Transfer_Encoding;


    // Used for caching
    std::pair<bool, std::string> cacheability; // (true, NULL), (false, "reason")
    //std::string expiration_time;
    size_t max_age;
    size_t age;
    std::string url;

public:
    Response(std::vector<char> _response);
    std::string convertStrToLowerCase(std::string);
    void parseResponseLine();
    void parseResponseHeader();
    void parseMaxAge();
    std::string getResponseLine();
    std::string getResponseHeader();
    std::vector<char> getReturnedResponse();
    std::string getRawResponse();
    std::string getRequestUrl();


    void resetAge();
    void updateAge();
    void updateExpire(std::pair<std::string, std::string> _expire_header);
    void updateDate(std::pair<std::string, std::string> _date_header);
    void updateCacheControl(std::pair<std::string, std::string> _cache_control_header);
    void setUrl(std::string);

    bool isFresh();
    bool isCacheable(std::string & reaseon);
    bool requireValidation(); // if require validation from origin server
    size_t getStatusCode();
    std::string getHttpVersion();
    std::string getStatusMessage();
    std::string getResponseFieldValue(std::string field_name);
    std::pair<std::string, std::string> getResponseHeaderFields(std::string field_name);

    //get Info for log file
    std::string getExpiredTime();
    std::string getServerName();
    std::string getUncachableReason();
};

#endif //ECE568_HW2_MASTER_RESPONSE_H
