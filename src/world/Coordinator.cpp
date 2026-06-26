#include "liquid/world/Coordinator.hpp"

#include <stdexcept>

Coordinator::Coordinator(WorldState& state)
    : state(state)
{
}

BehaviorId Coordinator::create_behavior() {
    BehaviorId behavior = state.behaviors.create();
    state.intents.create_behavior_pool(behavior);

    return behavior;
}

void Coordinator::destroy_behavior(BehaviorId id) {
    if (!state.behaviors.exists(id))
        throw std::runtime_error("behavior id not found");

    state.components.remove_behavior(id);
    state.behaviorSignatures.erase(id);
    state.systems.remove_behavior(id);
    state.intents.destroy_owned_by(id);
    state.behaviors.destroy(id);
}

bool Coordinator::behavior_exists(BehaviorId id) {
    return state.behaviors.exists(id);
}

std::size_t Coordinator::behavior_count() {
    return state.behaviors.size();
}

void Coordinator::destroy_intent(IntentId id) {
    state.intents.destroy(id);
}

bool Coordinator::intent_exists(IntentId id) {
    return state.intents.exists(id);
}

BehaviorId Coordinator::intent_owner(IntentId id) {
    return state.intents.owner_of(id);
}

ComponentTarget Coordinator::intent_target(IntentId id) {
    return state.intents.target_of(id);
}

IntentLifetime Coordinator::intent_lifetime(IntentId id) {
    return state.intents.lifetime_of(id);
}

const Intent& Coordinator::intent(IntentId id) const {
    return state.intents.intent(id);
}

std::vector<IntentId> Coordinator::live_intent_ids() const {
    return state.intents.live_intent_ids();
}

std::vector<IntentId> Coordinator::intents_for(ComponentTypeId type, ComponentSlotId slot) const {
    return state.intents.intents_for(type, slot);
}

const IntentTargetIndex& Coordinator::intent_target_index() const {
    return state.intents.target_index();
}

std::map<ComponentName, IntentId> Coordinator::resolve_intents(
    ComponentTypeId type,
    const std::map<ComponentName, ComponentSlotId>& components,
    IntentTime now
) {
    return state.intents.resolve(type, components, now);
}

void Coordinator::destroy_intents_for_target(ComponentTypeId type, ComponentSlotId slot) {
    std::vector<IntentId> targeted = state.intents.intents_for(type, slot);

    for (IntentId id : targeted)
        state.intents.destroy(id);
}

void Coordinator::destroy_intents_for_owner_target(BehaviorId owner, ComponentTypeId type, ComponentSlotId slot) {
    std::vector<IntentId> targeted = state.intents.intents_for(type, slot);

    for (IntentId id : targeted) {
        if (state.intents.owner_of(id) == owner)
            state.intents.destroy(id);
    }
}

std::size_t Coordinator::intent_count(BehaviorId owner) {
    if (!state.behaviors.exists(owner))
        throw std::runtime_error("behavior id not found");

    return state.intents.size(owner);
}

std::size_t Coordinator::system_count() const {
    return state.systems.size();
}

std::size_t Coordinator::run_systems(World& world, FrameNumber frame, IntentTime now) {
    return state.systems.run_systems(world, frame, now);
}

Signature Coordinator::behavior_signature(BehaviorId behavior) const {
    auto found = state.behaviorSignatures.find(behavior);

    if (found == state.behaviorSignatures.end())
        return {};

    return found->second;
}

void Coordinator::update_system_memberships(BehaviorId behavior) {
    state.systems.update_behavior(behavior, behavior_signature(behavior));
}
