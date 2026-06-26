#include "liquid/world/World.hpp"

World::World()
    : state(),
      coordinator(state)
{
}

BehaviorId World::create_behavior() {
    return coordinator.create_behavior();
}

void World::destroy_behavior(BehaviorId id) {
    coordinator.destroy_behavior(id);
}

bool World::behavior_exists(BehaviorId id) {
    return coordinator.behavior_exists(id);
}

std::size_t World::behavior_count() {
    return coordinator.behavior_count();
}

void World::destroy_intent(IntentId id) {
    coordinator.destroy_intent(id);
}

bool World::intent_exists(IntentId id) {
    return coordinator.intent_exists(id);
}

BehaviorId World::intent_owner(IntentId id) {
    return coordinator.intent_owner(id);
}

ComponentTarget World::intent_target(IntentId id) {
    return coordinator.intent_target(id);
}

IntentLifetime World::intent_lifetime(IntentId id) {
    return coordinator.intent_lifetime(id);
}

const Intent& World::intent(IntentId id) const {
    return coordinator.intent(id);
}

std::vector<IntentId> World::live_intent_ids() const {
    return coordinator.live_intent_ids();
}

std::vector<IntentId> World::intents_for(ComponentTypeId type, ComponentSlotId slot) const {
    return coordinator.intents_for(type, slot);
}

const IntentTargetIndex& World::intent_target_index() const {
    return coordinator.intent_target_index();
}

std::map<ComponentName, IntentId> World::resolve_intents(
    ComponentTypeId type,
    const std::map<ComponentName, ComponentSlotId>& components,
    IntentTime now
) {
    return coordinator.resolve_intents(type, components, now);
}

std::size_t World::intent_count(BehaviorId owner) {
    return coordinator.intent_count(owner);
}

std::size_t World::system_count() const {
    return coordinator.system_count();
}

std::size_t World::run_systems(FrameNumber frame, IntentTime now) {
    return coordinator.run_systems(*this, frame, now);
}

ComponentTypeId World::component_type(const TypeName& typeName) const {
    return coordinator.component_type(typeName);
}

Signature World::behavior_signature(BehaviorId behavior) const {
    return coordinator.behavior_signature(behavior);
}
