#include "liquid/IntentExpiration.hpp"

namespace liquid {

IntentExpiration expiration_for(const IntentLifetime& lifetime, IntentTime now) {
    if (lifetime.kind == IntentLifetimeKind::UntilTime && now >= lifetime.expiresAt)
        return {IntentStatus::Expired, IntentExpirationReason::TimeReached};

    return {};
}

IntentExpiration expiration_for(const Intent& intent, IntentTime now) {
    return expiration_for(intent.lifetime, now);
}

}
