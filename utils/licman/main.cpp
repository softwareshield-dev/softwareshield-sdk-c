#include <cxxopts.hpp>
#include <iostream>

#include <GS5.h>

int main(int argc, char *args[]) {
    cxxopts::Options opt("licman", "SoftwareShield License Management Utility");

    opt.add_options()("h,help", "Print usage");

    auto result = opt.parse(argc, args);

    if (result.arguments().empty() || result.count("help")) {
        std::cout << opt.help() << std::endl;
    }

    return 0;
}