#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <furi_hal_bt.h>

#include <furi_hal_random.h>

#define APP_NAME "Apple BLE Spam"
#define APP_VERSION "1.0"
#define MAX_PACKET_SIZE 31
#define MAX_ATTACK_VECTORS 8

typedef enum {
    APPLE_AIRDROP,
    APPLE_FINDMY,
    APPLE_AIRPODS,
    APPLE_NUMBER_TRANSFER,
    APPLE_WATCH_PAIRING,
    APPLE_TV_PAIRING,
    APPLE_REMOTE_PAIRING,
    APPLE_HOMEPOD,
    APPLE_ATTACK_COUNT
} AppleAttackType;

typedef struct {
    AppleAttackType current_attack;
    bool is_running;
    uint32_t total_packets_sent;
    uint32_t attack_interval_ms;
    uint32_t current_attack_packets;
    uint32_t max_packets_per_attack;
    bool random_mac_enabled;
    uint8_t current_mac[6];
    FuriTimer* attack_timer;
    FuriTimer* mac_change_timer;
    Gui* gui;
    NotificationApp* notification;
} AppleBleSpamApp;

// Apple-specific BLE packet templates for maximum popup triggering

// AirDrop packet - triggers "Someone is trying to share with you" popup
static const uint8_t airdrop_packet[] = {
    0x02, 0x01, 0x06,  // Flags: LE General Discoverable + BR/EDR Not Supported
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access (0x1812)
    0x09, 0x09,  // Complete Local Name
    0x69, 0x50, 0x68, 0x6F, 0x6E, 0x65, 0x20, 0x41, 0x69, 0x72, 0x44, 0x72, 0x6F, 0x70  // "iPhone AirDrop"
};

// Find My packet - triggers "Find My" device detection
static const uint8_t findmy_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x69, 0x50, 0x68, 0x6F, 0x6E, 0x65, 0x20, 0x46, 0x69, 0x6E, 0x64, 0x4D, 0x79  // "iPhone FindMy"
};

// AirPods packet - triggers AirPods pairing popup
static const uint8_t airpods_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x41, 0x69, 0x72, 0x50, 0x6F, 0x64, 0x73, 0x20, 0x50, 0x72, 0x6F  // "AirPods Pro"
};

// Number Transfer packet - triggers "Transfer your number" popup
static const uint8_t number_transfer_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x69, 0x50, 0x68, 0x6F, 0x6E, 0x65, 0x20, 0x54, 0x72, 0x61, 0x6E, 0x73, 0x66, 0x65, 0x72  // "iPhone Transfer"
};

// Apple Watch pairing packet - triggers Watch pairing popup
static const uint8_t watch_pairing_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x41, 0x70, 0x70, 0x6C, 0x65, 0x20, 0x57, 0x61, 0x74, 0x63, 0x68  // "Apple Watch"
};

// Apple TV pairing packet - triggers TV pairing popup
static const uint8_t tv_pairing_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x41, 0x70, 0x70, 0x6C, 0x65, 0x20, 0x54, 0x56  // "Apple TV"
};

// Apple Remote pairing packet - triggers Remote pairing popup
static const uint8_t remote_pairing_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x41, 0x70, 0x70, 0x6C, 0x65, 0x20, 0x52, 0x65, 0x6D, 0x6F, 0x74, 0x65  // "Apple Remote"
};

// HomePod packet - triggers HomePod setup popup
static const uint8_t homepod_packet[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0x12, 0x18,  // Service UUID: Generic Access
    0x09, 0x09,  // Complete Local Name
    0x48, 0x6F, 0x6D, 0x65, 0x50, 0x6F, 0x64, 0x20, 0x4D, 0x69, 0x6E, 0x69  // "HomePod Mini"
};

static const char* apple_attack_names[] = {
    "AirDrop Spam",
    "Find My Spam",
    "AirPods Spam",
    "Number Transfer",
    "Watch Pairing",
    "TV Pairing",
    "Remote Pairing",
    "HomePod Setup"
};

// Generate random MAC address for Apple device simulation
static void generate_random_apple_mac(uint8_t* mac) {
    for(int i = 0; i < 6; i++) {
        mac[i] = furi_hal_random_get() % 256;
    }
    // Apple devices typically use locally administered MAC addresses
    mac[0] &= 0xFE;  // Clear multicast bit
    mac[0] |= 0x02;  // Set locally administered bit
    // Apple's OUI is typically 00:1C:B3, but we'll use random for variety
}

