#ifndef GS5_INSPECTOR_H_
#define GS5_INSPECTOR_H_

/**
  *  Built-In License Model Inspectors
  *
  *  An inspector encapsulates LM-specific functions into a helper class, otherwise you have to access a LM via TGSLicense ( a generic way)
  *
  */
#include <chrono>
#include <stdexcept>
#include <string>

#include "GS5.h"

namespace gs {

using time_point_t = std::chrono::time_point<std::chrono::system_clock>;

//root base for all license model inspectors
class TLM_Inspector {
  protected:
    TGSLicense *_lic; //weak ptr to the license being inspected.
  public:
    TLM_Inspector(TGSLicense *lic) : _lic(lic) {}
    virtual ~TLM_Inspector() = default;

    virtual std::string status() const {
        auto stat = _lic->status();

        switch (stat) {
        case STATUS_INVALID:
            throw std::invalid_argument("Invalid license status");
        case STATUS_UNLOCKED:
            return "unlocked";
        case STATUS_LOCKED:
            return "locked";
        case STATUS_ACTIVE:
            return "active";
        default:
            throw std::invalid_argument("Unknown license status");
        }
    }
};

//common base for inspector of all trial license models
class TLM_Expire : public TLM_Inspector {
    typedef TLM_Inspector inherited;

  public:
    TLM_Expire(TGSLicense *lic) : inherited(lic) {}
    virtual ~TLM_Expire() = default;

    //If True the SDK will automatically exit application when the license is expired.
    //otherwise only expire event (EVENT_ENTITY_ACCESS_INVALID) is fired and
    //the developer is responsible for the appropriate response in event handler
    bool exitAppOnExpire() const {
        return _lic->getParamBool("exitAppOnExpire");
    }
};

/// LM_Expire_Duration Inspector
class TLM_Duration : public TLM_Expire {
    typedef TLM_Expire inherited;

  public:
    TLM_Duration(TGSLicense *lic) : inherited(lic) {}
    virtual ~TLM_Duration() = default;
};

/// LM_Expire_HardDate Inspector
class TLM_HardDate : public TLM_Expire {
    typedef TLM_Expire inherited;

  public:
    TLM_HardDate(TGSLicense *lic) : inherited(lic) {}
    virtual ~TLM_HardDate() = default;

    //Different work mode
    enum class Mode : int {
        VALID_SINCE = 0,  // becomes valid since a starting point [tBegin, +INF)
        EXPIRE_AFTER = 1, // becomes expired after a ending point (-INF, tEnd)
        VALID_RANGE = 2   // valid within a range [tBegin, tEnd)
    };

    //Get work mode
    Mode mode() const {
        if (timeBeginEnabled()) {
            return timeEndEnabled() ? Mode::VALID_RANGE : Mode::VALID_SINCE;
        } else {
            return Mode::EXPIRE_AFTER;
        }
    }

    //Is starting point defined / enabled ?
    bool timeBeginEnabled() const {
        return _lic->getParamBool("timeBeginEnabled");
    }

    time_point_t timeBegin() const {
        if (!timeBeginEnabled())
            throw std::invalid_argument("starting point not defined or enabled!");

        return std::chrono::system_clock::from_time_t(_lic->getParamUTCTime("timeBegin"));
    }
    //Is ending point defined / enabled ?
    bool timeEndEnabled() const {
        return _lic->getParamBool("timeEndEnabled");
    }

    time_point_t timeEnd() const {
        if (!timeEndEnabled())
            throw std::invalid_argument("ending point not defined or enabled!");

        return std::chrono::system_clock::from_time_t(_lic->getParamUTCTime("timeEnd"));
    }

    //maximum acceptable system clock rollback (in seconds)
    int rollbackTolerance() const {
        return _lic->getParamInt("rollbackTolerance");
    }
};

/// LM_Expire_Period Inspector
class TLM_Period : public TLM_Expire {
    typedef TLM_Expire inherited;

  public:
    TLM_Period(TGSLicense *lic) : inherited(lic) {}
    virtual ~TLM_Period() = default;

    //The license has already been used before.
    //(first-access-time is a valid date time)
    bool isUsed();
};

/// LM_Expire_Session Inspector
class TLM_Session : public TLM_Expire {
    typedef TLM_Expire inherited;

  public:
    TLM_Session(TGSLicense *lic) : inherited(lic) {}
    virtual ~TLM_Session() = default;
};

/// LM_Expire_AccessTime Inspector
class TLM_Access : public TLM_Expire {
    typedef TLM_Expire inherited;

  public:
    TLM_Access(TGSLicense *lic) : inherited(lic) {}
    ~TLM_Access() = default;
};

/// LM_Always_Run Inspector
class TLM_Run : public TLM_Inspector {
    typedef TLM_Inspector inherited;

  public:
    TLM_Run(TGSLicense *lic) : inherited(lic) {}
    virtual ~TLM_Run() = default;
};

/// LM_Always_Lock Inspector
class TLM_Lock : public TLM_Inspector {
    typedef TLM_Inspector inherited;

  public:
    TLM_Lock(TGSLicense *lic) : inherited(lic) {}
    ~TLM_Lock() = default;
};

//Date time helpers
std::string format_time_t(std::time_t t, const char *format);

std::string to_simple_string(time_point_t tp);
std::string to_iso_string(time_point_t tp);

} // namespace gs

#endif