#include <cxxopts.hpp>
#include <iostream>

#include "cmd-status.h"
#include "params.h"
#include "helpers.h"

#include <GS5.h>
using namespace gs;


int main(int argc, char *args[]) {
    cxxopts::Options opt("licman", "SoftwareShield License Management Utility");

    opt.add_options()("h,help", "Print usage")("v,verbose", "show more details")("version", "show SDK version")
        //common params
        ("productid", "product-id of the license data", cxxopts::value<std::string>())("password", "password to decode license data", cxxopts::value<std::string>())("origlic", "path to original compiled license file (*.lic)", cxxopts::value<std::string>())("s,status", "show current license status");

    auto result = opt.parse(argc, args);

    if (result.arguments().empty() || result.count("help")) {
        std::cout << opt.help() << std::endl;
        return 0;
    }

    if (result.count("verbose")) {
        licman::verbose = true;
    }

    try {
        if (result.count("version")) {
            std::cout << KEYWORD("SDK version: ") << TGSCore::SDKVersion() << std::endl;
            return 0;
        }

        // parse basic product parameters
        if (result.count("productid")) {
            licman::productId = result["productid"].as<std::string>();
        }
        if (result.count("password")) {
            licman::password = result["password"].as<std::string>();
        }
        if (result.count("origlic")) {
            licman::origLic = result["origlic"].as<std::string>();
            if (!std::filesystem::exists(licman::origLic))
                throw std::invalid_argument("original license file cannot be found!");
        }
        //show license information
        if (result.count("status")) {
            return licman::displayCurrentLicenseStatus();
        }
    } catch (std::exception &ex) {
        std::cerr << ERROR(ex.what()) << std::endl;
    }

    
    return -1;
}