#include "liquid/ComponentRegistry.hpp"

#include <stdexcept>

ComponentTypeId ComponentRegistry::component_type(const TypeName& typeName) const {
    auto found = componentTypes.find(typeName);

    if (found == componentTypes.end())
        throw std::runtime_error("component type not registered");

    return found->second;
}

void ComponentRegistry::remove_behavior(BehaviorId behavior) {
    for (auto& [type, storage] : storages) {
        (void)type;
        storage->removeAccessesOf(behavior);
    }
}

const TypeName& ComponentRegistry::type_name(ComponentTypeId type) const {
    if (type >= typeNames.size())
        throw std::runtime_error("component type not registered");

    return typeNames[type];
}

ComponentSlotId ComponentRegistry::named_slot(ComponentTypeId type, const ComponentName& name) const {
    auto typeComponents = componentNames.find(type);

    if (typeComponents == componentNames.end())
        throw std::runtime_error("component type not registered");

    auto found = typeComponents->second.find(name);

    if (found == typeComponents->second.end())
        throw std::runtime_error("component name not registered");

    return found->second;
}

liquid::ComponentSlotAccess::Mode ComponentRegistry::storage_access_mode(ComponentAccessMode mode) {
    switch (mode) {
    case ComponentAccessMode::Read:
        return liquid::ComponentSlotAccess::r;
    case ComponentAccessMode::Write:
        return liquid::ComponentSlotAccess::w;
    case ComponentAccessMode::ReadWrite:
        return liquid::ComponentSlotAccess::rw;
    }

    throw std::runtime_error("unknown component access mode");
}
