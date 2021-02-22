#include <cxxopts.hpp>
#include <iostream>

#include "cmd-lock.h"
#include "cmd-status.h"
#include "helpers.h"
#include "on-exit.h"
#include "params.h"

#include <GS5.h>
using namespace gs;

int main(int argc, char *args[]) {
    cxxopts::Options opt("licman", "SoftwareShield License Management Utility");

    { //Generic
        auto add = opt.add_options();
        add("h,help", "Print usage");
        add("version", "show SDK version");
    }
    {
        // common params
        auto add = opt.add_options("common");
        add("productid", "product-id of the license data", cxxopts::value<std::string>());
        add("password", "password to decode license data", cxxopts::value<std::string>());
        add("origlic", "path to original compiled license file (*.lic)", cxxopts::value<std::string>());
    }
    //-------- main functions --------
    // display current license status
    {
        auto add = opt.add_options("status");
        add("s,status", "show current license status");
        add("verbose", "show more details");
    }
    // lock
    {
        auto add = opt.add_options("lock");
        add("l,lock", "lock an entity or app (if no entity is specified)");
        add("entity", "name(s) of entity to lock (ex: E1,E3)", cxxopts::value<std::vector<std::string>>());
        add("entity-index", "index(es) of entity to lock (starts from 0, ex: 0,1,3)", cxxopts::value<std::vector<int>>());
        add("entity-id", "id(s) of entity to lock (ex: c46c0500-e79f-4a0f-994b-ff8b56b441c2 )", cxxopts::value<std::vector<std::string>>());
    }

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
        // show license information
        if (result.count("status")) {
            if (!licman::initSDK())
                return -1;

            ON_EXIT(licman::finishSDK(););

            return licman::displayCurrentLicenseStatus();
        }
        if (result.count("lock")) {
            if (!licman::initSDK())
                return -1;

            ON_EXIT(licman::finishSDK(););

            bool entity_specified = false;
            int sum = 0;
            //parse lock-specific parameters
            if (result.count("entity")) {
                entity_specified = true;
                sum += licman::lockEntitiesByName(result["entity"].as<std::vector<std::string>>());
            }
            if (result.count("entity-id")) {
                entity_specified = true;
                sum += licman::lockEntitiesById(result["entity-id"].as<std::vector<std::string>>());
            }
            if (result.count("entity-index")) {
                entity_specified = true;
                sum += licman::lockEntitiesByIndex(result["entity-index"].as<std::vector<int>>());
            }
            if (!entity_specified)
                sum += licman::lockApp();

            std::cout << BR;
            
            if (sum > 0) {
                std::cout << KEYWORD("total locked entities") << ": " << sum << BR;
            } else {
                std::cout << ERR("no entity name matches, lock ignored.") << BR;
            }
            return 0;
        }
    } catch (std::exception &ex) {
        std::cerr << ERR(ex.what()) << std::endl;
    }

    return -1;
}