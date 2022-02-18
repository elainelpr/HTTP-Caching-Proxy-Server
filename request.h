//
// Created by Xinyu
//

#ifndef ECE568_HW2_MASTER_REQUEST_H
#define ECE568_HW2_MASTER_REQUEST_H

#include <vector>
#include <string>
#include <unordered_map>

class Request {
private:
    int id; //ID
    std::string raw_request;
    std::string request_header;
    std::string request_body;
    size_t content_len;

    //e.g. GET /index.html or CONNECT www.example.com:443 HTTP/1.1
    std::string request_line; //REQUEST (HTTP request line)

    std::string method; // GRT/POST/CONNECT
    std::string url; //e.g. https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
    std::string path; //e.g. /en-US/docs/Web/HTTP/Messages
    std::string version;
    std::string port;

    std::string header_Host;
    std::string header_If_Modified_Since;
    std::string header_If_None_Match;


    std::string received_time; //TIME (request received time)
    std::string request_sender_ip; //IPFROM (request sender's address)

    bool hasBody

public:
    Request();
    void parseRequestLine();
    void parseRequestBody();
};


#endif //ECE568_HW2_MASTER_REQUEST_H
