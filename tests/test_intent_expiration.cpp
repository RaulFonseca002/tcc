#include "liquid/Coordinator.hpp"
#include "liquid/IntentExpiration.hpp"
#include "liquid/IntentRegistry.hpp"

#include <cassert>
#include <vector>

struct Light {
    int brightness = 0;
};

std::size_t destroy_expired(IntentRegistry& intents, IntentTime now)
{
    std::vector<IntentId> expired;

    for (IntentId id : intents.live_intent_ids()) {
        if (expiration_for(intents.intent(id), now).status == IntentStatus::Expired)
            expired.push_back(id);
    }

    for (IntentId id : expired)
        intents.destroy(id);

    return expired.size();
}

int main()
{
    {
        IntentLifetime persistent = IntentLifetime::persistent();
        IntentLifetime timed = IntentLifetime::until_time(10);
        IntentLifetime untilCancelled = IntentLifetime::until_cancelled();

        assert(persistent.kind == IntentLifetimeKind::Persistent);
        assert(timed.kind == IntentLifetimeKind::UntilTime);
        assert(timed.expiresAt == 10);
        assert(untilCancelled.kind == IntentLifetimeKind::UntilCancelled);

        assert(expiration_for(persistent, 100).status == IntentStatus::Alive);
        assert(expiration_for(untilCancelled, 100).status == IntentStatus::Alive);
        assert(expiration_for(timed, 9).status == IntentStatus::Alive);
        assert(expiration_for(timed, 10).status == IntentStatus::Expired);
        assert(expiration_for(timed, 10).reason == IntentExpirationReason::TimeReached);
    }

    {
        IntentRegistry intents;
        BehaviorId owner = 4;
        ComponentType<Light> lightType{2};
        ComponentSlotId slot = 8;

        intents.addBehaviour(owner);

        IntentId persistent = intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{10});
        IntentId timed = intents.create(owner, lightType, slot, IntentLifetime::until_time(20), Light{20});
        IntentId untilCancelled = intents.create(owner, lightType, slot, IntentLifetime::until_cancelled(), Light{30});

        assert(intents.size() == 3);
        assert(intents.intents_for(lightType.id, slot).size() == 3);
        assert(expiration_for(intents.intent(timed), 19).status == IntentStatus::Alive);
        assert(expiration_for(intents.intent(timed), 20).status == IntentStatus::Expired);
        assert(expiration_for(intents.intent(untilCancelled), 20).status == IntentStatus::Alive);

        assert(destroy_expired(intents, 20) == 1);
        assert(intents.exists(persistent));
        assert(!intents.exists(timed));
        assert(intents.exists(untilCancelled));
        assert(intents.size(owner) == 2);

        intents.destroy(untilCancelled);
        assert(!intents.exists(untilCancelled));
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
        IntentId untilCancelled = coordinator.create_intent(behavior, lightType, slot, IntentLifetime::until_cancelled(), Light{20});

        assert(coordinator.intent_target(timed) == (ComponentTarget{lightType.id, slot}));
        assert(coordinator.intents_for(lightType.id, slot).size() == 2);
        assert(coordinator.typed_intent(lightType, timed).value.brightness == 80);
        assert(expiration_for(coordinator.intent(timed), 5).status == IntentStatus::Expired);
        assert(expiration_for(coordinator.intent(untilCancelled), 5).status == IntentStatus::Alive);

        coordinator.destroy_intent(timed);
        assert(!coordinator.intent_exists(timed));
        assert(coordinator.intent_exists(untilCancelled));

        coordinator.destroy_intent(untilCancelled);
        assert(!coordinator.intent_exists(untilCancelled));
    }

    return 0;
}
