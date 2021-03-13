#include <catch2/catch.hpp>
#include <catch_ex.h>

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include <GS5.h>
#include <GS5_Inspector.h>
using namespace gs;

#include "main.h" // for clean_license()

namespace {
const char *tag = "[period]";
//entity params
constexpr auto period{2592000};
constexpr auto entity_id = "9f71d5f4-09dc-4667-b57f-dd066dad8f8a";
} // namespace

TEST_CASE("period-0", tag) {

    TGSCore *core = TGSCore::getInstance();

    std::unique_ptr<TGSEntity> e(core->getEntityById(entity_id));
    CHECK(e->description() == std::string("30-days"));

    SECTION("license") {

        CHECK(e->hasLicense());

        std::unique_ptr<TGSLicense> lic(e->getLicense());
        //license parameters
        CHECK(lic->id() == std::string("gs.lm.expire.period.1"));

        CHECK(lic->getParamBool("exitAppOnExpire"));

        CHECK_FALSE(lic->paramHasValue("timeFirstAccess")); //not accessed before
        CHECK_FALSE(lic->paramHasValue("timeEnd"));         //no expiry date due to not accessed before
        CHECK(lic->getParamInt("periodInSeconds") == period);

        SECTION("inspector") {
            TLM_Period insp(lic.get());

            CHECK(insp.exitAppOnExpire());

            CHECK_FALSE(insp.isAccessedBefore()); //not accessed before
            CHECK(insp.elapsed().count() == 0);
            CHECK_THROWS(insp.expiryDate()); //no expiry date due to not accessed before
            CHECK(insp.period().count() == period);
        }

        SECTION("access-it") {
            CHECK(lic->status() == TLicenseStatus::STATUS_ACTIVE);
            CHECK(lic->isValid());
            CHECK(e->isAccessible());
            CHECK_FALSE(e->isLocked());

            //access it
            auto now = std::chrono::system_clock::now();
            CHECK(e->beginAccess());
            CHECK(e->isAccessing());

            printf("access for 1 second...");
            std::this_thread::sleep_for(1s);
            printf("done!\n");

            CHECK(e->endAccess());

            TLM_Period insp(lic.get());
            CHECK(insp.isAccessedBefore());
            CHECK(insp.elapsed().count() > 0);
            CHECK(std::chrono::duration_cast<std::chrono::seconds>(insp.expiryDate() - now + 1s).count() >= period);

            clean_license(); //do not pollute license status

            CHECK_FALSE(insp.isAccessedBefore());

        }
    }

    SECTION("actions") {
        SECTION("set period") {
            constexpr int new_period = 1000;

            std::unique_ptr<TGSRequest> req(core->createRequest());
            std::unique_ptr<TGSAction> act(req->addAction(ACT_SET_EXPIRE_PERIOD, e.get()));
            std::unique_ptr<TGSVariable> v(act->getParamByName("newPeriodInSeconds"));

            v->fromInt(new_period);

            std::string req_code = req->code();
            //CHECK(req_code == "AKAWB-RW5F7-4WF6J-MSCZO-EAK5Q-63F3F");
            CHECK_FALSE(req_code.empty());

            //license-code is generated by manual-activator
            const char *license_code = "AV7B-PS6B-R5ON-EIWI-L9IT-JXX7-EG3P";

            CHECK(core->applyLicenseCode(license_code));

            //after applying the license code, the entity should be accessible
            CHECK(e->isAccessible());

            std::unique_ptr<TGSLicense> lic(e->getLicense());
            CHECK(lic->getParamInt("periodInSeconds") == new_period);

            clean_license(); //do not pollute license status
        }
        SECTION("add period") {
            constexpr int topup_period = 12000;

            std::unique_ptr<TGSRequest> req(core->createRequest());
            std::unique_ptr<TGSAction> act(req->addAction(ACT_ADD_EXPIRE_PERIOD, e.get()));
            std::unique_ptr<TGSVariable> v(act->getParamByName("addedPeriodInSeconds"));

            v->fromInt(topup_period);

            std::string req_code = req->code();
            //CHECK(req_code == "ADHEM-WAL75-9LB3J-L7CCX-NPZQD-IUF3F");
            CHECK_FALSE(req_code.empty());

            //license-code is generated by manual-activator
            const char *license_code = "AORU-GZ46-U3V9-PZHM-4ZEI-MCHX-PTOP";

            CHECK(core->applyLicenseCode(license_code));

            //after applying the license code, the entity should be accessible
            CHECK(e->isAccessible());

            std::unique_ptr<TGSLicense> lic(e->getLicense());
            CHECK(lic->getParamInt("periodInSeconds") == period + topup_period);

            clean_license(); //do not pollute license status
        }
    }

    clean_license(); //do not pollute license status
}