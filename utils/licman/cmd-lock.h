#ifndef LICMAN_LOCK_H_
#define LICMAN_LOCK_H_

#include <vector>
#include <string>

namespace licman
{
    int lockApp();
    int lockEntitiesByName(const std::vector<std::string>& names);
    int lockEntitiesById(const std::vector<std::string>& ids);
    int lockEntitiesByIndex(const std::vector<int>& indices);
    
} // namespace licman


#endif