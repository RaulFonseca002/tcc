#pragma once

#include "liquid/IntentRegistry.hpp"
#include "liquid/IntentLifetime.hpp"

namespace liquid {

enum class IntentStatus {
    Alive,
    Expired
};

enum class IntentExpirationReason {
    None,
    TimeReached
};

struct IntentExpiration {
    IntentStatus status = IntentStatus::Alive;
    IntentExpirationReason reason = IntentExpirationReason::None;
};

IntentExpiration expiration_for(const IntentLifetime& lifetime, IntentTime now);
IntentExpiration expiration_for(const Intent& intent, IntentTime now);

}

using liquid::IntentExpiration;
using liquid::IntentExpirationReason;
using liquid::IntentStatus;
using liquid::expiration_for;
