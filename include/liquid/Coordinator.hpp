#pragma once

#include "liquid/BehaviorRegistry.hpp"
#include "liquid/ComponentRegistry.hpp"
#include "liquid/Ids.hpp"
#include "liquid/IntentRegistry.hpp"
#include "liquid/SystemRegistry.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Coordinator {
private:
    // Coordinator is the public world interface. The lower registries do the
    // local work, while Coordinator keeps their lifetimes, permissions, and
    // behavior signatures in sync.
    ComponentRegistry components;
    BehaviorRegistry behaviors;
    IntentRegistry intents;
    SystemRegistry systems;
    std::map<BehaviorId, Signature> behaviorSignatures;

    void update_system_memberships(BehaviorId behavior);
    void destroy_intents_for_target(ComponentTypeId type, ComponentSlotId slot);
    void destroy_intents_for_owner_target(BehaviorId owner, ComponentTypeId type, ComponentSlotId slot);

public:
    BehaviorId create_behavior();
    void destroy_behavior(BehaviorId id);
    bool behavior_exists(BehaviorId id);
    std::size_t behavior_count();

    void destroy_intent(IntentId id);
    bool intent_exists(IntentId id);
    BehaviorId intent_owner(IntentId id);
    ComponentTarget intent_target(IntentId id);
    IntentLifetime intent_lifetime(IntentId id);
    const Intent& intent(IntentId id) const;
    std::vector<IntentId> live_intent_ids() const;
    std::vector<IntentId> intents_for(ComponentTypeId type, ComponentSlotId slot) const;
    const IntentTargetIndex& intent_target_index() const;
    std::size_t intent_count(BehaviorId owner);

    template <typename SystemType, typename... Args>
    void register_system(Args&&... args);

    template <typename SystemType>
    void destroy_system();

    template <typename SystemType>
    bool system_exists() const;

    std::size_t system_count() const;

    template <typename SystemType>
    void set_system_signature(Signature signature);

    template <typename SystemType>
    Signature system_signature() const;

    template <typename SystemType>
    SystemType& get_system();

    template <typename SystemType>
    const SystemType& get_system() const;

    template <typename SystemType>
    void add_behavior_to_system(BehaviorId behavior);

    template <typename SystemType>
    void remove_behavior_from_system(BehaviorId behavior);

    template <typename SystemType>
    bool system_has_behavior(BehaviorId behavior) const;

    template <typename SystemType>
    std::size_t system_behavior_count() const;

    template <typename Component>
    ComponentType<Component> register_component(TypeName typeName);

    ComponentTypeId component_type(const TypeName& typeName) const;

    template <typename Component>
    ComponentTypeId component_type(ComponentType<Component> type) const;

    template <typename Component>
    void add_component(ComponentType<Component> type, std::string name, Component component);

    template <typename Component>
    bool has_component_named(ComponentType<Component> type, const std::string& name) const;

    template <typename Component>
    Component* get_component_named(ComponentType<Component> type, const std::string& name);

    template <typename Component>
    const Component* get_component_named(ComponentType<Component> type, const std::string& name) const;

    template <typename Component>
    void remove_component(ComponentType<Component> type, const std::string& name);

    template <typename Component>
    void grant_component_access(ComponentType<Component> type, BehaviorId behavior, const std::string& name, ComponentAccessMode mode);

    template <typename Component>
    void revoke_component_access(ComponentType<Component> type, BehaviorId behavior, const std::string& name);

    template <typename Component>
    std::map<std::string, ComponentSlotId> get_components(ComponentType<Component> type, BehaviorId behavior) const;

    template <typename Component>
    const Component* read_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) const;

    template <typename Component>
    Component* write_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name);

    template <typename Component>
    Component* resolve_component(ComponentType<Component> type, ComponentSlotId slot);

    template <typename Component>
    const Component* resolve_component(ComponentType<Component> type, ComponentSlotId slot) const;

    template <typename Component>
    IntentId create_intent(BehaviorId owner, ComponentType<Component> type, ComponentSlotId slot, IntentLifetime lifetime, Component value);

    template <typename Component>
    const ComponentIntent<Component>& typed_intent(ComponentType<Component> type, IntentId id) const;

    template <typename Component>
    bool can_read_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) const;

    template <typename Component>
    bool can_write_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) const;

    Signature behavior_signature(BehaviorId behavior) const;
};

template <typename Component>
ComponentType<Component> Coordinator::register_component(TypeName typeName) {
    return components.register_component<Component>(std::move(typeName));
}

inline ComponentTypeId Coordinator::component_type(const TypeName& typeName) const {
    return components.component_type(typeName);
}

template <typename SystemType, typename... Args>
void Coordinator::register_system(Args&&... args) {
    systems.register_system<SystemType>(std::forward<Args>(args)...);
}

template <typename SystemType>
void Coordinator::destroy_system() {
    systems.destroy_system<SystemType>();
}

template <typename SystemType>
bool Coordinator::system_exists() const {
    return systems.exists<SystemType>();
}

template <typename SystemType>
void Coordinator::set_system_signature(Signature signature) {
    systems.set_signature<SystemType>(signature);

    for (const auto& [behavior, behaviorSignature] : behaviorSignatures) {
        (void)behaviorSignature;
        update_system_memberships(behavior);
    }
}

template <typename SystemType>
Signature Coordinator::system_signature() const {
    return systems.signature<SystemType>();
}

template <typename SystemType>
SystemType& Coordinator::get_system() {
    return systems.get_system<SystemType>();
}

template <typename SystemType>
const SystemType& Coordinator::get_system() const {
    return systems.get_system<SystemType>();
}

template <typename SystemType>
void Coordinator::add_behavior_to_system(BehaviorId behavior) {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    systems.add_behavior<SystemType>(behavior);
}

