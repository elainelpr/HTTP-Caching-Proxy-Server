//
// Created by
//

#ifndef ECE568_HW2_MASTER_RESPONSE_H
#define ECE568_HW2_MASTER_RESPONSE_H


#include <vector>
#include <string>
#include <pair>
#include <unordered_map>



class Response {
private:
    std::string raw_response;
    std::string response_header;
    std::string response_body;
    size_t content_len;

    std::string response_line; //RESPONSE
    size_t status_code;
    // response header fields
    std::string header_Cache_Control;
    std::string header_ETag;
    std::string header_Expire;
    std::string header_Date;
    std::string header_Last_Modified;
    std::string header_Transfer_Encoding;


    // Used for caching
    std::pair<bool, std::string> cacheability; // (true, NULL), (false, "reason")
    std::string expiration_time;
    size_t max_age;
    size_t age;
    bool freshness;
    bool validness;

public:
    Response();
    void parseResponseLine();
    void parseHeaderFields();

    void updateAge();

    bool isFresh();
    bool isCacheable();
    bool requireValidation(); // if require validation from origin server
    std::string getHttpVersion();
    std::string getStatusCode();
    std::string getStatusMessage();

    //get Info for log file
    std::string getExpiredTime();
    std::string getResponseLine();
    std::string getServerName();
    std::string getUncachableReason();
};

#endif //ECE568_HW2_MASTER_RESPONSE_H
