//
// Created by Peiru
//

#include "response.h"
#include <time.h>
#include<stdio.h>
#include <sstream>
#include <cstdlib>

/*
 *  time process functions
 */


// e.g. time_str: Wed, 21 Oct 2015 07:28:00 GMT
struct tm parseTimestampStrToStructTm(std::string time_str){
    struct tm t;
    // month, range 0 to 11
    std::unordered_map<std::string, int> month_map;
    month_map["Jan"] = 0; month_map["Feb"] = 1;  month_map["Mar"] = 2;
    month_map["Apr"] = 3; month_map["May"] = 4;  month_map["Jun"] = 5;
    month_map["Jul"] = 6; month_map["Aug"] = 7;  month_map["Sep"] = 8;
    month_map["Oct"] = 9; month_map["Nov"] = 10; month_map["Dec"] = 11;

    // week, range 1 to 7
    std::unordered_map<std::string, int> week_map;
    week_map["Mon"] = 1; week_map["Tue"] = 2; week_map["Wed"] = 3;
    week_map["Thu"] = 4; week_map["Fri"] = 5; week_map["Sat"] = 6;
    week_map["Sun"] = 7;

    //separate timestamp into each part
    std::stringstream ss(time_str);
    std::string week, month, time, timezone;
    int date, year;
    int hour, minute, second;

    ss >> week >> date >> month >> year >> time >> timezone;
    hour = atoi(time.substr(0, 2).c_str());
    minute = atoi(time.substr(3, 2).c_str());
    second = atoi(time.substr(6, 2).c_str());

    // day of the week, range 0 to 6
    t.tm_wday = week_map[week.substr(0, 3)];
    // day of the month, range 1 to 31
    t.tm_mday = date;
    // day of the month, range 1 to 31
    t.tm_mon = month_map[month];
    // The number of years since 1900
    t.tm_year = year - 1900;
    // seconds,  range 0 to 59
    t.tm_sec = second;
    // minutes, range 0 to 59
    t.tm_min = minute;
    // hours, range 0 to 23
    t.tm_hour = hour;

    return t;
};


// timestamp - current_time
double computeTimestampDiffWithCurrentTime(std::string s){
    time_t current_time = time(NULL);
    struct tm t = parseTimestampStrToStructTm(s);
    return difftime(mktime(&t), current_time) + 18000;
}


// current_time - timestamp
double computeCurrentTimeDiffWithTimestamp(std::string s){
    time_t current_time = time(NULL);
    struct tm t = parseTimestampStrToStructTm(s);
    return difftime(current_time, mktime(&t)) - 18000;
}


std::string Response::convertStrToLowerCase(std::string str_in){
    std::stringstream res;
    std::locale loc;
    for (std::string::size_type i=0; i<str_in.length(); ++i){
        res << std::tolower(str_in[i],loc);
    }
    return res.str();
}


// age = current_time  - date
///what if there is no Date header?
void Response::updateAge() {
    this->age = computeCurrentTimeDiffWithTimestamp(this->header_Date);
}


void Response::resetAge() {
    this->age = 0;
}


void Response::updateDate(std::pair<std::string, std::string> _date_header) {
//    time_t current_time = time(NULL);
//    struct tm * time_info = localtime (&current_time);
//    std::string date_str(asctime(time_info));
    this->header_Date = _date_header.second;
    header_map["date"] = _date_header;
}

void Response::updateExpire(std::pair<std::string, std::string> _expire_header) {
    this->header_Expire = _expire_header.second;
    header_map["expire"] = _expire_header;
}

void Response::updateCacheControl(std::pair<std::string, std::string> _cache_control_header) {
    this->header_Cache_Control = _cache_control_header.second;
    header_map["cache-control"] = _cache_control_header;
}


bool Response::isFresh() {
    //if max-age exists in cache-control && "Date" exist in the header field
    if(header_Cache_Control.find("max-age") != std::string::npos && header_map.count("date") > 0){
        updateAge();
        //if age < max_age -> fresh
        if(age < max_age){
            return true;
        }
    }
    //else if Expire exists in response header
    if(!header_Expire.empty()){
        //if Expire - current_time > 0 -> fresh
        if(computeTimestampDiffWithCurrentTime(header_Expire) > 0){
            return true;
        }
    }
    return false;
}