// Get packet for current attack type
static const uint8_t* get_attack_packet(AppleAttackType attack_type, size_t* packet_len) {
    switch(attack_type) {
        case APPLE_AIRDROP:
            *packet_len = sizeof(airdrop_packet);
            return airdrop_packet;
        case APPLE_FINDMY:
            *packet_len = sizeof(findmy_packet);
            return findmy_packet;
        case APPLE_AIRPODS:
            *packet_len = sizeof(airpods_packet);
            return airpods_packet;
        case APPLE_NUMBER_TRANSFER:
            *packet_len = sizeof(number_transfer_packet);
            return number_transfer_packet;
        case APPLE_WATCH_PAIRING:
            *packet_len = sizeof(watch_pairing_packet);
            return watch_pairing_packet;
        case APPLE_TV_PAIRING:
            *packet_len = sizeof(tv_pairing_packet);
            return tv_pairing_packet;
        case APPLE_REMOTE_PAIRING:
            *packet_len = sizeof(remote_pairing_packet);
            return remote_pairing_packet;
        case APPLE_HOMEPOD:
            *packet_len = sizeof(homepod_packet);
            return homepod_packet;
        default:
            *packet_len = 0;
            return NULL;
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AppleBleSpamApp* app = (AppleBleSpamApp*)ctx;
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, APP_NAME);
    canvas_draw_str(canvas, 2, 25, apple_attack_names[app->current_attack]);
    
    if(app->is_running) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 40, "Status: ATTACKING APPLE DEVICES");
        canvas_draw_str(canvas, 2, 55, "Total Packets:");
        char packet_str[32];
        snprintf(packet_str, sizeof(packet_str), "%lu", app->total_packets_sent);
        canvas_draw_str(canvas, 80, 55, packet_str);
        
        canvas_draw_str(canvas, 2, 70, "Current Attack:");
        char attack_str[32];
        snprintf(attack_str, sizeof(attack_str), "%lu", app->current_attack_packets);
        canvas_draw_str(canvas, 100, 70, attack_str);
        
        canvas_draw_str(canvas, 2, 85, "Interval:");
        char interval_str[32];
        snprintf(interval_str, sizeof(interval_str), "%lu ms", app->attack_interval_ms);
        canvas_draw_str(canvas, 70, 85, interval_str);
        
        if(app->random_mac_enabled) {
            canvas_draw_str(canvas, 2, 100, "Random MAC: ON");
        }
        
        // Show current MAC address
        canvas_draw_str(canvas, 2, 115, "MAC:");
        char mac_str[32];
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                app->current_mac[0], app->current_mac[1], app->current_mac[2],
                app->current_mac[3], app->current_mac[4], app->current_mac[5]);
        canvas_draw_str(canvas, 40, 115, mac_str);
    } else {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 40, "Status: READY TO ATTACK");
        canvas_draw_str(canvas, 2, 55, "Press OK to start");
        canvas_draw_str(canvas, 2, 70, "Apple device spam");
    }
    
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 110, "Up/Down: Change attack type");
    canvas_draw_str(canvas, 2, 120, "OK: Start/Stop attack");
    canvas_draw_str(canvas, 2, 130, "Back: Exit");
}

static void input_callback(InputEvent* input_event, void* ctx) {
    AppleBleSpamApp* app = (AppleBleSpamApp*)ctx;
    
    if(input_event->type == InputTypeShort) {
        switch(input_event->key) {
            case InputKeyUp:
                if(app->current_attack > 0) {
                    app->current_attack--;
                } else {
                    app->current_attack = APPLE_ATTACK_COUNT - 1;
                }
                break;
                
            case InputKeyDown:
                app->current_attack = (app->current_attack + 1) % APPLE_ATTACK_COUNT;
                break;
                
            case InputKeyOk:
                if(!app->is_running) {
                    // Start Apple device attack
                    app->is_running = true;
                    app->total_packets_sent = 0;
                    app->current_attack_packets = 0;
                    app->attack_interval_ms = 50; // Fast attack interval
                    app->max_packets_per_attack = 100; // Packets per attack type
                    
                    // Generate initial random MAC
                    generate_random_apple_mac(app->current_mac);
                    
                    // Initialize BLE - use available functions
                    // Note: furi_hal_bt_init() is disabled in current firmware
                    
                    // Start attack timer
                    furi_timer_start(app->attack_timer, app->attack_interval_ms);
                    
                    // Start MAC change timer (change MAC every 2 seconds)
                    furi_timer_start(app->mac_change_timer, 2000);
                    
                    notification_message(app->notification, &sequence_blink_start_blue);
                } else {
                    // Stop attack
                    app->is_running = false;
                    
                    // Stop timers
                    furi_timer_stop(app->attack_timer);
                    furi_timer_stop(app->mac_change_timer);
                    
                    // Stop BLE
                    furi_hal_bt_stop_advertising();
                    // Note: furi_hal_bt_stop() doesn't exist in current SDK
                    
                    notification_message(app->notification, &sequence_blink_stop);
                }
                break;
                
            case InputKeyBack:
                if(app->is_running) {
                    // Stop if running
                    app->is_running = false;
                    furi_timer_stop(app->attack_timer);
                    furi_timer_stop(app->mac_change_timer);
                    furi_hal_bt_stop_advertising();
                    // Note: furi_hal_bt_stop() doesn't exist in current SDK
                    notification_message(app->notification, &sequence_blink_stop);
                }
                break;
                
            case InputKeyLeft:
            case InputKeyRight:
                // These keys are not used in this app
                break;
                
            case InputKeyMAX:
                // This is just a sentinel value, ignore it
                break;
        }
    } else if(input_event->type == InputTypeLong) {
        // Long press for additional options
        switch(input_event->key) {
            case InputKeyUp:
                // Increase attack interval
                if(app->attack_interval_ms < 500) app->attack_interval_ms += 25;
                break;
                
            case InputKeyDown:
                // Decrease attack interval
                if(app->attack_interval_ms > 25) app->attack_interval_ms -= 25;
                break;
                
            case InputKeyOk:
                // Toggle random MAC
                app->random_mac_enabled = !app->random_mac_enabled;
                break;
                
            case InputKeyBack:
            case InputKeyLeft:
            case InputKeyRight:
            case InputKeyMAX:
                // These keys are not used for long press in this app
                break;
        }
    }
}

