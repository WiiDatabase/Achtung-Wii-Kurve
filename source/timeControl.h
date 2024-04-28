#include "ogc/lwp_watchdog.h"

extern u32 GFX_GetTime() {
    return ( ticks_to_millisecs(gettime()) );
}

extern u32 GFX_GetElapsedTime(const u32 last) {
    return ( ticks_to_millisecs(gettime()) - last );
}
