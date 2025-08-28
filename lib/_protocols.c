#include "_protocols.h"

// Apple Continuity protocol (external declaration)
extern const Protocol protocol_continuity;

// Protocol array - only Apple continuity for Apple-focused spam
const Protocol* protocols[] = {
    &protocol_continuity,
};

const size_t protocols_count = COUNT_OF(protocols);

// Protocol registration function
void register_protocols(void) {
    // Protocols are automatically registered through the array
    // This function exists for future extensibility
}
