#include "liquid/IntentRegistry.hpp"

#include <cassert>
#include <cstddef>
#include <set>

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

std::uint16_t local_intent(IntentId id)
{
    return static_cast<std::uint16_t>(id & LOW_16_BITS);
}

int main()
{
    {
        IntentRegistry intents;

        BehaviorId owner = 7;
        intents.addBehaviour(owner);

        IntentId id = intents.create(owner);

        assert(intents.exists(id));
        assert(intents.owner_of(id) == owner);
        assert((id >> INTENT_OWNER_SHIFT) == owner);
        assert(local_intent(id) < MAX_INTENTS);
        assert(intents.size(owner) == 1);
        assert(!intents.exists((static_cast<IntentId>(owner) << INTENT_OWNER_SHIFT) | 42u));
    }

    {
        IntentRegistry intents;

        BehaviorId owner = 1;
        intents.addBehaviour(owner);

        IntentId id = intents.create(owner);
        intents.destroy(id);

        assert(!intents.exists(id));
        assert(intents.size(owner) == 0);

        IntentId recycled = intents.create(owner);
        assert(recycled == id);
        assert(intents.exists(recycled));
    }

    {
        IntentRegistry intents;

        BehaviorId owner = 3;
        intents.addBehaviour(owner);

        IntentId beforeReset = intents.create(owner);
        assert(intents.exists(beforeReset));

        intents.addBehaviour(owner);

        assert(!intents.exists(beforeReset));
        assert(intents.size(owner) == 0);

        IntentId afterReset = intents.create(owner);
        assert(afterReset == beforeReset);
        assert(intents.exists(afterReset));
    }

    {
        IntentRegistry intents;

        BehaviorId ownerOne = 1;
        BehaviorId ownerTwo = 2;
        intents.addBehaviour(ownerOne);
        intents.addBehaviour(ownerTwo);

        IntentId ownedByOne = intents.create(ownerOne);
        IntentId alsoOwnedByOne = intents.create(ownerOne);
        IntentId ownedByTwo = intents.create(ownerTwo);

        intents.destroy_owned_by(ownerOne);

        assert(!intents.exists(ownedByOne));
        assert(!intents.exists(alsoOwnedByOne));
        assert(intents.exists(ownedByTwo));
        assert(intents.size(ownerOne) == 0);
        assert(intents.size(ownerTwo) == 1);

        expect_throw([&] {
            intents.create(ownerOne);
        });

        intents.addBehaviour(ownerOne);
        IntentId recreated = intents.create(ownerOne);

        assert(intents.owner_of(recreated) == ownerOne);
        assert(intents.exists(recreated));
    }

    {
        IntentRegistry intents;

        IntentId missingOwner = static_cast<IntentId>(99) << INTENT_OWNER_SHIFT;
        assert(!intents.exists(missingOwner));
        assert(intents.owner_of(missingOwner) == 99);
        assert(intents.size(99) == 0);
    }

    {
        IntentRegistry intents;
        BehaviorId owner = 11;
        intents.addBehaviour(owner);
        std::set<IntentId> created;

        for (std::size_t i = 0; i < MAX_INTENTS; ++i) {
            IntentId id = intents.create(owner);
            assert(created.insert(id).second);
            assert(intents.exists(id));
            assert(intents.owner_of(id) == owner);
        }

        assert(intents.size(owner) == MAX_INTENTS);

        expect_throw([&] {
            intents.create(owner);
        });

        IntentId recycled = *created.begin();
        intents.destroy(recycled);
        assert(intents.size(owner) == MAX_INTENTS - 1);
        assert(!intents.exists(recycled));
        assert(intents.create(owner) == recycled);
        assert(intents.size(owner) == MAX_INTENTS);
    }

    return 0;
}
