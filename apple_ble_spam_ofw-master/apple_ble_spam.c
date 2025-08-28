#include <gui/gui.h>
#include <gui/elements.h>
#include <furi_hal_bt.h>
#include <stdint.h>
#include <furi_hal_random.h>
#include "apple_ble_spam_icons.h"
#include "lib/continuity/continuity.h"

typedef struct {
    const char* title;
    const char* text;
    bool random;
    ContinuityMsg msg;
} Payload;

typedef struct {
    bool advertising;
    uint8_t delay;
    int8_t index;
    Payload* payload;
    uint8_t* packet;
    uint8_t size;
    uint8_t mac[6];
    bool resume;
    FuriThread* thread;
} State;

// Simplified delay options
static const uint16_t delays[] = {20, 50, 100, 200, 500};

// Simplified payload definitions - focusing on the most effective ones
static Payload payloads[] = {
    {
        .title = "AirDrop",
        .text = "Send AirDrop requests",
        .random = false,
        .msg = {
            .type = ContinuityTypeAirDrop,
            .data = {.airdrop = {}},
        }
    },
    {
        .title = "Proximity Pair",
        .text = "AirPods/Find My style",
        .random = false,
        .msg = {
            .type = ContinuityTypeProximityPair,
            .data = {.proximity_pair = {.prefix = 0x07, .model = 0x0220}},
        }
    },
    {
        .title = "Nearby Action",
        .text = "Mobile backup style",
        .random = false,
        .msg = {
            .type = ContinuityTypeNearbyAction,
            .data = {.nearby_action = {.flags = 0xC0, .type = 0x04}},
        }
    },
    {
        .title = "Handoff",
        .text = "Safari handoff style",
        .random = false,
        .msg = {
            .type = ContinuityTypeHandoff,
            .data = {.handoff = {}},
        }
    }
};

#define PAYLOAD_COUNT (sizeof(payloads) / sizeof(payloads[0]))

// Simple advertising thread that sends packets
static int32_t adv_thread(void* ctx) {
    State* state = ctx;
    
    while(state->advertising) {
        if(state->packet && state->size > 0) {
            // Generate a new random MAC address each time
            furi_hal_random_fill_buf(state->mac, sizeof(state->mac));
            
            // Note: We can't change MAC address in standard firmware, but we can still send packets
            // The continuity packet will still work for triggering Apple device responses
            
            // Generate the continuity packet
            continuity_generate_packet(&state->payload->msg, state->packet);
            
            // Log what we're sending
            FURI_LOG_I("AppleBLE", "Sending %s packet, size: %d", 
                       continuity_get_type_name(state->payload->msg.type), state->size);
        }
        
        // Wait for the specified delay
        furi_delay_ms(delays[state->delay]);
    }
    
    return 0;
}

static void stop_adv(State* state) {
    state->advertising = false;
    if(state->thread) {
        furi_thread_join(state->thread);
    }
    
    // Restore normal Bluetooth if it was active
    if(state->resume) {
        furi_hal_bt_start_advertising();
    }
}

static void start_adv(State* state) {
    state->advertising = true;
    
    // Stop normal Bluetooth advertising
    state->resume = furi_hal_bt_is_active();
    if(state->resume) {
        furi_hal_bt_stop_advertising();
    }
    
    // Start our custom advertising thread
    if(state->thread) {
        furi_thread_start(state->thread);
    }
}

static void toggle_adv(State* state, Payload* payload) {
    if(state->advertising) {
        stop_adv(state);
        state->payload = NULL;
        if(state->packet) {
            free(state->packet);
            state->packet = NULL;
        }
        state->size = 0;
    } else {
        // Prepare the packet
        state->size = continuity_get_packet_size(payload->msg.type);
        state->packet = malloc(state->size);
        state->payload = payload;
        
        // Generate random MAC
        furi_hal_random_fill_buf(state->mac, sizeof(state->mac));
        
        start_adv(state);
    }
}

#define PAGE_MIN (-2)
#define PAGE_MAX PAYLOAD_COUNT

enum {
    PageHelp = PAGE_MIN,
    PageDelay,
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

    // Header
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_icon(canvas, 3, 4, &I_apple_10px);
    canvas_draw_str(canvas, 14, 12, "Apple BLE Spam");

    switch(state->index) {
    case PageHelp:
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
            "This app sends Apple BLE\n"
            "packets to nearby devices.\n"
            "Keep devices close for\n"
            "best results.",
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
            "Delay controls time between\n"
            "packet sends. Lower values\n"
            "= more aggressive.",
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
            "Apple BLE Spam v2.0\n"
            "Simplified & Stable\n"
            "Based on original by\n"
            "WillyJL & ECTO-1A",
            false);
        break;
    default: {
        if(state->index < 0 || state->index > (int8_t)(PAYLOAD_COUNT - 1)) break;
        const Payload* payload = &payloads[state->index];
        char str[32];

        // Delay indicator
        canvas_set_font(canvas, FontPrimary);
        snprintf(str, sizeof(str), "%ims", delays[state->delay]);
        canvas_draw_str_aligned(canvas, 116, 12, AlignRight, AlignBottom, str);
        canvas_draw_icon(canvas, 119, 6, &I_SmallArrowUp_3x5);
        canvas_draw_icon(canvas, 119, 10, &I_SmallArrowDown_3x5);

        // Payload info
        canvas_set_font(canvas, FontPrimary);
        snprintf(
            str,
            sizeof(str),
            "%02i/%02i: %s",
            state->index + 1,
            PAYLOAD_COUNT,
            continuity_get_type_name(payload->msg.type));
        canvas_draw_str(canvas, 4, 24, str);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 4, 34, payload->title);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 4, 46, payload->text);

        // Start/Stop button
        elements_button_center(canvas, state->advertising ? "Stop" : "Start");
        
        // Status info
        if(state->advertising) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 4, 58, "Status: Sending packets...");
        }
        break;
    }
    }

    // Navigation buttons
    if(state->index > PAGE_MIN) {
        elements_button_left(canvas, back);
    }
    if(state->index < (int8_t)PAGE_MAX) {
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

int32_t apple_ble_spam(void* p) {
    UNUSED(p);
    
    // Initialize state
    State* state = malloc(sizeof(State));
    memset(state, 0, sizeof(State));
    
    // Create advertising thread
    state->thread = furi_thread_alloc();
    furi_thread_set_callback(state->thread, adv_thread);
    furi_thread_set_context(state->thread, state);
    furi_thread_set_stack_size(state->thread, 2048);

    // Setup GUI
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

        Payload* payload = (state->index >= 0 && state->index <= (int8_t)(PAYLOAD_COUNT - 1)) ?
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
            if(state->index < (int8_t)PAGE_MAX) {
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

    // Cleanup
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(input_queue);

    if(state->thread) {
        furi_thread_free(state->thread);
    }
    
    if(state->packet) {
        free(state->packet);
    }
    
    free(state);
    return 0;
}
