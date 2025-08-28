#pragma once

#include "_base.h"

// Apple Continuity protocol definitions
typedef enum {
    ContinuityTypeAirDrop = 0x05,
    ContinuityTypeProximityPair = 0x07,
    ContinuityTypeAirplayTarget = 0x09,
    ContinuityTypeHandoff = 0x0C,
    ContinuityTypeTetheringSource = 0x0E,
    ContinuityTypeNearbyAction = 0x0F,
    ContinuityTypeNearbyInfo = 0x10,
    ContinuityTypeCustomCrash,
    ContinuityTypeCount
} ContinuityType;

typedef struct {
    ContinuityType type;
    union {
        struct {
            uint16_t model;
            uint8_t color;
            uint8_t prefix;
        } proximity_pair;
        struct {
            uint8_t action;
            uint8_t flags;
        } nearby_action;
    } data;
} ContinuityCfg;

// Payload structure
typedef struct {
    bool random_mac;
    PayloadMode mode;
    Bruteforce bruteforce;
    ContinuityCfg continuity;
} Payload;

// Attack structure
typedef struct {
    const char* title;
    const char* text;
    Payload payload;
} Attack;

// Function declarations
const char* continuity_get_type_name(ContinuityType type);

// Protocol declaration
extern const Protocol protocol_continuity;
