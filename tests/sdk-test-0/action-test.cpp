#include <catch2/catch.hpp>
#include <catch_ex.h>

#include <GS5_Action.h>
using namespace gs;

namespace {
    constexpr auto tag = "[act]";
    constexpr auto entity_id = "9f71d5f4-09dc-4667-b57f-dd066dad8f8a";
}

TEST_CASE("act-0", tag){
    auto core = TGSCore::getInstance();
    std::unique_ptr<TGSEntity> e(core->getEntityById(entity_id));
    std::unique_ptr<TGSRequest> req(core->createRequest());

    using namespace gs::action;
    TUnlock unlock;
    unlock.addTo(req.get());

    TLock lock;
    lock.addTo(req.get());

    TSetPeriod(1000).addTo(req.get());
    TAddPeriod(2000).addTo(req.get());
}
