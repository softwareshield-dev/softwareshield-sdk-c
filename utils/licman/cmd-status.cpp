#include "cmd-status.h"

#include "on-exit.h"
#include "params.h"
#include "helpers.h"

#include <iostream>

#include <GS5.h>
using namespace gs;

namespace licman {
namespace {
std::string getEntityAttrString(unsigned int attr) {
    std::string result;
    if (attr & ENTITY_ATTRIBUTE_ACCESSIBLE)
        result = "ACCESSIBLE";
    else
        result = "NOT_ACCESSIBLE";

    if (attr & ENTITY_ATTRIBUTE_ACCESSING)
        result = result.empty() ? "ACCESSING" : result + " | ACCESSING";
    if (attr & ENTITY_ATTRIBUTE_LOCKED)
        result = result.empty() ? "LOCKED" : result + " | LOCKED";
    if (attr & ENTITY_ATTRIBUTE_UNLOCKED)
        result = result.empty() ? "UNLOCKED" : result + " | UNLOCKED";
    if (attr & ENTITY_ATTRIBUTE_AUTOSTART)
        result = result.empty() ? "AUTOSTART" : result + " | AUTOSTART";
    return result;
}

} // namespace
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

    std::clog << "SDK initialized successfully" << std::endl
              << std::endl;

    //Dump entity details
    int total_entities = core->getTotalEntities();
    std::cout << "Total Entities: " << total_entities << std::endl
              << std::string(18, '=') << std::endl;
    for (int i = 0; i < total_entities; i++) {
        std::unique_ptr<TGSEntity> entity(core->getEntityByIndex(i));

        std::cout << "[" << i << "] " << KEYWORD("name: ") << entity->name() << "," << KEYWORD(" id: ") << entity->id();
        auto attr = entity->attribute();
        std::cout << KEYWORD(" attribute: ") << attr << " ( " << getEntityAttrString(attr) << " )" << std::endl;
        std::cout << std::endl;

        //license
        if(!entity->hasLicense()) continue;

        std::unique_ptr<TGSLicense> lic(entity->getLicense());
    }

    ON_EXIT(
        std::clog << std::endl
                  << "exiting..." << std::endl;
        TGSCore::finish(););

    return 0;
}
} // namespace licman