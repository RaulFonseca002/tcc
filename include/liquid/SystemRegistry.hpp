#pragma once

#include "liquid/Ids.hpp"
#include "liquid/IntentLifetime.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class World;
class SystemRegistry;

class System {
protected:
    std::set<BehaviorId> behaviours;

    friend class SystemRegistry;

public:
    virtual ~System() = default;

    virtual void on_behavior_added(BehaviorId behavior);
    virtual void on_behavior_removed(BehaviorId behavior);
    virtual void run(World& world, FrameNumber frame, IntentTime now);
};

class SystemRegistry {
private:
    struct SystemRecord {
        Signature signature;
        std::shared_ptr<System> system;
    };

    std::unordered_map<std::type_index, SystemRecord> systems;
    std::vector<std::type_index> registrationOrder;

public:
    SystemRegistry() = default;

    template <typename SystemType, typename... Args>
    void register_system(Args&&... args);

    template <typename SystemType>
    void destroy_system();

    template <typename SystemType>
    bool exists() const;

    std::size_t size() const;

    template <typename SystemType>
    void set_signature(Signature signature);

    template <typename SystemType>
    Signature signature() const;

    template <typename SystemType>
    SystemType& get_system();

    template <typename SystemType>
    const SystemType& get_system() const;

    template <typename SystemType>
    void add_behavior(BehaviorId behavior);

    template <typename SystemType>
    void remove_behavior(BehaviorId behavior);

    template <typename SystemType>
    bool has_behavior(BehaviorId behavior) const;

    template <typename SystemType>
    std::size_t behavior_count() const;

    void update_behavior(BehaviorId behavior, Signature behaviorSignature);
    void remove_behavior(BehaviorId behavior);
    std::size_t run_systems(World& world, FrameNumber frame, IntentTime now);
};

template <typename SystemType, typename... Args>
void SystemRegistry::register_system(Args&&... args) {
    static_assert(std::is_base_of_v<System, SystemType>, "registered systems must inherit from System");

    std::type_index type = std::type_index(typeid(SystemType));

    if (systems.contains(type))
        throw std::runtime_error("system already registered");

    systems.emplace(type, SystemRecord{{}, std::make_shared<SystemType>(std::forward<Args>(args)...)});
    registrationOrder.push_back(type);
}

template <typename SystemType>
void SystemRegistry::destroy_system() {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    std::type_index type = std::type_index(typeid(SystemType));

    systems.erase(found);
    registrationOrder.erase(
        std::remove(registrationOrder.begin(), registrationOrder.end(), type),
        registrationOrder.end()
    );
}

template <typename SystemType>
bool SystemRegistry::exists() const {
    return systems.contains(std::type_index(typeid(SystemType)));
}

template <typename SystemType>
void SystemRegistry::set_signature(Signature signature) {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    found->second.signature = signature;
}

template <typename SystemType>
Signature SystemRegistry::signature() const {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    return found->second.signature;
}

template <typename SystemType>
SystemType& SystemRegistry::get_system() {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    auto system = std::dynamic_pointer_cast<SystemType>(found->second.system);

    if (!system)
        throw std::runtime_error("system type mismatch");

    return *system;
}

template <typename SystemType>
const SystemType& SystemRegistry::get_system() const {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    auto system = std::dynamic_pointer_cast<const SystemType>(found->second.system);

    if (!system)
        throw std::runtime_error("system type mismatch");

    return *system;
}

template <typename SystemType>
void SystemRegistry::add_behavior(BehaviorId behavior) {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    auto [position, inserted] = found->second.system->behaviours.emplace(behavior);
    (void)position;

    if (inserted)
        found->second.system->on_behavior_added(behavior);
}

template <typename SystemType>
void SystemRegistry::remove_behavior(BehaviorId behavior) {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    if (found->second.system->behaviours.erase(behavior) > 0)
        found->second.system->on_behavior_removed(behavior);
}

template <typename SystemType>
bool SystemRegistry::has_behavior(BehaviorId behavior) const {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    return found->second.system->behaviours.contains(behavior);
}

template <typename SystemType>
std::size_t SystemRegistry::behavior_count() const {
    auto found = systems.find(std::type_index(typeid(SystemType)));

    if (found == systems.end())
        throw std::runtime_error("system not registered");

    return found->second.system->behaviours.size();
}
