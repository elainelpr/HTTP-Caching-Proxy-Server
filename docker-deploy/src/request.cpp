//
// Created by Xinyu
//

#include "request.h"

std::string Request::convertStrToLowerCase(std::string str_in){
    std::stringstream res;
    std::locale loc;
    for (std::string::size_type i=0; i<str_in.length(); ++i){
        res << std::tolower(str_in[i],loc);
    }
    return res.str();
}

void Request::addRevalidationInfo(std::string etag, std::string last_modified) {
    // add Etag
    if(!etag.empty()){
        // check if header have had etag
        std::stringstream ss;
        if(header_map.count("if-none-match") > 0){
            ss << ", " << etag;
            header_map["if-none-match"].second += ss.str();
        }
        else {
            header_map["if-none-match"] = std::make_pair("If-None-Match", etag);
        }
    }
    // add Last-Modified
    if(!last_modified.empty()){
        // check if header have had etag
        std::stringstream ss;
        if(header_map.count("if-modified-since") > 0){
            ss << ", " << last_modified;
            header_map["if-modified-since"].second += ss.str();
        }
        else {
            header_map["if-modified-since"] = std::make_pair("If-Modified-Since", last_modified);
        }
    }
}

std::string Request::getRevalidationRequest() {
    std::string revalidate_request_header;
    for(auto const& header : header_map){
        revalidate_request_header += header.second.first + ": " + header.second.second + "\r\n";
    }
    std::string revalidate_request = this->request_line + "\r\n" + revalidate_request_header + "\r\n";
    return revalidate_request;
}

void Request::parseRequestLine() {
    std::stringstream ss;
    ss << this->request_line;
    ss >> this->method >> this->url >> this->version;
}

void Request::parseRequestHeader() {
    std::string text = this->request_header;
    std::string delim = "\r\n";
    size_t pos = 0;

    while ((pos = text.find(delim)) != std::string::npos){
        // get each field line
        std::string field = text.substr(0, pos);

        // save headers' field name and field value into the header map
        size_t comma_pos = field.find(":");
        if(comma_pos == std::string::npos){
        }
        std::string field_name = field.substr(0, comma_pos);
        std::string field_value = field.substr(comma_pos+2);
        std::string field_name_lowercase = convertStrToLowerCase(field_name);
        header_map[field_name_lowercase] = std::make_pair(field_name, field_value);

        text.erase(0, pos + delim.length());

        // reach the end of the request header -> detect an empty line
        if(text.compare("\r\n") == 0){
            break;
        }
    }
}

Request::Request(int _id, std::string _raw_request):id(_id), raw_request(_raw_request) {
    /// separate request line and header
    std::string delim = "\r\n";
    size_t pos = 0;
    if ((pos = this->raw_request.find(delim)) == std::string::npos) {
        throw MalformedRequestException();
    }
    this->request_line = this->raw_request.substr(0, pos);
}

Request::Request(int _id, std::string _raw_request, std::string _host_ip): id(_id), raw_request(_raw_request), host_ip(_host_ip), host_port("80"){
    /// separate request line and header and body
    size_t pos_end_line = 0;
    if ((pos_end_line = this->raw_request.find("\r\n")) == std::string::npos) {
        throw MalformedRequestException();
    }

    size_t pos_end_header = 0;
    if ((pos_end_header = this->raw_request.find("\r\n\r\n")) == std::string::npos) {
        throw MalformedRequestException();
    }

    this->request_line = this->raw_request.substr(0, pos_end_line);
    this->request_header = this->raw_request.substr(pos_end_line + 2, pos_end_header-pos_end_line-2);
    this->request_body = this->raw_request.substr(pos_end_header+4);

    /// parse request line
    parseRequestLine();
    /// parse request header
    parseRequestHeader();
    /// check if contain host field
    if(header_map.count("host") == 0){
        throw MalformedRequestException();
    }
}

int Request::getRequestId() {
    return this->id;
}

std::string Request::getRequestLine() {
    return this->request_line;
}

std::string Request::getRequestHeader() {
    return this->request_header;
}

std::string Request::getRawRequest() {
    return this->raw_request;
}

std::string Request::getRequestUrl(){
    return this->url;
}

std::pair<std::string, std::string> Request::getRequestHostInfo(){
    return std::make_pair(this->host_ip, this->host_port);
}

Request::~Request(){
}