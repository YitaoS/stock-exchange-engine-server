#ifndef __ENGINE_EXCEPTION_H__
#define __ENGINE_EXCEPTION_H__

#include <exception>
#include <stdexcept>
#include <string>

class engine_exception : public std::exception {
public:
    std::string err;
    engine_exception(const std::string & err_message, const std::string &info){
        err.append(err_message);
        err.append(" ");
        err.append(info);
        err.append("\n");
    }
    virtual const char * what() const throw() {
        return err.c_str();
    }
};

#endif