#ifndef LICMAN_HELPERS_H_
#define LICMAN_HELPERS_H_

#include "rang.hpp"

#include <ctime>

template<typename T>
inline int length(T v){
    return (int)sizeof(T);
}

template<> inline int length(const char* v){
    return (int)strlen(v);
}
template<> inline int length(std::string v){
    return (int)v.size();
}

#define H1(hdr) std::endl << hdr << std::endl << std::string(length(hdr), '*') << std::endl << std::endl
#define H2(hdr) std::endl << hdr << std::endl << std::string(length(hdr), '=') << std::endl << std::endl
#define H3(hdr) std::endl << hdr << std::endl << std::string(length(hdr), '-') << std::endl

#define HR std::endl << std::endl << std::endl
#define BR std::endl
#define PR std::endl << std::endl

#define KEYWORD(key) rang::style::bold << (key) << rang::style::reset

#define ERROR(msg) rang::style::blink << rang::fg::red << (msg) << rang::fg::reset << rang::style::reset

namespace licman {
    //render helpers
}

#endif