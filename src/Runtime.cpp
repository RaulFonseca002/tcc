#include "liquid/Runtime.hpp"

#include "liquid/IntentExpiration.hpp"

World& Runtime::world() {
    return ownedWorld;
}

const World& Runtime::world() const {
    return ownedWorld;
}

FrameNumber Runtime::frame() const {
    return currentFrame;
}

FrameLog Runtime::run_frame(
    IntentTime now,
    std::map<ComponentTypeId, std::map<ComponentName, ComponentSlotId>> resolutions
) {
    FrameLog log;
    log.frame = currentFrame;
    log.now = now;

    log.phases.push_back("begin_frame");

    log.phases.push_back("expire_intents");
    log.expired_intents = destroy_expired_intents(ownedWorld, now);

    log.phases.push_back("resolve_intents");
    log.resolution_requests = resolutions.size();

    for (const auto& [type, components] : resolutions) {
        std::map<ComponentName, IntentId> selected = ownedWorld.resolve_intents(type, components, now);
        log.selected_intents += selected.size();
        log.intent_selections.emplace(type, std::move(selected));
    }

    log.phases.push_back("run_systems");
    log.systems_run = ownedWorld.run_systems(currentFrame, now);

    log.phases.push_back("end_frame");

    latestFrameLog = log;
    ++currentFrame;

    return latestFrameLog;
}

const FrameLog& Runtime::last_frame_log() const {
    return latestFrameLog;
}
