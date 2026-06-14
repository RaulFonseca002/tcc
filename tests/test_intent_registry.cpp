#include "liquid/IntentRegistry.hpp"

#include <cassert>
#include <cstddef>
#include <set>

struct Light {
    int brightness = 0;
};

struct Temperature {
    int celsius = 0;
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
        IntentRegistry intents;

        BehaviorId owner = 7;
        ComponentType<Light> lightType{2};
        ComponentSlotId slot = 4;
        intents.addBehaviour(owner);

        IntentId id = intents.create(owner, lightType, slot, IntentLifetime::until_cancelled(), Light{60});

        assert(intents.exists(id));
        assert(intents.owner_of(id) == owner);
        assert(intents.target_of(id) == (ComponentTarget{lightType.id, slot}));
        assert(intents.lifetime_of(id).kind == IntentLifetimeKind::UntilCancelled);
        assert(intents.intent(id).id == id);
        assert(intents.intent(id).owner == owner);
        assert(intents.typed_intent(lightType, id).value.brightness == 60);
        assert(intents.intents_for(lightType.id, slot).size() == 1);
        assert(intents.target_index().at(lightType.id).at(slot).contains(id));
        assert(intents.size(owner) == 1);
        assert(!intents.exists(id + 100));
    }

    {
        IntentRegistry intents;

        BehaviorId owner = 1;
        ComponentType<Light> lightType{0};
        ComponentSlotId slot = 2;
        intents.addBehaviour(owner);

        IntentId id = intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{10});
        intents.destroy(id);

        assert(!intents.exists(id));
        assert(intents.size(owner) == 0);
        assert(intents.intents_for(lightType.id, slot).empty());

        IntentId recycled = intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{20});
        assert(recycled == id);
        assert(intents.exists(recycled));
        assert(intents.typed_intent(lightType, recycled).value.brightness == 20);
    }

    {
        IntentRegistry intents;

        BehaviorId owner = 3;
        ComponentType<Temperature> temperatureType{1};
        ComponentSlotId slot = 6;
        intents.addBehaviour(owner);

        IntentId beforeReset = intents.create(owner, temperatureType, slot, IntentLifetime::until_time(30), Temperature{22});
        assert(intents.exists(beforeReset));

        intents.addBehaviour(owner);

        assert(!intents.exists(beforeReset));
        assert(intents.size(owner) == 0);
        assert(intents.intents_for(temperatureType.id, slot).empty());

        IntentId afterReset = intents.create(owner, temperatureType, slot, IntentLifetime::until_time(30), Temperature{24});
        assert(afterReset == beforeReset);
        assert(intents.exists(afterReset));
    }

    {
        IntentRegistry intents;

        BehaviorId ownerOne = 1;
        BehaviorId ownerTwo = 2;
        ComponentType<Light> lightType{0};
        ComponentSlotId slot = 9;
        intents.addBehaviour(ownerOne);
        intents.addBehaviour(ownerTwo);

        IntentId ownedByOne = intents.create(ownerOne, lightType, slot, IntentLifetime::persistent(), Light{1});
        IntentId alsoOwnedByOne = intents.create(ownerOne, lightType, slot, IntentLifetime::persistent(), Light{2});
        IntentId ownedByTwo = intents.create(ownerTwo, lightType, slot, IntentLifetime::persistent(), Light{3});

        intents.destroy_owned_by(ownerOne);

        assert(!intents.exists(ownedByOne));
        assert(!intents.exists(alsoOwnedByOne));
        assert(intents.exists(ownedByTwo));
        assert(intents.size(ownerOne) == 0);
        assert(intents.size(ownerTwo) == 1);
        assert(intents.intents_for(lightType.id, slot).size() == 1);

        expect_throw([&] {
            intents.create(ownerOne, lightType, slot, IntentLifetime::persistent(), Light{4});
        });

        intents.addBehaviour(ownerOne);
        IntentId recreated = intents.create(ownerOne, lightType, slot, IntentLifetime::persistent(), Light{5});

        assert(intents.owner_of(recreated) == ownerOne);
        assert(intents.exists(recreated));
    }

    {
        IntentRegistry intents;

        IntentId missing = 99;
        assert(!intents.exists(missing));
        expect_throw([&] {
            intents.owner_of(missing);
        });
        assert(intents.size(99) == 0);
    }

    {
        IntentRegistry intents;
        BehaviorId owner = 11;
        ComponentType<Light> lightType{0};
        ComponentSlotId slot = 1;
        intents.addBehaviour(owner);
        std::set<IntentId> created;

        for (std::size_t i = 0; i < MAX_INTENTS; ++i) {
            IntentId id = intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{static_cast<int>(i)});
            assert(created.insert(id).second);
            assert(intents.exists(id));
            assert(intents.owner_of(id) == owner);
        }

        assert(intents.size(owner) == MAX_INTENTS);
        assert(intents.size() == MAX_INTENTS);

        expect_throw([&] {
            intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{0});
        });

        IntentId recycled = *created.begin();
        intents.destroy(recycled);
        assert(intents.size(owner) == MAX_INTENTS - 1);
        assert(!intents.exists(recycled));
        assert(intents.create(owner, lightType, slot, IntentLifetime::persistent(), Light{42}) == recycled);
        assert(intents.size(owner) == MAX_INTENTS);
    }

    return 0;
}
