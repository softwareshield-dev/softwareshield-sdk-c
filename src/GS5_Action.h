#ifndef GS5_ACTION_H_
#define GS5_ACTION_H_

/**
 *  Utility actions supported by built-in license models. 
 */

#include "GS5.h"

namespace gs {
namespace action {

template <action_id_t action_id>
class TAction {
  protected:
    //setup action with proper parameters
    virtual void prepare(TGSAction *act) {}

  public:
    virtual ~TAction() = default;
    //add this action to a request code and optionally specify the action's target entity.
    //if the target is not specified, the action will be applied to all entities.
    void addTo(TGSRequest *req, const char *target_entityid = nullptr) {
        std::unique_ptr<TGSAction> act(req->addAction(action_id, target_entityid));
        this->prepare(act.get());
    }
};

//Generic Actions
using TUnlock = TAction<ACT_UNLOCK>;
using TLock = TAction<ACT_LOCK>;

//license-specific
class TSetPeriod : public TAction<ACT_SET_EXPIRE_PERIOD> {
  private:
    int _period; // in seconds
  protected:
    virtual void prepare(TGSAction *act) override;

  public:
    TSetPeriod(int periodInSeconds) : _period(periodInSeconds) {}
};

class TAddPeriod : public TAction<ACT_ADD_EXPIRE_PERIOD> {
  private:
    int _inc; // in seconds
  protected:
    virtual void prepare(TGSAction *act) override;

  public:
    TAddPeriod(int addedPeriodInSeconds) : _inc(addedPeriodInSeconds) {}
};

} // namespace action
} // namespace gs

#endif