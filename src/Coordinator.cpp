#include "liquid/Coordinator.hpp"

#include <stdexcept>

BehaviorId Coordinator::create_behavior() {
    BehaviorId behavior = behaviors.create();
    intents.create_behavior_pool(behavior);

    return behavior;
}

void Coordinator::destroy_behavior(BehaviorId id) {
    if (!behaviors.exists(id))
        throw std::runtime_error("behavior id not found");

    components.remove_behavior(id);
    behaviorSignatures.erase(id);
    systems.remove_behavior(id);
    intents.destroy_owned_by(id);
    behaviors.destroy(id);
}

bool Coordinator::behavior_exists(BehaviorId id) {
    return behaviors.exists(id);
}

std::size_t Coordinator::behavior_count() {
    return behaviors.size();
}

void Coordinator::destroy_intent(IntentId id) {
    intents.destroy(id);
}

bool Coordinator::intent_exists(IntentId id) {
    return intents.exists(id);
}

BehaviorId Coordinator::intent_owner(IntentId id) {
    return intents.owner_of(id);
}

ComponentTarget Coordinator::intent_target(IntentId id) {
    return intents.target_of(id);
}

IntentLifetime Coordinator::intent_lifetime(IntentId id) {
    return intents.lifetime_of(id);
}

const Intent& Coordinator::intent(IntentId id) const {
    return intents.intent(id);
}

std::vector<IntentId> Coordinator::live_intent_ids() const {
    return intents.live_intent_ids();
}

std::vector<IntentId> Coordinator::intents_for(ComponentTypeId type, ComponentSlotId slot) const {
    return intents.intents_for(type, slot);
}

const IntentTargetIndex& Coordinator::intent_target_index() const {
    return intents.target_index();
}

void Coordinator::destroy_intents_for_target(ComponentTypeId type, ComponentSlotId slot) {
    std::vector<IntentId> targeted = intents.intents_for(type, slot);

    for (IntentId id : targeted)
        intents.destroy(id);
}

void Coordinator::destroy_intents_for_owner_target(BehaviorId owner, ComponentTypeId type, ComponentSlotId slot) {
    std::vector<IntentId> targeted = intents.intents_for(type, slot);

    for (IntentId id : targeted) {
        if (intents.owner_of(id) == owner)
            intents.destroy(id);
    }
}

std::size_t Coordinator::intent_count(BehaviorId owner) {
    if (!behaviors.exists(owner))
        throw std::runtime_error("behavior id not found");

    return intents.size(owner);
}

std::size_t Coordinator::system_count() const {
    return systems.size();
}

Signature Coordinator::behavior_signature(BehaviorId behavior) const {
    auto found = behaviorSignatures.find(behavior);

    if (found == behaviorSignatures.end())
        return {};

    return found->second;
}

void Coordinator::update_system_memberships(BehaviorId behavior) {
    systems.update_behavior(behavior, behavior_signature(behavior));
}
