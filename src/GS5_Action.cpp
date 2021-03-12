#include "GS5_Action.h"

namespace gs::action {

void TSetPeriod::prepare(TGSAction *act) {
    std::unique_ptr<TGSVariable> v(act->getParamByName("newPeriodInSeconds"));
    v->fromInt(_period);
}

void TAddPeriod::prepare(TGSAction *act) {
    std::unique_ptr<TGSVariable> v(act->getParamByName("addedPeriodInSeconds"));
    v->fromInt(_inc);
}

} // namespace gs::action