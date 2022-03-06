//
// Created by Xinyu
//

#ifndef ECE568_HW2_MASTER_REQUEST_H
#define ECE568_HW2_MASTER_REQUEST_H

#include <vector>
#include <string>
#include <sstream>
#include <locale>
#include <utility>
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include "proxyexception.h"


class Request {
private:
    int id;
    std::string raw_request;
    std::string request_header;
    std::string request_body;
    //size_t content_len;

    //e.g. GET /index.html or CONNECT www.example.com:443 HTTP/1.1
    std::string request_line; //REQUEST (HTTP request line)

    std::string method; // GRT/POST/CONNEC T
    std::string url; //e.g. https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
    //std::string path; //e.g. /en-US/docs/Web/HTTP/Messages
    std::string version;
    std::string host_ip;
    std::string host_port;

    std::unordered_map<std::string, std::pair<std::string, std::string>> header_map;
    std::string header_Host;
    std::string header_If_Modified_Since;
    std::string header_If_None_Match;


    std::string received_time; //TIME (request received time)
    std::string request_sender_ip; //IPFROM (request sender's address)

    //bool hasBody;

public:
    Request(int _id, std::string _raw_request, std::string _host_ip);
    Request(int _id, std::string _raw_request);
    std::string convertStrToLowerCase(std::string);
    void parseRequestLine();
    void parseRequestHeader();
    void addRevalidationInfo(std::string etag, std::string last_modified);

    int getRequestId();
    std::string getRawRequest();
    std::string getRequestUrl();
    std::string getRequestLine();
    std::string getRequestHeader();
    std::string getRevalidationRequest();
    std::pair<std::string, std::string> getRequestHostInfo(); // (IP, port)

    ~Request();
};


#endif //ECE568_HW2_MASTER_REQUEST_H
