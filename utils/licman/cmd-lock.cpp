#include "cmd-lock.h"

#include "GS.h"
#include "helpers.h"

using namespace gs;

namespace licman {
int lockApp() {
    std::cout << H1("lock app") << PR;
    auto core = gs::TGSCore::getInstance();
    core->lockAllEntities();
    return core->getTotalEntities();
}

int lockEntitiesByName(const std::vector<std::string> &names) {
    std::cout << H1("lock entity by name") << PR;

    auto core = gs::TGSCore::getInstance();
    int N = core->getTotalEntities();
    int sum = 0;
    for (int i = 0; i < N; i++) {
        auto entity(core->getEntityByIndex(i));
        if (std::find(names.begin(), names.end(), entity->name()) != names.end()) {
            std::cout << "locking " << entity->name() << "(index: "<< i << ", id: " << entity->id() << ")" << "..." << BR;
            entity->lock();
            sum++;
        }
    }

    return sum;
}

int lockEntitiesById(const std::vector<std::string> &ids) {
    std::cout << H1("lock entity by id") << PR;

    auto core = gs::TGSCore::getInstance();
    int sum = 0;
    for (const auto &id : ids) {
        try {
            auto entity(core->getEntityById(id.c_str()));
            std::cout << "locking " << entity->name() << "(id: " << entity->id() << ")" << "..." << BR;
            entity->lock();
            sum++;
        } catch (std::exception &ex) {
            std::clog << ex.what() << BR;
        }
    }

    return sum;
}

int lockEntitiesByIndex(const std::vector<int> &indices) {
    std::cout << H1("lock entity by index") << PR;

    auto core = gs::TGSCore::getInstance();
    int sum = 0;
    for (auto i : indices) {
        try {
            auto entity(core->getEntityByIndex(i));
            std::cout << "locking " << entity->name() << "(index: "<< i << ", id: " << entity->id() << ")" << "..." << BR;
            entity->lock();
            sum++;
        } catch (std::exception &ex) {
            std::clog << ex.what() << BR;
        }
    }
    return sum;
}
} // namespace licman