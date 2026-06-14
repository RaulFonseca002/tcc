#pragma once

#include "liquid/Ids.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace liquid {

// Access is local to one typed storage. The slot identifies the component
// instance inside this storage; ComponentRegistry owns the name/type lookup
// needed to find that slot.
struct ComponentSlotAccess {

    enum Mode {
        r,
        rw,
        w
    };

    Mode mode;
    Slot slot;
};

class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;

    virtual const std::unordered_map<BehaviorId, std::vector<ComponentSlotAccess>>& allAccesses() const = 0;
    virtual const std::vector<ComponentSlotAccess>& accessesOf(BehaviorId behavior) const = 0;
    // Cleanup hooks used by ComponentRegistry/Coordinator before slots or
    // behavior IDs are recycled.
    virtual void removeAccess(BehaviorId behavior, Slot slot) = 0;
    virtual void removeAccessesOf(BehaviorId behavior) = 0;
    virtual void removeAccessesTo(Slot slot) = 0;
};

template<typename Component>
class ComponentStorage : public IComponentStorage{
private:
    // One ComponentStorage<T> stores only T instances. Names, type IDs, and
    // cross-manager permission checks belong to ComponentRegistry/Coordinator.
    std::vector<Component> components;
    std::vector<Slot> availableSlots;
    std::unordered_map<BehaviorId, std::vector<ComponentSlotAccess>> accesses;
    std::size_t count = 0;

public:

    Slot add(Component component) {

        Slot id = 0;

        if (!availableSlots.empty()) {
            id = availableSlots.back();
            availableSlots.pop_back();

            components[id] = std::move(component);
        }else{

            if (components.size() >= MaxComponentSlots)
                throw std::runtime_error("all component slots are already filled");

            id = components.size();
            components.push_back(std::move(component));
        }

        count++;
        return id;
    }

    void remove(Slot id) {
        if (id >= components.size())
            throw std::out_of_range("invalid component slot id");

        if (std::find(availableSlots.begin(), availableSlots.end(), id) != availableSlots.end())
            throw std::runtime_error("component slot already removed");

        availableSlots.push_back(id);
        count--;
    }

    void addAccess(BehaviorId behavior, ComponentSlotAccess::Mode mode, Slot slot) {
        accesses[behavior].push_back({mode, slot});
    }

    void removeAccess(BehaviorId behavior, Slot slot) override {
        auto it = accesses.find(behavior);

        if (it == accesses.end())
            return;

        auto& behaviorAccesses = it->second;
        behaviorAccesses.erase(
            std::remove_if(behaviorAccesses.begin(), behaviorAccesses.end(), [slot](const ComponentSlotAccess& access) {
                return access.slot == slot;
            }),
            behaviorAccesses.end());

        if (behaviorAccesses.empty())
            accesses.erase(it);
    }

    void removeAccessesOf(BehaviorId behavior) override {
        accesses.erase(behavior);
    }

    void removeAccessesTo(Slot slot) override {
        for (auto it = accesses.begin(); it != accesses.end();) {
            auto& behaviorAccesses = it->second;
            behaviorAccesses.erase(
                std::remove_if(behaviorAccesses.begin(), behaviorAccesses.end(), [slot](const ComponentSlotAccess& access) {
                    return access.slot == slot;
                }),
                behaviorAccesses.end());

            if (behaviorAccesses.empty()) {
                it = accesses.erase(it);
            } else {
                ++it;
            }
        }
    }

    const std::unordered_map<BehaviorId, std::vector<ComponentSlotAccess>>& allAccesses() const override {
        return accesses;
    }

    const std::vector<ComponentSlotAccess>& accessesOf(BehaviorId behavior) const override {
        auto it = accesses.find(behavior);

        if (it == accesses.end())
            throw std::runtime_error("behavior has no access to this component storage");

        return it->second;
    }

    Component& operator[](Slot id) {
        if (id >= components.size())
            throw std::out_of_range("invalid component slot id");

        if (std::find(availableSlots.begin(), availableSlots.end(), id) != availableSlots.end())
            throw std::runtime_error("component slot removed");

        return components[id];
    }

    const Component& operator[](Slot id) const {
        if (id >= components.size())
            throw std::out_of_range("invalid component slot id");
        if (std::find(availableSlots.begin(), availableSlots.end(), id) != availableSlots.end())
            throw std::runtime_error("component slot removed");

        return components[id];
    }

    bool has(Slot id) const {
        if (id >= components.size())
            return false;

        return std::find(availableSlots.begin(), availableSlots.end(), id) == availableSlots.end();
    }

    Component* get(Slot id) {
        if (!has(id))
            return nullptr;

        return &components[id];
    }

    const Component* get(Slot id) const {
        if (!has(id))
            return nullptr;

        return &components[id];
    }

    std::size_t size() const {
        return count;
    }

    std::size_t slot_count() const {
        return components.size();
    }

};

}
