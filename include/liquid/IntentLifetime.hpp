#pragma once

#include <cstdint>

namespace liquid {

using IntentTime = std::uint64_t;

enum class IntentLifetimeKind {
    Persistent,
    UntilTime
};

struct IntentLifetime {
    IntentLifetime() = delete;

    IntentLifetimeKind kind;
    IntentTime expiresAt;

    IntentLifetime(IntentLifetimeKind kind, IntentTime expiresAt)
        : kind(kind),
          expiresAt(expiresAt)
    {
    }

    static IntentLifetime persistent() {
        return {IntentLifetimeKind::Persistent, 0};
    }

    static IntentLifetime until_time(IntentTime time) {
        return {IntentLifetimeKind::UntilTime, time};
    }
};

}

using liquid::IntentLifetime;
using liquid::IntentLifetimeKind;
using liquid::IntentTime;
