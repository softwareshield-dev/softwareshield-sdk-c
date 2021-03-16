#include <catch2/catch.hpp>
#include <catch_ex.h>

#include <GS_Action.h>
using namespace gs;

#include <chrono>

namespace {
    constexpr auto tag = "[act]";
    constexpr auto entity_id = "9f71d5f4-09dc-4667-b57f-dd066dad8f8a";
}

TEST_CASE("act-0", tag){
    auto core = TGSCore::getInstance();
    auto e(core->getEntityById(entity_id));

    auto req1(core->createRequest());
    auto req2(core->createRequest());

    using namespace gs::action;

    //style-1    
    TUnlock().addTo(*req1, entity_id);
    TLock().addTo(*req1);

    TSetPeriod(1000).addTo(*req1);
    TAddPeriod(2000).addTo(*req1);

    //style-2: chained to add multiple actions.
    (*req2).add(TUnlock(), entity_id).add(TLock()).add(TSetPeriod(1000)).add(TAddPeriod(2000));    

    CHECK(req1->code() == req2->code());

    using namespace std::chrono_literals;
    TSetPeriod act1(1s);
    TSetPeriod act2(30min);
}
