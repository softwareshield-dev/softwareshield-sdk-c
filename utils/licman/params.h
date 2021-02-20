#ifndef LICMAN_PARAMS_H_
#define LICMAN_PARAMS_H_

#include <filesystem>
#include <string>

namespace licman {
extern std::string productId;         // unique product-id
extern std::string password;          // license password
extern std::filesystem::path origLic; //path to original license
} // namespace licman

#endif