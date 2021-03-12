#include "cmd-status.h"

#include "helpers.h"
#include "params.h"

#include <map>

#include <GS5.h>
#include <GS5_Inspector.h>

using namespace gs;

namespace licman {
namespace {
std::string getEntityAttrString(unsigned int attr) {
    std::string result;
    if (attr & ENTITY_ATTRIBUTE_ACCESSIBLE)
        result = "ACCESSIBLE";
    else
        result = "NOT_ACCESSIBLE";

    if (attr & ENTITY_ATTRIBUTE_ACCESSING)
        result = result.empty() ? "ACCESSING" : result + " | ACCESSING";
    if (attr & ENTITY_ATTRIBUTE_LOCKED)
        result = result.empty() ? "LOCKED" : result + " | LOCKED";
    if (attr & ENTITY_ATTRIBUTE_UNLOCKED)
        result = result.empty() ? "UNLOCKED" : result + " | UNLOCKED";
    if (attr & ENTITY_ATTRIBUTE_AUTOSTART)
        result = result.empty() ? "AUTOSTART" : result + " | AUTOSTART";
    return result;
}

const char *mode_strs[3] = {"VALID_SINCE", "EXPIRE_AFTER", "VALID_RANGE"};

void dumpDateTime(const char *name, time_point_t tp) {
    std::cout << KEYWORD(name) << ": " << gs::to_simple_string(tp) << " (" << std::chrono::system_clock::to_time_t(tp) << ")" << BR;
}
void dumpDuration(const char *name, std::chrono::seconds du) {
    int days{0}, hours{0}, mins{0}, seconds{0};
    const int64_t ts = du.count();
    constexpr auto SECONDS_PER_MIN = 60;
    constexpr auto SECONDS_PER_HOUR = 60 * 60;
    constexpr auto SECONDS_PER_DAY = 24 * 60 * 60;

    days = ts / SECONDS_PER_DAY;

    int64_t left = ts - days * SECONDS_PER_DAY;
    hours = left / SECONDS_PER_HOUR;

    left = left - hours * SECONDS_PER_HOUR;
    mins = left / SECONDS_PER_MIN;

    seconds = left - mins * SECONDS_PER_MIN;

    std::string s;
    if (days) {
        s = std::to_string(days);
        s += days > 1 ? " (days)" : " (day)";
    }
    if (hours) {
        if (!s.empty())
            s += " ";
        s += std::to_string(hours);
        s += hours > 1 ? " (hours)" : "(hour)";
    }
    if (mins) {
        if (!s.empty())
            s += " ";
        s += std::to_string(mins);
        s += mins > 1 ? " (mins)" : "(min)";
    }
    if (seconds) {
        if (!s.empty())
            s += " ";
        s += std::to_string(seconds);
        s += seconds > 1 ? " (seconds)" : "(second)";
    }

    if (ts == 0)
        std::cout << KEYWORD(name) << ": 0 (second)" << BR;
    else
        std::cout << KEYWORD(name) << ": " << ts << (ts > 1 ? " (seconds)" : " (second)") << " => " << s << BR;
}

void dumpLM(TLM_Inspector &lm) {
    std::cout << KEYWORD("status") << ": " << lm.status() << PR;
}

void dumpLMExpire(TLM_Expire &lm) {
    dumpLM(lm);

    std::cout << KEYWORD("exit-app-on-expire") << ": " << lm.exitAppOnExpire() << PR;
}
void dumpLMHardDate(TGSLicense *lic) {

    TLM_HardDate lm(lic);
    dumpLMExpire(lm);

    std::cout << KEYWORD("rollback-tolerance") << ": " << lm.rollbackTolerance() << BR;

    auto mode = lm.mode();
    std::cout << KEYWORD("mode") << ": " << mode_strs[(int)mode] << BR;
    switch (mode) {
    case TLM_HardDate::Mode::VALID_SINCE: {
        dumpDateTime("time-begin", lm.timeBegin());
        break;
    }
    case TLM_HardDate::Mode::EXPIRE_AFTER: {
        dumpDateTime("time-end", lm.timeEnd());
        break;
    }
    case TLM_HardDate::Mode::VALID_RANGE: {
        dumpDateTime("time-begin", lm.timeBegin());
        dumpDateTime("time-end", lm.timeEnd());
        break;
    }
    }

    std::cout << BR << KEYWORD("rollback-tolerance") << ": " << lm.rollbackTolerance() << " (seconds)" << BR;
}
void dumpLMSession(TGSLicense *lic) {
    TLM_Session lm(lic);
    dumpLMExpire(lm);

    dumpDuration("session", lm.session());
    dumpDuration("elapsed", lm.elapsed());
}
void dumpLMDuration(TGSLicense *lic) {
    TLM_Duration lm(lic);
    dumpLMExpire(lm);

    dumpDuration("duration", lm.duration());
    dumpDuration("elapsed", lm.elapsed());
}
void dumpLMPeriod(TGSLicense *lic) {
    TLM_Period lm(lic);
    dumpLMExpire(lm);

    dumpDuration("period", lm.period());

    if (lm.isAccessedBefore()) {
        dumpDateTime("first-access-date", lm.firstAccessDate());
        dumpDateTime("expiry-date", lm.expiryDate());
        dumpDuration("elapsed", lm.elapsed());
    } else {
        std::cout << KEYWORD("never accessed before") << BR;
    }
}
void dumpLMAccessTime(TGSLicense *lic) {
    TLM_Access lm(lic);
    dumpLMExpire(lm);

    std::cout << KEYWORD("total") << ": " << lm.total() << BR;
    std::cout << KEYWORD("used") << ": " << lm.used() << BR;
}
void dumpLMAlwaysRun(TGSLicense *lic) {
    TLM_Run lm(lic);
}
void dumpLMAlwaysLock(TGSLicense *lic) {
    TLM_Lock lm(lic);
}

typedef void (*dump_func_t)(TGSLicense *);

std::map<std::string, dump_func_t> dumps{
    {"gs.lm.expire.hardDate.1", dumpLMHardDate},
    {"gs.lm.expire.accessTime.1", dumpLMAccessTime},
    {"gs.lm.expire.period.1", dumpLMPeriod},
    {"gs.lm.expire.duration.1", dumpLMDuration},
    {"gs.lm.expire.sessionTime.1", dumpLMSession},
    {"gs.lm.alwaysRun.1", dumpLMAlwaysRun},
    {"gs.lm.alwaysLock.1", dumpLMAlwaysLock}};

} // namespace
int displayCurrentLicenseStatus() {
    auto core = gs::TGSCore::getInstance();
    //Dump entity details
    int total_entities = core->getTotalEntities();
    std::cout << H1("Current License Status");

    std::cout << "Total Entities: " << total_entities << BR << std::string(18, '=') << BR;
    for (int i = 0; i < total_entities; i++) {
        std::unique_ptr<TGSEntity> entity(core->getEntityByIndex(i));

        std::cout << "[" << i << "] " << KEYWORD("name: ") << entity->name() << "," << KEYWORD(" id: ") << entity->id() << BR
                  << KEYWORD("description") << ": " << entity->description() << BR;
        auto attr = entity->attribute();
        if (verbose) {
            std::cout << KEYWORD("attribute") << ": " << attr << " ( " << getEntityAttrString(attr) << " )" << BR;
        } else {
            std::cout << KEYWORD("attribute") << ": " << getEntityAttrString(attr) << BR;
        }

        //license
        if (!entity->hasLicense())
            continue;

        std::cout << H3("License");

        std::unique_ptr<TGSLicense> lic(entity->getLicense());
        std::string id = lic->id();

        std::cout << KEYWORD("type: ") << id << BR;

        //display license status
        if (auto it = dumps.find(id); it != dumps.end()) {
            it->second(lic.get());
        } else {
            std::cerr << ERR("license type not supported!") << BR;
        }

        std::cout << PR << KEYWORD("unlock request-code: ") << lic->getUnlockRequestCode() << HR;
    }

    return 0;
}
} // namespace licman