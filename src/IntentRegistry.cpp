#include "liquid/IntentRegistry.hpp"

#include <numeric>
#include <stdexcept>

IntentId IntentRegistry::create(BehaviorId owner) {

    if (availableIds[owner].empty())
        throw std::runtime_error("all the intents are already in use");


    std::uint16_t intent = availableIds[owner].back();
    availableIds[owner].pop_back();

    active[owner].emplace(intent);

    IntentId head = static_cast<IntentId>(owner) << INTENT_OWNER_SHIFT;
    return head | intent;
}

void IntentRegistry::addBehaviour(BehaviorId id) {
    active[id].clear();
    availableIds[id].resize(MAX_INTENTS);
    std::iota(availableIds[id].begin(), availableIds[id].end(), 0);
}

void IntentRegistry::destroy(IntentId id) {

    BehaviorId owner = id >> INTENT_OWNER_SHIFT;
    std::uint16_t intent = id & LOW_16_BITS;

    active[owner].erase(intent);
    availableIds[owner].emplace_back(intent);

}

void IntentRegistry::destroy_owned_by(BehaviorId owner) {

    active.erase(owner);
    availableIds.erase(owner);
}

bool IntentRegistry::exists(IntentId id) {

    BehaviorId owner = id >> INTENT_OWNER_SHIFT;
    std::uint16_t intent = id & LOW_16_BITS;

    auto ownerPosition = active.find(owner);

    if (ownerPosition == active.end())
        return false;

    return ownerPosition->second.contains(intent);
}

BehaviorId IntentRegistry::owner_of(IntentId id) {
    return id >> INTENT_OWNER_SHIFT;
}

std::size_t IntentRegistry::size(BehaviorId id) {
    auto ownerPosition = active.find(id);

    if (ownerPosition == active.end())
        return 0;

    return ownerPosition->second.size();
}
