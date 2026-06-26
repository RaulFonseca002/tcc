#include "liquid/Runtime.hpp"

#include <cassert>
#include <map>
#include <string>
#include <vector>

struct Light {
    int level = 0;
};

struct CapturedFrame {
    FrameNumber frame = 0;
    IntentTime now = 0;
};

struct FrameCaptureSystem : System {
    std::vector<CapturedFrame> frames;

    void run(World& world, FrameNumber frame, IntentTime now) override {
        (void)world;
        frames.push_back({frame, now});
    }
};

struct FirstOrderSystem : System {
    std::vector<std::string>* order = nullptr;

    explicit FirstOrderSystem(std::vector<std::string>* order)
        : order(order)
    {
    }

    void run(World& world, FrameNumber frame, IntentTime now) override {
        (void)world;
        (void)now;
        order->push_back("first:" + std::to_string(frame));
    }
};

struct SecondOrderSystem : System {
    std::vector<std::string>* order = nullptr;

    explicit SecondOrderSystem(std::vector<std::string>* order)
        : order(order)
    {
    }

    void run(World& world, FrameNumber frame, IntentTime now) override {
        (void)world;
        (void)now;
        order->push_back("second:" + std::to_string(frame));
    }
};

struct IntentCreatingSystem : System {
    ComponentType<Light> type;
    BehaviorId owner = 0;
    ComponentSlotId slot = InvalidComponentSlotId;
    IntentId created = 0;

    IntentCreatingSystem(ComponentType<Light> type, BehaviorId owner, ComponentSlotId slot)
        : type(type),
          owner(owner),
          slot(slot)
    {
    }

    void run(World& world, FrameNumber frame, IntentTime now) override {
        (void)frame;

        if (created != 0)
            return;

        created = world.create_intent(
            owner,
            type,
            slot,
            IntentLifetime::persistent(),
            Light{static_cast<int>(now)}
        );
    }
};

std::map<ComponentTypeId, std::map<ComponentName, ComponentSlotId>> resolutions_for(
    ComponentType<Light> type,
    ComponentSlotId slot
) {
    return {{type.id, {{"officeLight", slot}}}};
}

int main()
{
    {
        Runtime runtime;
        World& world = runtime.world();
        world.register_system<FrameCaptureSystem>();

        FrameLog first = runtime.run_frame(10);
        FrameLog second = runtime.run_frame(20);

        auto& system = world.get_system<FrameCaptureSystem>();

        assert(first.frame == 0);
        assert(first.now == 10);
        assert(second.frame == 1);
        assert(second.now == 20);
        assert(runtime.frame() == 2);
        assert(system.frames.size() == 2);
        assert(system.frames[0].frame == 0);
        assert(system.frames[0].now == 10);
        assert(system.frames[1].frame == 1);
        assert(system.frames[1].now == 20);

        std::vector<std::string> expectedPhases{
            "begin_frame",
            "expire_intents",
            "resolve_intents",
            "run_systems",
            "end_frame"
        };
        assert(first.phases == expectedPhases);
        assert(runtime.last_frame_log().frame == 1);
    }

    {
        Runtime runtime;
        World& world = runtime.world();
        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{0});

        BehaviorId behavior = world.create_behavior();
        world.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);
        ComponentSlotId slot = world.get_components(lightType, behavior).at("officeLight");

        IntentId persistent = world.create_intent(
            behavior,
            lightType,
            slot,
            IntentLifetime::persistent(),
            Light{10},
            IntentPriority::Low
        );
        IntentId expired = world.create_intent(
            behavior,
            lightType,
            slot,
            IntentLifetime::until_time(5),
            Light{90},
            IntentPriority::High
        );

        FrameLog log = runtime.run_frame(5, resolutions_for(lightType, slot));

        assert(world.intent_exists(persistent));
        assert(!world.intent_exists(expired));
        assert(log.expired_intents == 1);
        assert(log.resolution_requests == 1);
        assert(log.selected_intents == 1);
        assert(log.intent_selections.size() == 1);
        assert(log.intent_selections.at(lightType.id).at("officeLight") == persistent);
        assert(world.get_component_named(lightType, "officeLight")->level == 0);
    }

    {
        Runtime runtime;
        World& world = runtime.world();
        std::vector<std::string> order;

        world.register_system<FirstOrderSystem>(&order);
        world.register_system<SecondOrderSystem>(&order);

        FrameLog log = runtime.run_frame(2);

        assert(log.systems_run == 2);
        assert((order == std::vector<std::string>{"first:0", "second:0"}));
    }

    {
        Runtime runtime;
        World& world = runtime.world();
        ComponentType<Light> lightType = world.register_component<Light>("Light");
        world.add_component(lightType, "officeLight", Light{0});

        BehaviorId behavior = world.create_behavior();
        world.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);
        ComponentSlotId slot = world.get_components(lightType, behavior).at("officeLight");

        world.register_system<IntentCreatingSystem>(lightType, behavior, slot);

        FrameLog first = runtime.run_frame(42, resolutions_for(lightType, slot));
        auto& system = world.get_system<IntentCreatingSystem>();

        assert(first.selected_intents == 0);
        assert(system.created != 0);
        assert(world.intent_exists(system.created));

        FrameLog second = runtime.run_frame(43, resolutions_for(lightType, slot));

        assert(second.selected_intents == 1);
        assert(second.intent_selections.at(lightType.id).at("officeLight") == system.created);
    }

    return 0;
}
