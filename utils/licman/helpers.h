#ifndef LICMAN_HELPERS_H_
#define LICMAN_HELPERS_H_

#include "rang.hpp"

#define KEYWORD(key) rang::style::bold << (key) << rang::style::reset

#define ERROR(msg) rang::style::blink << rang::fg::red << (msg) << rang::fg::reset << rang::style::reset

namespace licman {
    //render helpers
}

#endif