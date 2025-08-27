#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ContinuityTypeAirDrop,
    ContinuityTypeAirplayTarget,
    ContinuityTypeHandoff,
    ContinuityTypeTetheringSource,
    ContinuityTypeNearbyAction,
    ContinuityTypeNearbyInfo,
    ContinuityTypeProximityPair,
    ContinuityTypeCustomCrash,
    ContinuityTypeCount,
} ContinuityType;

typedef struct {
    uint8_t flags;
    uint8_t type;
} NearbyAction;

typedef struct {
    uint8_t prefix;
    uint16_t model;
} ProximityPair;

typedef struct {
    // Empty for now, can be extended
} AirDrop;

typedef struct {
    // Empty for now, can be extended
} AirplayTarget;

typedef struct {
    // Empty for now, can be extended
} Handoff;

typedef struct {
    // Empty for now, can be extended
} TetheringSource;

typedef struct {
    // Empty for now, can be extended
} NearbyInfo;

typedef struct {
    // Empty for now, can be extended
} CustomCrash;

typedef union {
    NearbyAction nearby_action;
    ProximityPair proximity_pair;
    AirDrop airdrop;
    AirplayTarget airplay_target;
    Handoff handoff;
    TetheringSource tethering_source;
    NearbyInfo nearby_info;
    CustomCrash custom_crash;
} ContinuityData;

typedef struct {
    ContinuityType type;
    ContinuityData data;
} ContinuityMsg;

const char* continuity_get_type_name(ContinuityType type);
uint8_t continuity_get_packet_size(ContinuityType type);
void continuity_generate_packet(const ContinuityMsg* msg, uint8_t* packet);
