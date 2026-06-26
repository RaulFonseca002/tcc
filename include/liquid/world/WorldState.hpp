#pragma once

#include "liquid/BehaviorRegistry.hpp"
#include "liquid/ComponentRegistry.hpp"
#include "liquid/IntentRegistry.hpp"
#include "liquid/SystemRegistry.hpp"

#include <map>

struct WorldState {
    ComponentRegistry components;
    BehaviorRegistry behaviors;
    IntentRegistry intents;
    SystemRegistry systems;
    std::map<BehaviorId, Signature> behaviorSignatures;
};
