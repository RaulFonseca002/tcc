#pragma once

#include <cstdint>

namespace liquid {

using IntentTime = std::uint64_t;

enum class IntentLifetimeKind {
    Persistent,
    UntilTime,
    UntilCancelled
};

struct IntentLifetime {
    IntentLifetimeKind kind = IntentLifetimeKind::Persistent;
    IntentTime expiresAt = 0;

    static IntentLifetime persistent() {
        return {};
    }

    static IntentLifetime until_time(IntentTime time) {
        return {IntentLifetimeKind::UntilTime, time};
    }

    static IntentLifetime until_cancelled() {
        return {IntentLifetimeKind::UntilCancelled, 0};
    }
};

}

using liquid::IntentLifetime;
using liquid::IntentLifetimeKind;
using liquid::IntentTime;
