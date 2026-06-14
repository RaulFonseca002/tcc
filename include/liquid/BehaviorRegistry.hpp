#pragma once

#include "liquid/Ids.hpp"

#include <cstddef>
#include <set>
#include <vector>

class IntentRegistry;

class BehaviorRegistry {
private:
    std::set<BehaviorId> active;
    std::vector<BehaviorId> availableIds;

public:
    BehaviorRegistry();
    BehaviorId create();
    void destroy(BehaviorId id);
    bool exists(BehaviorId id) const;
    std::size_t size() const;
};
