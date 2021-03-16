#include "GS_Action.h"

namespace gs::action {

void TSetPeriod::prepare(std::shared_ptr<TGSAction> act) const {
    act->getParamByName("newPeriodInSeconds")->set(_period);
}

void TAddPeriod::prepare(std::shared_ptr<TGSAction> act) const {
    act->getParamByName("addedPeriodInSeconds")->set(_inc);
}

} // namespace gs::action