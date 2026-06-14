#include "liquid/ComponentRegistry.hpp"

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

struct Light {
    int brightness = 0;
};

struct Temperature {
    int celsius = 0;
};

struct Marker {
    int value = 0;
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
        ComponentRegistry registry;

        expect_throw([&] {
            registry.register_component<Light>("");
        });

        ComponentType<Light> lightType = registry.register_component<Light>("Light");
        ComponentType<Temperature> temperatureType = registry.register_component<Temperature>("Temperature");

        assert(lightType.id == 0);
        assert(temperatureType.id == 1);
        assert(registry.component_type("Light") == lightType);
        assert(registry.component_type(lightType) == lightType.id);

        expect_throw([&] {
            registry.register_component<Light>("Light");
        });

        expect_throw([&] {
            registry.component_type("Missing");
        });
    }

    {
        ComponentRegistry registry;

        for (std::size_t i = 0; i < MAX_COMPONENT_TYPES; ++i) {
            ComponentType<Marker> type = registry.register_component<Marker>("Marker" + std::to_string(i));
            assert(type.id == i);
        }

        expect_throw([&] {
            registry.register_component<Marker>("Overflow");
        });
    }

    {
        ComponentRegistry registry;

        ComponentType<Light> lightType = registry.register_component<Light>("Light");
        ComponentType<Temperature> temperatureType = registry.register_component<Temperature>("Temperature");

        expect_throw([&] {
            registry.add_component(lightType, "", Light{1});
        });

        registry.add_component(lightType, "officeLight", Light{40});
        registry.add_component(temperatureType, "officeTemperature", Temperature{22});

        assert(registry.has_component_named(lightType, "officeLight"));
        assert(!registry.has_component_named(lightType, "missing"));
        assert(registry.get_component_named(lightType, "officeLight")->brightness == 40);
        assert(static_cast<const ComponentRegistry&>(registry).get_component_named(lightType, "officeLight")->brightness == 40);

        registry.get_component_named(lightType, "officeLight")->brightness = 45;
        assert(registry.get_component_named(lightType, "officeLight")->brightness == 45);

        expect_throw([&] {
            registry.add_component(lightType, "officeLight", Light{80});
        });

        expect_throw([&] {
            registry.get_component_named(lightType, "missing");
        });

        expect_throw([&] {
            registry.remove_component(lightType, "missing");
        });

        ComponentType<Temperature> wrongType{lightType.id};
        expect_throw([&] {
            registry.component_type(wrongType);
        });

        expect_throw([&] {
            registry.add_component(wrongType, "badTemperature", Temperature{1});
        });

        assert(!registry.has_component_named(wrongType, "officeLight"));

        ComponentType<Light> invalidType;
        expect_throw([&] {
            registry.add_component(invalidType, "badLight", Light{1});
        });

        assert(!registry.has_component_named(invalidType, "badLight"));
        assert(!registry.can_read(invalidType, 1, "badLight"));
        assert(!registry.can_write(invalidType, 1, "badLight"));
    }

    {
        ComponentRegistry registry;

        ComponentType<Light> lightType = registry.register_component<Light>("Light");
        ComponentType<Temperature> temperatureType = registry.register_component<Temperature>("Temperature");

        registry.add_component(lightType, "officeLight", Light{40});
        registry.add_component(lightType, "deskLight", Light{70});
        registry.add_component(temperatureType, "officeTemperature", Temperature{22});

        BehaviorId trackingBehavior = 3;
        BehaviorId focusBehavior = 1;
        BehaviorId writeOnlyBehavior = 2;

        registry.grant_access(lightType, trackingBehavior, "officeLight", ComponentAccessMode::ReadWrite);
        registry.grant_access(lightType, focusBehavior, "officeLight", ComponentAccessMode::Read);
        registry.grant_access(lightType, writeOnlyBehavior, "officeLight", ComponentAccessMode::Write);
        registry.grant_access(temperatureType, trackingBehavior, "officeTemperature", ComponentAccessMode::Read);

        auto trackingLights = registry.get_components(lightType, trackingBehavior);
        auto focusLights = registry.get_components(lightType, focusBehavior);
        auto trackingTemperatures = registry.get_components(temperatureType, trackingBehavior);

        assert(trackingLights.size() == 1);
        assert(focusLights.size() == 1);
        assert(trackingTemperatures.size() == 1);
        assert(trackingLights.at("officeLight") == 0);
        assert(focusLights.at("officeLight") == 0);
        assert(trackingTemperatures.at("officeTemperature") == 0);
        assert(registry.resolve_component(lightType, trackingLights.at("officeLight")) == registry.get_component_named(lightType, "officeLight"));
        assert(static_cast<const ComponentRegistry&>(registry).resolve_component(lightType, focusLights.at("officeLight")) == registry.get_component_named(lightType, "officeLight"));

        assert(registry.can_read(lightType, trackingBehavior, "officeLight"));
        assert(registry.can_write(lightType, trackingBehavior, "officeLight"));
        assert(registry.can_read(lightType, focusBehavior, "officeLight"));
        assert(!registry.can_write(lightType, focusBehavior, "officeLight"));
        assert(!registry.can_read(lightType, writeOnlyBehavior, "officeLight"));
        assert(registry.can_write(lightType, writeOnlyBehavior, "officeLight"));
        assert(!registry.can_read(lightType, 99, "officeLight"));
        assert(!registry.can_write(lightType, 99, "officeLight"));
        assert(!registry.can_read(lightType, trackingBehavior, "missing"));
        assert(!registry.can_write(lightType, trackingBehavior, "missing"));

        std::vector<BehaviorId> behaviors = registry.behaviors_with_access(lightType);
        assert((behaviors == std::vector<BehaviorId>{focusBehavior, writeOnlyBehavior, trackingBehavior}));

        registry.grant_access(lightType, focusBehavior, "officeLight", ComponentAccessMode::Write);

        focusLights = registry.get_components(lightType, focusBehavior);
        assert(focusLights.size() == 1);
        assert(!registry.can_read(lightType, focusBehavior, "officeLight"));
        assert(registry.can_write(lightType, focusBehavior, "officeLight"));

        registry.revoke_access(lightType, focusBehavior, "officeLight");

        assert(registry.get_components(lightType, focusBehavior).empty());
        assert(!registry.can_read(lightType, focusBehavior, "officeLight"));
        assert(!registry.can_write(lightType, focusBehavior, "officeLight"));

        registry.remove_behavior(trackingBehavior);

        assert(registry.get_components(lightType, trackingBehavior).empty());
        assert(registry.get_components(temperatureType, trackingBehavior).empty());
        assert(!registry.can_read(lightType, trackingBehavior, "officeLight"));
        assert(!registry.can_read(temperatureType, trackingBehavior, "officeTemperature"));
    }

    {
        ComponentRegistry registry;

        ComponentType<Light> lightType = registry.register_component<Light>("Light");
        registry.add_component(lightType, "officeLight", Light{40});
        registry.add_component(lightType, "deskLight", Light{80});

        BehaviorId trackingBehavior = 1;
        BehaviorId focusBehavior = 2;
        registry.grant_access(lightType, trackingBehavior, "officeLight", ComponentAccessMode::ReadWrite);
        registry.grant_access(lightType, focusBehavior, "officeLight", ComponentAccessMode::Read);

        registry.remove_component(lightType, "officeLight");

        assert(!registry.has_component_named(lightType, "officeLight"));
        assert(registry.get_components(lightType, trackingBehavior).empty());
        assert(registry.get_components(lightType, focusBehavior).empty());
        assert(registry.behaviors_with_access(lightType).empty());
        assert(registry.resolve_component(lightType, 0) == nullptr);

        registry.add_component(lightType, "reusedLight", Light{90});
        auto reused = registry.get_component_named(lightType, "reusedLight");
        assert(reused != nullptr);
        assert(reused->brightness == 90);

        registry.grant_access(lightType, trackingBehavior, "reusedLight", ComponentAccessMode::Read);
        auto lights = registry.get_components(lightType, trackingBehavior);
        assert(lights.size() == 1);
        assert(lights.at("reusedLight") == 0);
    }

    return 0;
}
