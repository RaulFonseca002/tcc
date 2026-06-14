#include "liquid/Coordinator.hpp"

#include <stdexcept>

BehaviorId Coordinator::create_behavior() {
    BehaviorId behavior = behaviors.create();
    intents.addBehaviour(behavior);

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

IntentId Coordinator::create_intent(BehaviorId owner) {
    if (!behaviors.exists(owner))
        throw std::runtime_error("behavior id not found");

    return intents.create(owner);
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
