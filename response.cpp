//
// Created by Xinyu
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
    return difftime(mktime(&t), current_time);
}

// current_time - timestamp
double computeCurrentTimeDiffWithTimestamp(std::string s){
    time_t current_time = time(NULL);
    struct tm t = parseTimestampStrToStructTm(s);
    return difftime(current_time, mktime(&t));
}

// age = current_time  - date
///what if there is no Date header?
void Response::updateAge() {
    this->age = computeCurrentTimeDiffWithTimestamp(this->header_Date);
}

bool Response::isFresh() {
    //if max-age exists in cache-control
    if(header_Cache_Control.find("max-age") != std::string::npos){
        updateAge();
        //if age < max_age -> fresh
        if(age < max_age){
            return true;
        }
    }
    //else if Expire exists in response header
    else if(!header_Expire.empty()){
        //if Expire - current_time > 0 -> fresh
        if(computeTimestampDiffWithCurrentTime(header_Expire) > 0){
            return true;
        }
    }
    else{
        return false;
    }


}

// response is cacheable if: 200 ok + ! no-store + ! private
bool Response::isCacheable() {
    if(status_code == 200){
        if(header_Cache_Control.find("no-store") == std::string::npos){
            if(header_Cache_Control.find("private") == std::string::npos){
                return true;
            }
        }
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

Response::Response(): age(0) {

}