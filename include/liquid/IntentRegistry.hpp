#pragma once

#include "liquid/Ids.hpp"
#include "liquid/IntentLifetime.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace liquid {

struct Intent {
    IntentId id = 0;
    BehaviorId owner = 0;
    ComponentTarget target;
    IntentLifetime lifetime;
};

template <typename Component>
struct ComponentIntent : Intent {
    Component value;
};

using IntentTargetIndex = std::map<ComponentTypeId, std::map<ComponentSlotId, std::set<IntentId>>>;

class I_IntentStorage {
public:
    virtual ~I_IntentStorage() = default;

    virtual void destroy(IntentId id) = 0;
    virtual bool exists(IntentId id) const = 0;
    virtual const Intent& intent(IntentId id) const = 0;
};

template <typename Component>
class IntentStorage : public I_IntentStorage {
private:
    std::map<IntentId, ComponentIntent<Component>> records;

public:
    void add(ComponentIntent<Component> intent) {
        records.emplace(intent.id, std::move(intent));
    }

    void destroy(IntentId id) override {
        records.erase(id);
    }

    bool exists(IntentId id) const override {
        return records.contains(id);
    }

    const Intent& intent(IntentId id) const override {
        auto found = records.find(id);

        if (found == records.end())
            throw std::runtime_error("intent id not found");

        return found->second;
    }

    const ComponentIntent<Component>& typed_intent(IntentId id) const {
        auto found = records.find(id);

        if (found == records.end())
            throw std::runtime_error("intent id not found");

        return found->second;
    }
};

}

using liquid::ComponentIntent;
using liquid::I_IntentStorage;
using liquid::Intent;
using liquid::IntentStorage;
using liquid::IntentTargetIndex;

class IntentRegistry {
private:
    std::map<ComponentTypeId, std::shared_ptr<I_IntentStorage>> storages;
    std::map<IntentId, ComponentTypeId> intentTypes;
    std::vector<IntentId> availableIds;
    IntentId nextId = 1;

    std::map<BehaviorId, std::set<IntentId>> byOwner;
    IntentTargetIndex byTarget;

    IntentId next_intent_id();
    const I_IntentStorage& storage_for(IntentId id) const;
    void erase_from_indexes(const Intent& intent);

    template <typename Component>
    std::shared_ptr<IntentStorage<Component>> storage_for(ComponentType<Component> type);

    template <typename Component>
    std::shared_ptr<const IntentStorage<Component>> storage_for(ComponentType<Component> type) const;

public:
    template <typename Component>
    IntentId create(BehaviorId owner, ComponentType<Component> type, ComponentSlotId slot, IntentLifetime lifetime, Component value);

    void destroy(IntentId id);
    void destroy_owned_by(BehaviorId owner);

    bool exists(IntentId id) const;
    const Intent& intent(IntentId id) const;

    template <typename Component>
    const ComponentIntent<Component>& typed_intent(ComponentType<Component> type, IntentId id) const;

    BehaviorId owner_of(IntentId id) const;
    ComponentTarget target_of(IntentId id) const;
    IntentLifetime lifetime_of(IntentId id) const;

    std::vector<IntentId> live_intent_ids() const;
    std::vector<IntentId> intents_for(ComponentTypeId type, ComponentSlotId slot) const;
    const IntentTargetIndex& target_index() const;

    std::size_t size(BehaviorId id) const;
    std::size_t size() const;
    void addBehaviour(BehaviorId id);
};

template <typename Component>
std::shared_ptr<IntentStorage<Component>> IntentRegistry::storage_for(ComponentType<Component> type) {
    if (type.id == InvalidComponentTypeId)
        throw std::runtime_error("invalid component type id");

    auto found = storages.find(type.id);

    if (found == storages.end()) {
        auto storage = std::make_shared<IntentStorage<Component>>();
        storages.emplace(type.id, storage);
        return storage;
    }

    auto storage = std::dynamic_pointer_cast<IntentStorage<Component>>(found->second);

    if (!storage)
        throw std::runtime_error("intent storage type mismatch");

    return storage;
}

template <typename Component>
std::shared_ptr<const IntentStorage<Component>> IntentRegistry::storage_for(ComponentType<Component> type) const {
    auto found = storages.find(type.id);

    if (found == storages.end())
        throw std::runtime_error("intent storage type not found");

    auto storage = std::dynamic_pointer_cast<const IntentStorage<Component>>(found->second);

    if (!storage)
        throw std::runtime_error("intent storage type mismatch");

    return storage;
}

template <typename Component>
IntentId IntentRegistry::create(BehaviorId owner, ComponentType<Component> type, ComponentSlotId slot, IntentLifetime lifetime, Component value) {
    if (!byOwner.contains(owner))
        throw std::runtime_error("behavior intent pool not found");

    if (type.id == InvalidComponentTypeId)
        throw std::runtime_error("invalid component type id");

    if (slot == InvalidComponentSlotId)
        throw std::runtime_error("invalid component slot id");

    IntentId id = next_intent_id();

    ComponentIntent<Component> record;
    record.id = id;
    record.owner = owner;
    record.target = {type.id, slot};
    record.lifetime = lifetime;
    record.value = std::move(value);

    storage_for(type)->add(std::move(record));
    intentTypes.emplace(id, type.id);
    byOwner[owner].emplace(id);
    byTarget[type.id][slot].emplace(id);

    return id;
}

template <typename Component>
const ComponentIntent<Component>& IntentRegistry::typed_intent(ComponentType<Component> type, IntentId id) const {
    auto found = intentTypes.find(id);

    if (found == intentTypes.end())
        throw std::runtime_error("intent id not found");

    if (found->second != type.id)
        throw std::runtime_error("intent component type mismatch");

    return storage_for(type)->typed_intent(id);
}
