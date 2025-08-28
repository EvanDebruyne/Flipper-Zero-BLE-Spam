#pragma once

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct Protocol Protocol;

// Base protocol interface
typedef void (*ProtocolMakePacket)(uint8_t* size, uint8_t** packet, void* payload);
typedef const char* (*ProtocolGetName)(void* payload);

struct Protocol {
    const char* name;
    ProtocolMakePacket make_packet;
    ProtocolGetName get_name;
};

// Common payload modes
typedef enum {
    PayloadModeRandom,
    PayloadModeValue,
    PayloadModeBruteforce,
} PayloadMode;

// Bruteforce structure
typedef struct {
    uint8_t counter;
    uint32_t value;
    uint8_t size;
} Bruteforce;
