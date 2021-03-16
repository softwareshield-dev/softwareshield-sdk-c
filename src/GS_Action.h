#ifndef GS_ACTION_H_
#define GS_ACTION_H_

/**
 *  Utility actions supported by built-in license models. 
 */

#include <chrono>

#include "GS.h"

namespace gs {
namespace action {

#define DECLARE_ACTION_ID(act_id)             \
  protected:                                  \
    virtual action_id_t id() const override { \
        return act_id;                        \
    }

//Generic Actions
class TUnlock : public TAction {
    DECLARE_ACTION_ID(ACT_UNLOCK);
};

class TLock : public TAction {
    DECLARE_ACTION_ID(ACT_LOCK);
};

//license-specific
class TSetPeriod : public TAction {
    DECLARE_ACTION_ID(ACT_SET_EXPIRE_PERIOD);

  private:
    int _period; // in seconds
  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override;

  public:
    TSetPeriod(int periodInSeconds) : _period(periodInSeconds) {}

    template <typename TDuration>
    TSetPeriod(const TDuration &period) : _period(std::chrono::duration_cast<std::chrono::seconds>(period).count()) {}
};

class TAddPeriod : public TAction {
    DECLARE_ACTION_ID(ACT_ADD_EXPIRE_PERIOD);

  private:
    int _inc; // in seconds
  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override;

  public:
    TAddPeriod(int addedPeriodInSeconds) : _inc(addedPeriodInSeconds) {}

    template <typename TDuration>
    TAddPeriod(const TDuration &addedPeriod) : _inc(std::chrono::duration_cast<std::chrono::seconds>(addedPeriod).count()) {}
};

} // namespace action
} // namespace gs

#endif