// response is cacheable if: 200 ok + ! no-store + ! private
bool Response::isCacheable(std::string & reason) {
    if(status_code == 200){
        if(header_Cache_Control.find("no-store") == std::string::npos){
            if(header_Cache_Control.find("private") == std::string::npos){
                return true;
            }
            else{
                reason = "is private";
            }
        }
        else{
            reason = "is no-store";
        }
        reason = "not 200 OK";
    }
    return false;
}


// cached response requires revalidate, if
// 1. NO cache-control header
// 2. HAVE no-cache
// 3. expired
bool Response::requireValidation() {
    if(header_Cache_Control.empty()){
        return true;
    }
    else if(header_Cache_Control.find("no-cache") != std::string::npos){
        return true;
    }
    else if(!isFresh()){
        return true;
    }
    return false;
}


void Response::parseResponseLine(){
    std::stringstream ss;
    ss << this->response_line;
    std::string http_version;
    std::string reason_phrase;
    ss >> http_version >> this->status_code >> reason_phrase;
}


void Response::parseResponseHeader() {
    std::string text = this->response_header;
    std::string delim = "\r\n";
    size_t pos = 0;

    while ((pos = text.find(delim)) != std::string::npos){
        // get each field line
        std::string field = text.substr(0, pos);

        // save headers' field name and field value into the header map
        if(field.find(":") == std::string::npos){
            throw CorruptedResponseException();
        }
        size_t comma_pos = field.find(":");

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


void Response::parseMaxAge(){
    size_t pos = 0;
    std::string max_age_str;

    if((pos = this->header_Cache_Control.find("max-age")) != std::string::npos){
        size_t pos_end = 0;

        if((pos_end = this->header_Cache_Control.find(",", pos)) != std::string::npos){
            max_age_str = this->header_Cache_Control.substr(pos+8, pos_end-pos-8);
        }
        else{
            max_age_str = this->header_Cache_Control.substr(pos+8);
        }

        this->max_age = atoi(max_age_str.c_str());
    }
}


Response::Response(std::vector<char> _response): returned_response(_response), max_age(0), age(0) {
    std::string temp(_response.begin(), _response.end());
    raw_response = temp;
    /// separate request line and header and body
    size_t pos_end_line = 0;
    if ((pos_end_line = this->raw_response.find("\r\n")) == std::string::npos) {
        throw CorruptedResponseException();
    }

    size_t pos_end_header = 0;
    if ((pos_end_header = this->raw_response.find("\r\n\r\n")) == std::string::npos) {
        throw CorruptedResponseException();
    }

    this->response_line = this->raw_response.substr(0, pos_end_line);
    this->response_header = this->raw_response.substr(pos_end_line + 2, pos_end_header-pos_end_line-2);
    this->response_body = this->raw_response.substr(pos_end_header+4);

    /// parse response line
    parseResponseLine();

    /// parse response header
    parseResponseHeader();
    if(header_map.count("cache-control") > 0){
        this->header_Cache_Control = header_map["cache-control"].second;
    }
    if(header_map.count("etag") > 0){
        this->header_ETag = header_map["etag"].second;
    }
    if(header_map.count("expire") > 0){
        this->header_Expire = header_map["expire"].second;
    }
    if(header_map.count("date") > 0){
        this->header_Date = header_map["date"].second;
    }
    if(header_map.count("last-modified") > 0){
        this->header_Last_Modified = header_map["last-modified"].second;
    }
    if(header_map.count("transfer-encoding") > 0){
        this->header_Transfer_Encoding = header_map["transfer-encoding"].second;
    }
}


void Response::setUrl(std::string _url) {
    this->url = _url;
}


size_t Response::getStatusCode() {
    return this->status_code;
}


std::string Response::getResponseFieldValue(std::string field_name){
    if(this->header_map.count(field_name) == 0){
        return "";
    }
    return this->header_map[field_name].second;
}

std::pair<std::string, std::string> Response::getResponseHeaderFields(std::string field_name){
    return this->header_map[field_name];
}

std::string Response::getRawResponse(){
    return this->raw_response;
}

std::vector<char> Response::getReturnedResponse() {
    return this->returned_response;
}


std::string Response::getResponseLine() {
    return this->response_line;
}


std::string Response::getResponseHeader() {
    return this->response_header;
}


std::string Response::getRequestUrl(){
    return this->url;
}

std::string Response::getExpiredTime() {
    return this->header_Expire;
}
