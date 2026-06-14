#include "liquid/ComponentStorage.hpp"

#include <cassert>
#include <cstddef>

struct Light {
    int brightness = 0;
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
        liquid::ComponentStorage<Light> storage;

        ComponentSlotId first = storage.add(Light{10});
        ComponentSlotId second = storage.add(Light{20});

        assert(first == 0);
        assert(second == 1);
        assert(storage.size() == 2);
        assert(storage.slot_count() == 2);
        assert(storage.has(first));
        assert(storage.has(second));
        assert(storage.get(first)->brightness == 10);
        assert(storage.get(second)->brightness == 20);

        const auto& constStorage = storage;
        assert(constStorage.get(first)->brightness == 10);
        assert(constStorage[first].brightness == 10);

        storage[first].brightness = 15;
        assert(storage.get(first)->brightness == 15);

        storage.remove(first);

        assert(!storage.has(first));
        assert(storage.has(second));
        assert(storage.get(first) == nullptr);
        assert(storage.size() == 1);
        assert(storage.slot_count() == 2);

        expect_throw([&] {
            storage[first];
        });

        expect_throw([&] {
            storage.remove(first);
        });

        ComponentSlotId reused = storage.add(Light{30});

        assert(reused == first);
        assert(storage.size() == 2);
        assert(storage.slot_count() == 2);
        assert(storage.get(reused)->brightness == 30);
    }

    {
        liquid::ComponentStorage<Light> storage;

        assert(!storage.has(42));
        assert(storage.get(42) == nullptr);

        expect_throw([&] {
            storage.remove(42);
        });

        expect_throw([&] {
            storage[42];
        });
    }

    {
        liquid::ComponentStorage<Light> storage;

        ComponentSlotId first = storage.add(Light{10});
        ComponentSlotId second = storage.add(Light{20});
        BehaviorId reader = 4;
        BehaviorId writer = 7;

        storage.addAccess(reader, liquid::ComponentSlotAccess::r, first);
        storage.addAccess(reader, liquid::ComponentSlotAccess::rw, second);
        storage.addAccess(writer, liquid::ComponentSlotAccess::w, first);

        assert(storage.allAccesses().size() == 2);
        assert(storage.accessesOf(reader).size() == 2);
        assert(storage.accessesOf(reader)[0].slot == first);
        assert(storage.accessesOf(reader)[0].mode == liquid::ComponentSlotAccess::r);
        assert(storage.accessesOf(reader)[1].slot == second);
        assert(storage.accessesOf(reader)[1].mode == liquid::ComponentSlotAccess::rw);
        assert(storage.accessesOf(writer).size() == 1);
        assert(storage.accessesOf(writer)[0].mode == liquid::ComponentSlotAccess::w);

        expect_throw([&] {
            storage.accessesOf(99);
        });

        storage.removeAccess(reader, first);

        assert(storage.accessesOf(reader).size() == 1);
        assert(storage.accessesOf(reader)[0].slot == second);
        assert(storage.accessesOf(reader)[0].mode == liquid::ComponentSlotAccess::rw);

        storage.removeAccess(reader, first);
        assert(storage.accessesOf(reader).size() == 1);

        storage.removeAccessesTo(first);

        assert(storage.accessesOf(reader).size() == 1);
        expect_throw([&] {
            storage.accessesOf(writer);
        });

        storage.removeAccessesOf(reader);
        assert(storage.allAccesses().empty());
    }

    {
        liquid::ComponentStorage<Light> storage;

        for (std::size_t i = 0; i < MAX_COMPONENT_SLOTS; ++i) {
            ComponentSlotId slot = storage.add(Light{static_cast<int>(i)});
            assert(slot == i);
            assert(storage.has(slot));
        }

        assert(storage.size() == MAX_COMPONENT_SLOTS);
        assert(storage.slot_count() == MAX_COMPONENT_SLOTS);

        expect_throw([&] {
            storage.add(Light{1});
        });
    }

    return 0;
}
