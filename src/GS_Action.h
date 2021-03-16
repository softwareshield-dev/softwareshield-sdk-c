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
class TDummy : public TAction {
    DECLARE_ACTION_ID(ACT_DUMMY);
};

class TUnlock : public TAction {
    DECLARE_ACTION_ID(ACT_UNLOCK);
};

class TLock : public TAction {
    DECLARE_ACTION_ID(ACT_LOCK);
};

class TClean : public TAction {
    DECLARE_ACTION_ID(ACT_CLEAN);
};

class TFix : public TAction {
    DECLARE_ACTION_ID(ACT_FIX);
};

//reset trial period/duration to initial value as if app is first launched.
class TResetExpiration : public TAction {
    DECLARE_ACTION_ID(ACT_RESET_ALLEXPIRATION);
};

class TDisableFingerPrint : public TAction {
    DECLARE_ACTION_ID(ACT_DISABLE_COPYPROTECTION);
};

class TDisableClockRollbackChecking : public TAction {
    DECLARE_ACTION_ID(ACT_DISABLE_CLOCK_ROLLBACK_CHECKING);
};

class TSetVar : public TAction {
    DECLARE_ACTION_ID(ACT_SET_PARAM);

  private:
    std::string _name;
    std::string _value;

  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("name")->fromString(_value.c_str());
    }

  public:
    TSetVar(const std::string &name, const std::string &value) : _name(name), _value(value) {}
};

class TOneShot : public TAction {
    DECLARE_ACTION_ID(ACT_ONE_SHOT);
};

class TShelfTime : public TAction {
    DECLARE_ACTION_ID(ACT_SHELFTIME);

  private:
    time_point_t _expireDate;

  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("endDate")->fromUTCTime(std::chrono::system_clock::to_time_t(_expireDate));
    }

  public:
    TShelfTime(time_point_t expireDate) : _expireDate(_expireDate) {}
};

//license-specific
class TSetPeriod : public TAction {
    DECLARE_ACTION_ID(ACT_SET_EXPIRE_PERIOD);

  private:
    int _period; // in seconds
  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("newPeriodInSeconds")->set(_period);
    }

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
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("addedPeriodInSeconds")->set(_inc);
    }

  public:
    TAddPeriod(int addedPeriodInSeconds) : _inc(addedPeriodInSeconds) {}

    template <typename TDuration>
    TAddPeriod(const TDuration &addedPeriod) : _inc(std::chrono::duration_cast<std::chrono::seconds>(addedPeriod).count()) {}
};

class TAddAccessTime : public TAction {
    DECLARE_ACTION_ID(ACT_ADD_ACCESSTIME);

  private:
    int _count;

  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("addedAccessTime")->set(_count);
    }

  public:
    TAddAccessTime(int addedTimes) : _count(addedTimes) {}
};

class TSetAccessTime : public TAction {
    DECLARE_ACTION_ID(ACT_SET_ACCESSTIME);

  private:
    int _value;

  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("newAccessTime")->set(_value);
    }

  public:
    TSetAccessTime(int times) : _value(times) {}
};

class TSetStartDate : public TAction {
    DECLARE_ACTION_ID(ACT_SET_STARTDATE);

  private:
    time_point_t _value;

  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("startDate")->fromUTCTime(std::chrono::system_clock::to_time_t(_value));
    }

  public:
    TSetStartDate(time_point_t startDate) : _value(startDate) {}
};

class TSetEndDate : public TAction {
    DECLARE_ACTION_ID(ACT_SET_ENDDATE);

  private:
    time_point_t _value;

  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("endDate")->fromUTCTime(std::chrono::system_clock::to_time_t(_value));
    }

  public:
    TSetEndDate(time_point_t endDate) : _value(endDate) {}
};

class TSetDuration : public TAction {
    DECLARE_ACTION_ID(ACT_SET_EXPIRE_DURATION);

  private:
    int _value; // in seconds
  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("duration")->set(_value);
    }

  public:
    TSetDuration(int durationInSeconds) : _value(durationInSeconds) {}

    template <typename TDuration>
    TSetDuration(const TDuration &duration) : _value(std::chrono::duration_cast<std::chrono::seconds>(duration).count()) {}
};

class TAddDuration : public TAction {
    DECLARE_ACTION_ID(ACT_ADD_EXPIRE_DURATION);

  private:
    int _value; // in seconds
  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("addedDuration")->set(_value);
    }

  public:
    TAddDuration(int durationInSeconds) : _value(durationInSeconds) {}

    template <typename TDuration>
    TAddDuration(const TDuration &duration) : _value(std::chrono::duration_cast<std::chrono::seconds>(duration).count()) {}
};

class TSetSessionTime : public TAction {
    DECLARE_ACTION_ID(ACT_SET_SESSIONTIME);

  private:
    int _value; //new session time
  protected:
    virtual void prepare(std::shared_ptr<TGSAction> act) const override {
        act->getParamByName("newSessionTime")->set(_value);
    }

  public:
    TSetSessionTime(int sessionTime) : _value(sessionTime) {}
};

} // namespace action
} // namespace gs

#endif