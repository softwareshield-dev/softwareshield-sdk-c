#include <cxxopts.hpp>
#include <iostream>

#include <GS5.h>
using namespace gs;

int main(int argc, char *args[]) {
    cxxopts::Options opt("licman", "SoftwareShield License Management Utility");

    opt.add_options()("h,help", "Print usage")("version", "show SDK version");

    auto result = opt.parse(argc, args);

    if (result.arguments().empty() || result.count("help")) {
        std::cout << opt.help() << std::endl;
        return 0;
    }

    try {
        if (result.count("version")) {
            std::cout << "SDK version: " << TGSCore::SDKVersion() << std::endl;
            return 0;
        }
        return 0;
    } catch (std::exception &ex) {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }

    return -1;
}