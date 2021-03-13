#include <catch2/catch.hpp>
#include <catch_ex.h>

#include <GS5_Action.h>
using namespace gs;

#include <chrono>

namespace {
    constexpr auto tag = "[act]";
    constexpr auto entity_id = "9f71d5f4-09dc-4667-b57f-dd066dad8f8a";
}

TEST_CASE("act-0", tag){
    auto core = TGSCore::getInstance();
    std::unique_ptr<TGSEntity> e(core->getEntityById(entity_id));

    std::unique_ptr<TGSRequest> req1(core->createRequest());
    std::unique_ptr<TGSRequest> req2(core->createRequest());

    using namespace gs::action;

    //style-1    
    TUnlock().addTo(req1.get(), entity_id);
    TLock().addTo(req1.get());

    TSetPeriod(1000).addTo(req1.get());
    TAddPeriod(2000).addTo(req1.get());

    //style-2: chained to add multiple actions.
    (*req2).add(TUnlock(), entity_id).add(TLock()).add(TSetPeriod(1000)).add(TAddPeriod(2000));    

    CHECK(req1->code() == req2->code());

    using namespace std::chrono_literals;
    TSetPeriod act1(1s);
    TSetPeriod act2(30min);
}