template <typename SystemType>
void Coordinator::remove_behavior_from_system(BehaviorId behavior) {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    systems.remove_behavior<SystemType>(behavior);
}

template <typename SystemType>
bool Coordinator::system_has_behavior(BehaviorId behavior) const {
    return systems.has_behavior<SystemType>(behavior);
}

template <typename SystemType>
std::size_t Coordinator::system_behavior_count() const {
    return systems.behavior_count<SystemType>();
}

template <typename Component>
ComponentTypeId Coordinator::component_type(ComponentType<Component> type) const {
    return components.component_type(type);
}

template <typename Component>
IntentId Coordinator::create_intent(BehaviorId owner, ComponentType<Component> type, ComponentSlotId slot, IntentLifetime lifetime, Component value) {
    if (!behaviors.exists(owner))
        throw std::runtime_error("behavior id not found");

    ComponentTypeId typeId = component_type(type);

    if (!components.resolve_component(type, slot))
        throw std::runtime_error("component slot not found");

    if (!components.can_write(type, owner, slot))
        throw std::runtime_error("component write access denied");

    return intents.create(owner, ComponentType<Component>{typeId}, slot, lifetime, std::move(value));
}

template <typename Component>
const ComponentIntent<Component>& Coordinator::typed_intent(ComponentType<Component> type, IntentId id) const {
    return intents.typed_intent(type, id);
}

template <typename Component>
void Coordinator::add_component(ComponentType<Component> type, std::string name, Component component) {
    components.add_component(type, std::move(name), std::move(component));
}

template <typename Component>
bool Coordinator::has_component_named(ComponentType<Component> type, const std::string& name) const {
    return components.has_component_named(type, name);
}

template <typename Component>
Component* Coordinator::get_component_named(ComponentType<Component> type, const std::string& name) {
    return components.get_component_named(type, name);
}

template <typename Component>
const Component* Coordinator::get_component_named(ComponentType<Component> type, const std::string& name) const {
    return components.get_component_named(type, name);
}

template <typename Component>
void Coordinator::remove_component(ComponentType<Component> type, const std::string& name) {
    ComponentTypeId typeId = components.component_type(type);
    ComponentSlotId slot = components.component_slot(type, name);
    std::vector<BehaviorId> affectedBehaviors = components.behaviors_with_access(type);

    destroy_intents_for_target(typeId, slot);

    // ComponentRegistry clears storage access and name/slot indexes. Coordinator
    // first removes intents targeting the slot, then clears behavior signatures
    // for behaviors that no longer have any component of this type.
    components.remove_component(type, name);

    for (BehaviorId behavior : affectedBehaviors) {
        if (components.get_components(type, behavior).empty()) {
            behaviorSignatures[behavior].reset(typeId);
            update_system_memberships(behavior);
        }
    }
}

template <typename Component>
void Coordinator::grant_component_access(ComponentType<Component> type, BehaviorId behavior, const std::string& name, ComponentAccessMode mode) {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    ComponentTypeId typeId = components.component_type(type);
    ComponentSlotId slot = components.component_slot(type, name);

    // Behavior existence is checked here, not in ComponentRegistry, so invalid
    // ownership is rejected at the world boundary.
    components.grant_access(type, behavior, name, mode);

    if (mode == ComponentAccessMode::Read)
        destroy_intents_for_owner_target(behavior, typeId, slot);

    behaviorSignatures[behavior].set(typeId);
    update_system_memberships(behavior);
}

template <typename Component>
void Coordinator::revoke_component_access(ComponentType<Component> type, BehaviorId behavior, const std::string& name) {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    ComponentTypeId typeId = components.component_type(type);
    ComponentSlotId slot = components.component_slot(type, name);

    destroy_intents_for_owner_target(behavior, typeId, slot);

    components.revoke_access(type, behavior, name);

    if (components.get_components(type, behavior).empty()) {
        behaviorSignatures[behavior].reset(typeId);
        update_system_memberships(behavior);
    }
}

template <typename Component>
std::map<std::string, ComponentSlotId> Coordinator::get_components(ComponentType<Component> type, BehaviorId behavior) const {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    return components.get_components(type, behavior);
}

template <typename Component>
const Component* Coordinator::read_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) const {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    auto available = components.get_components(type, behavior);
    auto found = available.find(name);

    if (found == available.end() || !components.can_read(type, behavior, name))
        throw std::runtime_error("component read access denied");

    // Pointers are only returned after both behavior existence and access mode
    // checks succeed.
    return components.resolve_component(type, found->second);
}

template <typename Component>
Component* Coordinator::write_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) {
    if (!behaviors.exists(behavior))
        throw std::runtime_error("behavior id not found");

    auto available = components.get_components(type, behavior);
    auto found = available.find(name);

    if (found == available.end() || !components.can_write(type, behavior, name))
        throw std::runtime_error("component write access denied");

    // Write access is stricter than resolution: resolving by slot is internal,
    // but mutable access through Coordinator must pass permission checks.
    return components.resolve_component(type, found->second);
}

template <typename Component>
Component* Coordinator::resolve_component(ComponentType<Component> type, ComponentSlotId slot) {
    return components.resolve_component(type, slot);
}

template <typename Component>
const Component* Coordinator::resolve_component(ComponentType<Component> type, ComponentSlotId slot) const {
    return components.resolve_component(type, slot);
}

template <typename Component>
bool Coordinator::can_read_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) const {
    return components.can_read(type, behavior, name);
}

template <typename Component>
bool Coordinator::can_write_component(ComponentType<Component> type, BehaviorId behavior, const std::string& name) const {
    return components.can_write(type, behavior, name);
}
