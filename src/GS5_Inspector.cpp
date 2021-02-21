#include "GS5_Inspector.h"

#include <iomanip>

namespace gs {

std::string format_time_t(std::time_t t, const char *format) {
    std::ostringstream os;
    os << std::put_time(std::gmtime(&t), format);
    return os.str();
}

std::string to_simple_string(time_point_t tp) {
    return format_time_t(std::chrono::system_clock::to_time_t(tp), "%Y-%h-%d %H:%M:%S");
}

std::string to_iso_string(time_point_t tp) {
    return format_time_t(std::chrono::system_clock::to_time_t(tp), "%Y%m%dT%H%M%S");
}
} // namespace gs