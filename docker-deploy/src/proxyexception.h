//
// Created by 郭欣瑜 on 2/23/22.
//

#ifndef HW2_FINAL_EXCEPTION_H
#define HW2_FINAL_EXCEPTION_H

#include <exception>
#include <string>

class CorruptedResponseException: public std::exception{
public:
    const char * what () const throw () override{
        return "Corrupted response got from web server\n";
    }
};

class NonFoundRequestMethodException: public std::exception{
public:
    const char * what () const throw () override{
        return "Unsupport request method\n";
    }
};

class MalformedRequestException: public std::exception{
public:
    const char * what () const throw () override{
        return "Invalid request format\n";
    }
};

class ProxyException : public std::exception{
private:
    std::string error_message;
public:
    ProxyException(const std::string & _error_message):error_message(_error_message){
    }
    virtual const char * what () const noexcept override{
        return error_message.c_str();
    }
};




#endif //HW2_FINAL_EXCEPTION_H
