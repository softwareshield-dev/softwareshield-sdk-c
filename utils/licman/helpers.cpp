#include "helpers.h"
#include "params.h"

#include "GS5.h"

namespace licman {
namespace {
bool sdk_inited = false;
}
bool initSDK() {
    if (sdk_inited)
        return true;

    if (productId.empty())
        throw std::invalid_argument("product-id must be specified (by \"--productid\")!");
    if (password.empty())
        throw std::invalid_argument("password must be specified (by \"--password\")!");
    if (origLic.empty())
        throw std::invalid_argument("original license must be specified (by \"--origlic\") !");

    auto core = gs::TGSCore::getInstance();
    if (!core->init(productId.c_str(), origLic.string().c_str(), password.c_str())) {
        std::cerr << "SDK initialize failure, err code: " << core->lastErrorCode() << "err: " << core->lastErrorMessage() << PR;
        return false;
    }

    std::clog << "SDK initialized successfully" << PR;

    sdk_inited = true;
    return true;
}

void finishSDK() {
    if (sdk_inited) {
        std::clog << BR << "exiting..." << BR;
        gs::TGSCore::finish();
    }
}
} // namespace licman