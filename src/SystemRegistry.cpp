#include "liquid/SystemRegistry.hpp"

void System::on_behavior_added(BehaviorId behavior) {
    (void)behavior;
}

void System::on_behavior_removed(BehaviorId behavior) {
    (void)behavior;
}

std::size_t SystemRegistry::size() const {
    return systems.size();
}

void SystemRegistry::update_behavior(BehaviorId behavior, Signature behaviorSignature) {
    for (auto& [type, record] : systems) {
        (void)type;

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
    for (auto& [type, record] : systems) {
        (void)type;

        if (record.system->behaviours.erase(behavior) > 0)
            record.system->on_behavior_removed(behavior);
    }
}
