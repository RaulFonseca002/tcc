#pragma once

#include "liquid/Ids.hpp"
#include "liquid/IntentLifetime.hpp"
#include "liquid/world/World.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

struct FrameLog {
    FrameNumber frame = 0;
    IntentTime now = 0;
    std::vector<std::string> phases;
    std::size_t expired_intents = 0;
    std::size_t resolution_requests = 0;
    std::size_t selected_intents = 0;
    std::size_t systems_run = 0;
    std::map<ComponentTypeId, std::map<ComponentName, IntentId>> intent_selections;
};

class Runtime {
private:
    World ownedWorld;
    FrameNumber currentFrame = 0;
    FrameLog latestFrameLog;

public:
    Runtime() = default;

    World& world();
    const World& world() const;
    FrameNumber frame() const;
    FrameLog run_frame(
        IntentTime now,
        std::map<ComponentTypeId, std::map<ComponentName, ComponentSlotId>> resolutions = {}
    );
    const FrameLog& last_frame_log() const;
};
