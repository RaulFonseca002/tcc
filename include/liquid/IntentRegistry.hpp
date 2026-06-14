#pragma once

#include "liquid/Ids.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <vector>


class IntentRegistry {
private:
    std::map<BehaviorId, std::set<std::uint16_t>> active;
    std::map<BehaviorId, std::vector<std::uint16_t>> availableIds;

public:
    IntentId create(BehaviorId owner);
    void destroy(IntentId id);
    void destroy_owned_by(BehaviorId owner);
    bool exists(IntentId id);
    BehaviorId owner_of(IntentId id);
    std::size_t size(BehaviorId id);
    void addBehaviour(BehaviorId id);
};
