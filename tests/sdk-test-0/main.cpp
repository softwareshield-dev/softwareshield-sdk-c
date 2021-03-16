#include <catch_ex.h>

#include <sdk-test-0/license_data.h>

#include <stdexcept>

#include <GS.h>
using namespace gs;

namespace {
const char *productId = "b5e5cfab-3783-4358-a575-3520d1ef0f7b";
const char *password = "egsne_3111&IJGN&dcsvo&17332";
//the liense code generated by manual-activator to reset local license storage.
const char *lic_clean = "EZDH-E9E4-KZLZ-GSV3-CI9G-MFH3-ILDB-GW57-4YEP";

} // namespace

#define COMBINE1(X, Y) X##Y
#define COMBINE(X, Y) COMBINE1(X, Y)

//which build to use?
#define BUILD_ID 5
#define LICENSE_DATA COMBINE(sdk_test_0_lic_data_build_, BUILD_ID)

void clean_license() {
    bool ok = TGSCore::getInstance()->applyLicenseCode(lic_clean);
    if(!ok) throw std::runtime_error("cannot clean local license!");
}

void test_callback(bool start) {
    auto core = TGSCore::getInstance();
    if (start) {
        bool ok = core->init(productId, LICENSE_DATA, sizeof(LICENSE_DATA), password);
        if (!ok) {
            char buf[2048];
            snprintf(buf, sizeof(buf), "license cannot be initialized, error-code: [%d] error-message: [%s]", core->lastErrorCode(), core->lastErrorMessage());
            throw std::runtime_error(buf);
        }

        clean_license();
    } else {
        printf("exiting...\n");
        TGSCore::finish();
    }
}

REGISTER_TEST_CALLBACK(test_callback);
