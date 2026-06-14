#include "liquid/Coordinator.hpp"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

struct Light {
    int brightness = 0;
};

struct Temperature {
    int celsius = 0;
};

struct TrackingSystem : System {
    std::vector<std::string> events;

    void on_behavior_added(BehaviorId behavior) override {
        events.push_back("+" + std::to_string(behavior));
    }

    void on_behavior_removed(BehaviorId behavior) override {
        events.push_back("-" + std::to_string(behavior));
    }
};

struct TemperatureSystem : System {
    std::vector<BehaviorId> added;
    std::vector<BehaviorId> removed;

    void on_behavior_added(BehaviorId behavior) override {
        added.push_back(behavior);
    }

    void on_behavior_removed(BehaviorId behavior) override {
        removed.push_back(behavior);
    }
};

struct ConfiguredSystem : System {
    int threshold = 0;
    std::string label;

    ConfiguredSystem(int threshold, std::string label)
        : threshold(threshold),
          label(std::move(label))
    {
    }
};

template <typename Function>
void expect_throw(Function function)
{
    bool thrown = false;

    try {
        function();
    } catch (...) {
        thrown = true;
    }

    assert(thrown);
}

int main()
{
    {
        Coordinator coordinator;

        BehaviorId behavior = coordinator.create_behavior();
        IntentId intent = coordinator.create_intent(behavior);

        assert(behavior == 0);
        assert(coordinator.behavior_exists(behavior));
        assert(coordinator.behavior_count() == 1);
        assert(coordinator.intent_exists(intent));
        assert(coordinator.intent_owner(intent) == behavior);
        assert(coordinator.intent_count(behavior) == 1);

        expect_throw([&] {
            coordinator.create_intent(42);
        });

        expect_throw([&] {
            coordinator.intent_count(42);
        });

        expect_throw([&] {
            coordinator.destroy_behavior(42);
        });

        coordinator.destroy_behavior(behavior);

        assert(!coordinator.behavior_exists(behavior));
        assert(!coordinator.intent_exists(intent));
        assert(coordinator.behavior_count() == 0);

        expect_throw([&] {
            coordinator.intent_count(behavior);
        });

        BehaviorId recycled = coordinator.create_behavior();
        IntentId recreatedIntent = coordinator.create_intent(recycled);

        assert(recycled == behavior);
        assert(recreatedIntent == intent);
        assert(coordinator.intent_exists(recreatedIntent));
        assert(coordinator.intent_count(recycled) == 1);
    }

    {
        Coordinator coordinator;

        BehaviorId behavior = coordinator.create_behavior();
        IntentId intent = coordinator.create_intent(behavior);

        coordinator.destroy_intent(intent);

        assert(coordinator.behavior_exists(behavior));
        assert(!coordinator.intent_exists(intent));
        assert(coordinator.behavior_count() == 1);
        assert(coordinator.intent_count(behavior) == 0);
    }

    {
        Coordinator coordinator;

        ComponentType<Light> lightType = coordinator.register_component<Light>("Light");
        ComponentType<Temperature> temperatureType = coordinator.register_component<Temperature>("Temperature");

        coordinator.add_component(lightType, "officeLight", Light{40});
        coordinator.add_component(lightType, "deskLight", Light{80});
        coordinator.add_component(temperatureType, "officeTemperature", Temperature{22});

        BehaviorId trackingBehavior = coordinator.create_behavior();
        BehaviorId focusBehavior = coordinator.create_behavior();

        coordinator.register_system<TrackingSystem>();
        Signature lightSignature;
        lightSignature.set(lightType.id);
        coordinator.set_system_signature<TrackingSystem>(lightSignature);

        auto& trackingSystem = coordinator.get_system<TrackingSystem>();

        expect_throw([&] {
            coordinator.get_components(lightType, 99);
        });

        expect_throw([&] {
            coordinator.grant_component_access(lightType, 99, "officeLight", ComponentAccessMode::Read);
        });

        coordinator.grant_component_access(lightType, trackingBehavior, "officeLight", ComponentAccessMode::ReadWrite);
        coordinator.grant_component_access(lightType, trackingBehavior, "deskLight", ComponentAccessMode::Read);
        coordinator.grant_component_access(lightType, focusBehavior, "officeLight", ComponentAccessMode::Read);
        coordinator.grant_component_access(temperatureType, trackingBehavior, "officeTemperature", ComponentAccessMode::Read);

        auto trackingLights = coordinator.get_components(lightType, trackingBehavior);
        auto focusLights = coordinator.get_components(lightType, focusBehavior);

        assert(trackingLights.size() == 2);
        assert(focusLights.size() == 1);
        assert(trackingLights.at("officeLight") == 0);
        assert(trackingLights.at("deskLight") == 1);
        assert(focusLights.at("officeLight") == 0);
        assert(coordinator.resolve_component(lightType, trackingLights.at("officeLight"))->brightness == 40);
        assert(coordinator.read_component(lightType, focusBehavior, "officeLight")->brightness == 40);
        assert(coordinator.write_component(lightType, trackingBehavior, "officeLight")->brightness == 40);

        expect_throw([&] {
            coordinator.write_component(lightType, focusBehavior, "officeLight");
        });

        expect_throw([&] {
            coordinator.read_component(lightType, 99, "officeLight");
        });

        expect_throw([&] {
            coordinator.revoke_component_access(lightType, 99, "officeLight");
        });

        assert(coordinator.can_read_component(lightType, focusBehavior, "officeLight"));
        assert(!coordinator.can_write_component(lightType, focusBehavior, "officeLight"));
        assert(coordinator.can_write_component(lightType, trackingBehavior, "officeLight"));
        assert(coordinator.behavior_signature(trackingBehavior).test(lightType.id));
        assert(coordinator.behavior_signature(trackingBehavior).test(temperatureType.id));
        assert(coordinator.behavior_signature(focusBehavior).test(lightType.id));
        assert(coordinator.system_behavior_count<TrackingSystem>() == 2);
        assert(coordinator.system_has_behavior<TrackingSystem>(trackingBehavior));
        assert(coordinator.system_has_behavior<TrackingSystem>(focusBehavior));
        assert((trackingSystem.events == std::vector<std::string>{"+0", "+1"}));

        coordinator.remove_component(lightType, "officeLight");

        assert(!coordinator.has_component_named(lightType, "officeLight"));
        assert(coordinator.get_components(lightType, focusBehavior).empty());
        assert(!coordinator.behavior_signature(focusBehavior).test(lightType.id));
        assert(!coordinator.system_has_behavior<TrackingSystem>(focusBehavior));
        assert(coordinator.behavior_signature(trackingBehavior).test(lightType.id));
        assert(coordinator.system_has_behavior<TrackingSystem>(trackingBehavior));
        assert(coordinator.get_components(lightType, trackingBehavior).size() == 1);
        assert(coordinator.get_components(lightType, trackingBehavior).at("deskLight") == 1);

        coordinator.add_component(lightType, "taskLight", Light{55});
        coordinator.grant_component_access(lightType, focusBehavior, "taskLight", ComponentAccessMode::ReadWrite);

        auto reusedLights = coordinator.get_components(lightType, focusBehavior);
        assert(reusedLights.size() == 1);
        assert(reusedLights.at("taskLight") == 0);
        assert(coordinator.system_has_behavior<TrackingSystem>(focusBehavior));
        assert(coordinator.resolve_component(lightType, reusedLights.at("taskLight"))->brightness == 55);

        coordinator.revoke_component_access(lightType, focusBehavior, "taskLight");

        assert(coordinator.get_components(lightType, focusBehavior).empty());
        assert(!coordinator.behavior_signature(focusBehavior).test(lightType.id));
        assert(!coordinator.system_has_behavior<TrackingSystem>(focusBehavior));

        coordinator.remove_component(lightType, "deskLight");

        assert(coordinator.get_components(lightType, trackingBehavior).empty());
        assert(!coordinator.behavior_signature(trackingBehavior).test(lightType.id));
        assert(!coordinator.system_has_behavior<TrackingSystem>(trackingBehavior));
        assert(coordinator.behavior_signature(trackingBehavior).test(temperatureType.id));
    }

    {
        Coordinator coordinator;

        ComponentType<Temperature> temperatureType = coordinator.register_component<Temperature>("Temperature");
        coordinator.add_component(temperatureType, "officeTemperature", Temperature{22});

        coordinator.register_system<TemperatureSystem>();
        Signature temperatureSignature;
        temperatureSignature.set(temperatureType.id);
        coordinator.set_system_signature<TemperatureSystem>(temperatureSignature);

        BehaviorId first = coordinator.create_behavior();
        BehaviorId second = coordinator.create_behavior();
        IntentId intent = coordinator.create_intent(first);

        coordinator.grant_component_access(temperatureType, first, "officeTemperature", ComponentAccessMode::ReadWrite);
        coordinator.grant_component_access(temperatureType, second, "officeTemperature", ComponentAccessMode::Read);

        assert(coordinator.system_has_behavior<TemperatureSystem>(first));
        assert(coordinator.system_has_behavior<TemperatureSystem>(second));

        coordinator.destroy_behavior(first);

        assert(!coordinator.behavior_exists(first));
        assert(!coordinator.intent_exists(intent));
        assert(!coordinator.system_has_behavior<TemperatureSystem>(first));
        assert(coordinator.system_has_behavior<TemperatureSystem>(second));

        expect_throw([&] {
            coordinator.get_components(temperatureType, first);
        });

        BehaviorId recycled = coordinator.create_behavior();

        assert(recycled == first);
        assert(coordinator.get_components(temperatureType, recycled).empty());
        assert(!coordinator.behavior_signature(recycled).test(temperatureType.id));
        assert(!coordinator.system_has_behavior<TemperatureSystem>(recycled));
    }

    {
        Coordinator coordinator;

        coordinator.register_system<ConfiguredSystem>(4, "manual");
        auto& configured = coordinator.get_system<ConfiguredSystem>();
        assert(configured.threshold == 4);
        assert(configured.label == "manual");

        BehaviorId behavior = coordinator.create_behavior();

        coordinator.add_behavior_to_system<ConfiguredSystem>(behavior);
        coordinator.add_behavior_to_system<ConfiguredSystem>(behavior);

        assert(coordinator.system_has_behavior<ConfiguredSystem>(behavior));
        assert(coordinator.system_behavior_count<ConfiguredSystem>() == 1);

        coordinator.remove_behavior_from_system<ConfiguredSystem>(behavior);
        coordinator.remove_behavior_from_system<ConfiguredSystem>(behavior);

        assert(!coordinator.system_has_behavior<ConfiguredSystem>(behavior));

        expect_throw([&] {
            coordinator.add_behavior_to_system<ConfiguredSystem>(99);
        });

        expect_throw([&] {
            coordinator.remove_behavior_from_system<ConfiguredSystem>(99);
        });

        coordinator.destroy_system<ConfiguredSystem>();
        assert(!coordinator.system_exists<ConfiguredSystem>());
        assert(coordinator.system_count() == 0);
    }

    return 0;
}
