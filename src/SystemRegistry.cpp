#include "liquid/SystemRegistry.hpp"

void System::on_behavior_added(BehaviorId behavior) {
    (void)behavior;
}

void System::on_behavior_removed(BehaviorId behavior) {
    (void)behavior;
}

void System::run(World& world, FrameNumber frame, IntentTime now) {
    (void)world;
    (void)frame;
    (void)now;
}

std::size_t SystemRegistry::size() const {
    return systems.size();
}

void SystemRegistry::update_behavior(BehaviorId behavior, Signature behaviorSignature) {
    for (const std::type_index& type : registrationOrder) {
        auto& record = systems.at(type);

        if ((behaviorSignature & record.signature) == record.signature) {
            auto [position, inserted] = record.system->behaviours.emplace(behavior);
            (void)position;

            if (inserted)
                record.system->on_behavior_added(behavior);
        } else {
            if (record.system->behaviours.erase(behavior) > 0)
                record.system->on_behavior_removed(behavior);
        }
    }
}

void SystemRegistry::remove_behavior(BehaviorId behavior) {
    for (const std::type_index& type : registrationOrder) {
        auto& record = systems.at(type);

        if (record.system->behaviours.erase(behavior) > 0)
            record.system->on_behavior_removed(behavior);
    }
}

std::size_t SystemRegistry::run_systems(World& world, FrameNumber frame, IntentTime now) {
    std::vector<std::type_index> order = registrationOrder;
    std::size_t systemsRun = 0;

    for (const std::type_index& type : order) {
        auto found = systems.find(type);

        if (found == systems.end())
            continue;

        found->second.system->run(world, frame, now);
        ++systemsRun;
    }

    return systemsRun;
}
