#include "liquid/world/World.hpp"

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
        World world;

        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{40});

        BehaviorId behavior = world.create_behavior();
        world.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);
        ComponentSlotId slot = world.get_components(lightType, behavior).at("officeLight");
        IntentId intent = world.create_intent(behavior, lightType, slot, IntentLifetime::persistent(), Light{80});

        assert(behavior == 0);
        assert(world.behavior_exists(behavior));
        assert(world.behavior_count() == 1);
        assert(world.intent_exists(intent));
        assert(world.intent_owner(intent) == behavior);
        assert(world.intent_target(intent) == (ComponentTarget{lightType.id, slot}));
        assert(world.typed_intent(lightType, intent).value.brightness == 80);
        assert(world.intent_count(behavior) == 1);

        expect_throw([&] {
            world.create_intent(42, lightType, slot, IntentLifetime::persistent(), Light{10});
        });

        expect_throw([&] {
            world.intent_count(42);
        });

        expect_throw([&] {
            world.destroy_behavior(42);
        });

        world.destroy_behavior(behavior);

        assert(!world.behavior_exists(behavior));
        assert(!world.intent_exists(intent));
        assert(world.behavior_count() == 0);

        expect_throw([&] {
            world.intent_count(behavior);
        });

        BehaviorId recycled = world.create_behavior();
        world.grant_component_access(lightType, recycled, "officeLight", ComponentAccessMode::ReadWrite);
        IntentId recreatedIntent = world.create_intent(recycled, lightType, slot, IntentLifetime::persistent(), Light{90});

        assert(recycled == behavior);
        assert(recreatedIntent == intent);
        assert(world.intent_exists(recreatedIntent));
        assert(world.intent_count(recycled) == 1);
    }

    {
        World world;

        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{40});

        BehaviorId behavior = world.create_behavior();
        world.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);
        ComponentSlotId slot = world.get_components(lightType, behavior).at("officeLight");
        IntentId intent = world.create_intent(behavior, lightType, slot, IntentLifetime::persistent(), Light{80});

        world.destroy_intent(intent);

        assert(world.behavior_exists(behavior));
        assert(!world.intent_exists(intent));
        assert(world.behavior_count() == 1);
        assert(world.intent_count(behavior) == 0);
    }

    {
        World world;

        ComponentType<Light> lightType = world.register_component<Light>("Light");
        ComponentType<Temperature> temperatureType = world.register_component<Temperature>("Temperature");

        world.add_component(lightType, "officeLight", Light{40});
        world.add_component(lightType, "deskLight", Light{80});
        world.add_component(temperatureType, "officeTemperature", Temperature{22});

        BehaviorId trackingBehavior = world.create_behavior();
        BehaviorId focusBehavior = world.create_behavior();

        world.register_system<TrackingSystem>();
        Signature lightSignature;
        lightSignature.set(lightType.id);
        world.set_system_signature<TrackingSystem>(lightSignature);

        auto& trackingSystem = world.get_system<TrackingSystem>();

        expect_throw([&] {
            world.get_components(lightType, 99);
        });

        expect_throw([&] {
            world.grant_component_access(lightType, 99, "officeLight", ComponentAccessMode::Read);
        });

        world.grant_component_access(lightType, trackingBehavior, "officeLight", ComponentAccessMode::ReadWrite);
        world.grant_component_access(lightType, trackingBehavior, "deskLight", ComponentAccessMode::Read);
        world.grant_component_access(lightType, focusBehavior, "officeLight", ComponentAccessMode::Read);
        world.grant_component_access(temperatureType, trackingBehavior, "officeTemperature", ComponentAccessMode::Read);

        auto trackingLights = world.get_components(lightType, trackingBehavior);
        auto focusLights = world.get_components(lightType, focusBehavior);

        assert(trackingLights.size() == 2);
        assert(focusLights.size() == 1);
        assert(trackingLights.at("officeLight") == 0);
        assert(trackingLights.at("deskLight") == 1);
        assert(focusLights.at("officeLight") == 0);
        assert(world.resolve_component(lightType, trackingLights.at("officeLight"))->brightness == 40);
        assert(world.read_component(lightType, focusBehavior, "officeLight")->brightness == 40);
        assert(world.write_component(lightType, trackingBehavior, "officeLight")->brightness == 40);

        expect_throw([&] {
            world.write_component(lightType, focusBehavior, "officeLight");
        });

        expect_throw([&] {
            world.read_component(lightType, 99, "officeLight");
        });

        expect_throw([&] {
            world.revoke_component_access(lightType, 99, "officeLight");
        });

        assert(world.can_read_component(lightType, focusBehavior, "officeLight"));
        assert(!world.can_write_component(lightType, focusBehavior, "officeLight"));
        assert(world.can_write_component(lightType, trackingBehavior, "officeLight"));
        assert(world.behavior_signature(trackingBehavior).test(lightType.id));
        assert(world.behavior_signature(trackingBehavior).test(temperatureType.id));
        assert(world.behavior_signature(focusBehavior).test(lightType.id));
        assert(world.system_behavior_count<TrackingSystem>() == 2);
        assert(world.system_has_behavior<TrackingSystem>(trackingBehavior));
        assert(world.system_has_behavior<TrackingSystem>(focusBehavior));
        assert((trackingSystem.events == std::vector<std::string>{"+0", "+1"}));

        world.remove_component(lightType, "officeLight");

        assert(!world.has_component_named(lightType, "officeLight"));
        assert(world.get_components(lightType, focusBehavior).empty());
        assert(!world.behavior_signature(focusBehavior).test(lightType.id));
        assert(!world.system_has_behavior<TrackingSystem>(focusBehavior));
        assert(world.behavior_signature(trackingBehavior).test(lightType.id));
        assert(world.system_has_behavior<TrackingSystem>(trackingBehavior));
        assert(world.get_components(lightType, trackingBehavior).size() == 1);
        assert(world.get_components(lightType, trackingBehavior).at("deskLight") == 1);

        world.add_component(lightType, "taskLight", Light{55});
        world.grant_component_access(lightType, focusBehavior, "taskLight", ComponentAccessMode::ReadWrite);

        auto reusedLights = world.get_components(lightType, focusBehavior);
        assert(reusedLights.size() == 1);
        assert(reusedLights.at("taskLight") == 0);
        assert(world.system_has_behavior<TrackingSystem>(focusBehavior));
        assert(world.resolve_component(lightType, reusedLights.at("taskLight"))->brightness == 55);

        world.revoke_component_access(lightType, focusBehavior, "taskLight");

        assert(world.get_components(lightType, focusBehavior).empty());
        assert(!world.behavior_signature(focusBehavior).test(lightType.id));
        assert(!world.system_has_behavior<TrackingSystem>(focusBehavior));

        world.remove_component(lightType, "deskLight");

        assert(world.get_components(lightType, trackingBehavior).empty());
        assert(!world.behavior_signature(trackingBehavior).test(lightType.id));
        assert(!world.system_has_behavior<TrackingSystem>(trackingBehavior));
        assert(world.behavior_signature(trackingBehavior).test(temperatureType.id));
    }

    {
        World world;

        ComponentType<Temperature> temperatureType = world.register_component<Temperature>("Temperature");
        world.add_component(temperatureType, "officeTemperature", Temperature{22});

        world.register_system<TemperatureSystem>();
        Signature temperatureSignature;
        temperatureSignature.set(temperatureType.id);
        world.set_system_signature<TemperatureSystem>(temperatureSignature);

        BehaviorId first = world.create_behavior();
        BehaviorId second = world.create_behavior();

        world.grant_component_access(temperatureType, first, "officeTemperature", ComponentAccessMode::ReadWrite);
        world.grant_component_access(temperatureType, second, "officeTemperature", ComponentAccessMode::Read);
        ComponentSlotId temperatureSlot = world.get_components(temperatureType, first).at("officeTemperature");
        IntentId intent = world.create_intent(first, temperatureType, temperatureSlot, IntentLifetime::persistent(), Temperature{24});

        assert(world.system_has_behavior<TemperatureSystem>(first));
        assert(world.system_has_behavior<TemperatureSystem>(second));

        world.destroy_behavior(first);

        assert(!world.behavior_exists(first));
        assert(!world.intent_exists(intent));
        assert(!world.system_has_behavior<TemperatureSystem>(first));
        assert(world.system_has_behavior<TemperatureSystem>(second));

        expect_throw([&] {
            world.get_components(temperatureType, first);
        });

        BehaviorId recycled = world.create_behavior();

        assert(recycled == first);
        assert(world.get_components(temperatureType, recycled).empty());
        assert(!world.behavior_signature(recycled).test(temperatureType.id));
        assert(!world.system_has_behavior<TemperatureSystem>(recycled));
    }

    {
        World world;

        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{40});

        BehaviorId reader = world.create_behavior();
        BehaviorId writer = world.create_behavior();

        world.grant_component_access(lightType, reader, "officeLight", ComponentAccessMode::Read);
        world.grant_component_access(lightType, writer, "officeLight", ComponentAccessMode::Write);

        ComponentSlotId slot = world.get_components(lightType, writer).at("officeLight");

        expect_throw([&] {
            world.create_intent(reader, lightType, slot, IntentLifetime::persistent(), Light{10});
        });

        IntentId writerIntent = world.create_intent(writer, lightType, slot, IntentLifetime::persistent(), Light{80});
        assert(world.intent_exists(writerIntent));

        world.grant_component_access(lightType, writer, "officeLight", ComponentAccessMode::Read);

        assert(!world.intent_exists(writerIntent));

        expect_throw([&] {
            world.create_intent(writer, lightType, slot, IntentLifetime::persistent(), Light{90});
        });
    }

    {
        World world;

        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{40});

        BehaviorId behavior = world.create_behavior();
        world.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);

        ComponentSlotId slot = world.get_components(lightType, behavior).at("officeLight");
        IntentId intent = world.create_intent(behavior, lightType, slot, IntentLifetime::persistent(), Light{80});

        world.remove_component(lightType, "officeLight");

        assert(!world.intent_exists(intent));
        assert(world.intents_for(lightType.id, slot).empty());

        world.add_component(lightType, "taskLight", Light{55});
        world.grant_component_access(lightType, behavior, "taskLight", ComponentAccessMode::ReadWrite);

        ComponentSlotId reusedSlot = world.get_components(lightType, behavior).at("taskLight");
        assert(reusedSlot == slot);
        assert(world.intents_for(lightType.id, reusedSlot).empty());
    }

    {
        World world;

        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{40});

        BehaviorId behavior = world.create_behavior();
        world.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);

        ComponentSlotId slot = world.get_components(lightType, behavior).at("officeLight");
        IntentId intent = world.create_intent(behavior, lightType, slot, IntentLifetime::persistent(), Light{80});

        world.revoke_component_access(lightType, behavior, "officeLight");

        assert(!world.intent_exists(intent));

        expect_throw([&] {
            world.create_intent(behavior, lightType, slot, IntentLifetime::persistent(), Light{90});
        });
    }

    {
        World world;

        world.register_system<ConfiguredSystem>(4, "manual");
        auto& configured = world.get_system<ConfiguredSystem>();
        assert(configured.threshold == 4);
        assert(configured.label == "manual");

        BehaviorId behavior = world.create_behavior();

        world.add_behavior_to_system<ConfiguredSystem>(behavior);
        world.add_behavior_to_system<ConfiguredSystem>(behavior);

        assert(world.system_has_behavior<ConfiguredSystem>(behavior));
        assert(world.system_behavior_count<ConfiguredSystem>() == 1);

        world.remove_behavior_from_system<ConfiguredSystem>(behavior);
        world.remove_behavior_from_system<ConfiguredSystem>(behavior);

        assert(!world.system_has_behavior<ConfiguredSystem>(behavior));

        expect_throw([&] {
            world.add_behavior_to_system<ConfiguredSystem>(99);
        });

        expect_throw([&] {
            world.remove_behavior_from_system<ConfiguredSystem>(99);
        });

        world.destroy_system<ConfiguredSystem>();
        assert(!world.system_exists<ConfiguredSystem>());
        assert(world.system_count() == 0);
    }

    return 0;
}
