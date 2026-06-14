#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

namespace liquid {

using BehaviorId = std::uint16_t;
using IntentId = std::uint32_t;
using ComponentName = std::string;
using Slot = std::uint16_t;
using ComponentTypeId = std::uint16_t;
using ComponentSlotId = Slot;
using TypeName = std::string;

inline constexpr ComponentTypeId InvalidComponentTypeId =
    std::numeric_limits<ComponentTypeId>::max();
inline constexpr ComponentSlotId InvalidComponentSlotId =
    std::numeric_limits<ComponentSlotId>::max();

struct ComponentTarget {
    ComponentTypeId type = InvalidComponentTypeId;
    ComponentSlotId slot = InvalidComponentSlotId;

    bool operator<(const ComponentTarget& other) const {
        if (type != other.type)
            return type < other.type;

        return slot < other.slot;
    }

    bool operator==(const ComponentTarget& other) const {
        return type == other.type && slot == other.slot;
    }
};

// Typed runtime handle for a registered component type.
// The explicit TypeName creates the ComponentTypeId during registration; after
// that, outside code passes this handle back instead of depending on typeid or
// hidden global template state. The id is world-local and recyclable later.
template <typename Component>
struct ComponentType {
    ComponentTypeId id = InvalidComponentTypeId;

    operator ComponentTypeId() const {
        return id;
    }
};

enum class ComponentAccessMode {
    Read,
    Write,
    ReadWrite
};

inline constexpr std::size_t MaxBehaviours = 5000;
inline constexpr std::size_t MaxIntents = 5000;
inline constexpr std::size_t MaxComponentTypes = 64;
inline constexpr std::size_t MaxComponentSlots =
    static_cast<std::size_t>(std::numeric_limits<Slot>::max());

inline constexpr std::uint32_t Low16Bits = 0x0000FFFF;
inline constexpr std::uint32_t IntentOwnerShift = 16;

using Signature = std::bitset<MaxComponentTypes>;

}

using liquid::BehaviorId;
using liquid::ComponentAccessMode;
using liquid::ComponentName;
using liquid::ComponentSlotId;
using liquid::ComponentTarget;
using liquid::ComponentType;
using liquid::ComponentTypeId;
using liquid::InvalidComponentSlotId;
using liquid::InvalidComponentTypeId;
using liquid::IntentId;
using liquid::Slot;
using liquid::Signature;
using liquid::TypeName;

using liquid::Low16Bits;
using liquid::MaxBehaviours;
using liquid::MaxComponentSlots;
using liquid::MaxComponentTypes;
using liquid::MaxIntents;
using liquid::IntentOwnerShift;

inline constexpr std::size_t MAX_BEHAVIOURS = liquid::MaxBehaviours;
inline constexpr std::size_t MAX_INTENTS = liquid::MaxIntents;
inline constexpr std::size_t MAX_COMPONENT_TYPES = liquid::MaxComponentTypes;
inline constexpr std::size_t MAX_COMPONENT_SLOTS = liquid::MaxComponentSlots;
inline constexpr std::uint32_t LOW_16_BITS = liquid::Low16Bits;
inline constexpr std::uint32_t INTENT_OWNER_SHIFT = liquid::IntentOwnerShift;
