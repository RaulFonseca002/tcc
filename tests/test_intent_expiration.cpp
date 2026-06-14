#include "liquid/Coordinator.hpp"
#include "liquid/IntentExpiration.hpp"
#include "liquid/IntentRegistry.hpp"

#include <cassert>
#include <type_traits>

struct Light {
    int brightness = 0;
};

bool contains(const std::vector<IntentId>& ids, IntentId wanted)
{
    for (IntentId id : ids) {
        if (id == wanted)
            return true;
    }

    return false;
}

int main()
{
    {
        static_assert(!std::is_default_constructible_v<IntentLifetime>);

        IntentLifetime persistent = IntentLifetime::persistent();
        IntentLifetime timed = IntentLifetime::until_time(10);

        assert(persistent.kind == IntentLifetimeKind::Persistent);
        assert(timed.kind == IntentLifetimeKind::UntilTime);
        assert(timed.expiresAt == 10);
    }

    {
        IntentRegistry intents;
        BehaviorId owner = 4;
        ComponentType<Light> lightType{2};
        ComponentSlotId slot = 8;

        intents.create_behavior_pool(owner);

        IntentId persistent = intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{10});
        IntentId timed = intents.create(owner, lightType, slot, IntentLifetime::until_time(20), Light{20});
        IntentId later = intents.create(owner, lightType, slot, IntentLifetime::until_time(30), Light{30});

        assert(intents.size() == 3);
        assert(intents.intents_for(lightType.id, slot).size() == 3);

        std::vector<IntentId> beforeExpiration = expired_intent_ids(intents, 19);
        assert(beforeExpiration.empty());

        std::vector<IntentId> expired = expired_intent_ids(intents, 20);
        assert(expired.size() == 1);
        assert(contains(expired, timed));

        assert(destroy_expired_intents(intents, 20) == 1);
        assert(intents.exists(persistent));
        assert(!intents.exists(timed));
        assert(intents.exists(later));
        assert(intents.size(owner) == 2);

        intents.destroy(later);
        assert(!intents.exists(later));
        assert(intents.size(owner) == 1);
    }

    {
        Coordinator coordinator;
        ComponentType<Light> lightType = coordinator.register_component<Light>("Light");

        coordinator.add_component(lightType, "officeLight", Light{50});

        BehaviorId behavior = coordinator.create_behavior();
        coordinator.grant_component_access(lightType, behavior, "officeLight", ComponentAccessMode::ReadWrite);

        ComponentSlotId slot = coordinator.get_components(lightType, behavior).at("officeLight");

        IntentId timed = coordinator.create_intent(behavior, lightType, slot, IntentLifetime::until_time(5), Light{80});
        IntentId persistent = coordinator.create_intent(behavior, lightType, slot, IntentLifetime::persistent(), Light{20});

        assert(coordinator.intent_target(timed) == (ComponentTarget{lightType.id, slot}));
        assert(coordinator.intents_for(lightType.id, slot).size() == 2);
        assert(coordinator.typed_intent(lightType, timed).value.brightness == 80);

        std::vector<IntentId> expired = expired_intent_ids(coordinator, 5);
        assert(expired.size() == 1);
        assert(contains(expired, timed));

        assert(destroy_expired_intents(coordinator, 5) == 1);
        assert(!coordinator.intent_exists(timed));
        assert(coordinator.intent_exists(persistent));

        coordinator.destroy_intent(persistent);
        assert(!coordinator.intent_exists(persistent));
    }

    return 0;
}
