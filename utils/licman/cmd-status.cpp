#include "cmd-status.h"

#include "on-exit.h"
#include "params.h"

#include <iostream>

#include <GS5.h>
using namespace gs;

namespace licman {
int displayCurrentLicenseStatus() {

    if (productId.empty())
        throw std::invalid_argument("product-id must be specified (by \"--productid\")!");
    if (password.empty())
        throw std::invalid_argument("password must be specified (by \"--password\")!");
    if (origLic.empty())
        throw std::invalid_argument("original license must be specified (by \"--origlic\") !");

    auto core = TGSCore::getInstance();
    if (!core->init(productId.c_str(), origLic.c_str(), password.c_str())) {
        std::cerr << "SDK initialize failure, err code: " << core->lastErrorCode() << "err: " << core->lastErrorMessage() << std::endl;
        return -1;
    }

    std::cout << "SDK initialized successfully" << std::endl;

    ON_EXIT(
        std::cout << "exiting..." << std::endl;
        TGSCore::finish(););

    return 0;
}
} // namespace licman