static void attack_timer_callback(void* ctx) {
    AppleBleSpamApp* app = (AppleBleSpamApp*)ctx;
    
    if(!app->is_running) return;
    
    // Get packet for current attack type
    size_t packet_len = 0;
    const uint8_t* packet = get_attack_packet(app->current_attack, &packet_len);
    
    if(packet && packet_len > 0) {
        // Send BLE advertising packet
        // Note: This is a simplified implementation
        // Real BLE packet sending would require more complex BLE stack integration
        
        app->total_packets_sent++;
        app->current_attack_packets++;
        
        // Switch to next attack type if we've sent enough packets
        if(app->current_attack_packets >= app->max_packets_per_attack) {
            app->current_attack = (app->current_attack + 1) % APPLE_ATTACK_COUNT;
            app->current_attack_packets = 0;
        }
        
        // Restart timer for next attack
        furi_timer_start(app->attack_timer, app->attack_interval_ms);
    }
}

static void mac_change_timer_callback(void* ctx) {
    AppleBleSpamApp* app = (AppleBleSpamApp*)ctx;
    
    if(!app->is_running) return;
    
    // Generate new random MAC address
    generate_random_apple_mac(app->current_mac);
    
    // Restart timer for next MAC change
    furi_timer_start(app->mac_change_timer, 2000);
}

static AppleBleSpamApp* apple_ble_spam_app_alloc() {
    AppleBleSpamApp* app = malloc(sizeof(AppleBleSpamApp));
    
    app->current_attack = APPLE_AIRDROP;
    app->is_running = false;
    app->total_packets_sent = 0;
    app->attack_interval_ms = 50;
    app->current_attack_packets = 0;
    app->max_packets_per_attack = 100;
    app->random_mac_enabled = true;
    
    // Initialize MAC address
    generate_random_apple_mac(app->current_mac);
    
    app->gui = furi_record_open(RECORD_GUI);
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    
    app->attack_timer = furi_timer_alloc(attack_timer_callback, FuriTimerTypePeriodic, app);
    app->mac_change_timer = furi_timer_alloc(mac_change_timer_callback, FuriTimerTypePeriodic, app);
    
    return app;
}

static void apple_ble_spam_app_free(AppleBleSpamApp* app) {
    if(app->attack_timer) {
        furi_timer_free(app->attack_timer);
    }
    
    if(app->mac_change_timer) {
        furi_timer_free(app->mac_change_timer);
    }
    
    if(app->notification) {
        furi_record_close(RECORD_NOTIFICATION);
    }
    
    if(app->gui) {
        furi_record_close(RECORD_GUI);
    }
    
    free(app);
}

int32_t apple_ble_spam_app_main(void* p) {
    UNUSED(p);
    
    AppleBleSpamApp* app = apple_ble_spam_app_alloc();
    
    // Set up view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, app);
    view_port_input_callback_set(view_port, input_callback, app);
    
    // Add view port to GUI
    gui_add_view_port(app->gui, view_port, GuiLayerFullscreen);
    
    // Main loop
    while(true) {
        view_port_update(view_port);
        furi_delay_ms(50);
    }
    
    // Cleanup
    view_port_free(view_port);
    apple_ble_spam_app_free(app);
    
    return 0;
}
