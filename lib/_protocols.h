#pragma once

#include "_base.h"
#include "continuity.h"

// Protocol function declarations
extern const Protocol* protocols[];
extern const size_t protocols_count;

// Protocol registration
void register_protocols(void);
