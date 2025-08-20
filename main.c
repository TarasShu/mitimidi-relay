/******************************************************************************
 * @file main.c
 *
 * @brief MIDI Relay Controller for Pico W
 *        Controls 4 relays (GPIO 16-19) based on MIDI messages
 *        Supports both USB MIDI and Bluetooth MIDI input
 *
 * @author mitimidi-relay
 * @date 2025-08-07
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

// TinyUSB includes for MIDI
#include "tusb.h"
#include "class/midi/midi_device.h"

// Bluetooth includes - BLE MIDI support
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "ble_midi_server.h"
#include "btstack.h"
#include "midimiti.h"

// MIDI constants
#define MIDI_NOTE_OFF    0x80
#define MIDI_NOTE_ON     0x90
#define MIDI_CC          0xB0
#define MIDI_PROGRAM_CHANGE 0xC0

// Relay GPIO pins (same as breadboard-os)
#define RELAY_1_PIN      16
#define RELAY_2_PIN      17
#define RELAY_3_PIN      18
#define RELAY_4_PIN      19

// MIDI note mappings for relays
#define RELAY_1_NOTE     60  // C4
#define RELAY_2_NOTE     61  // C#4
#define RELAY_3_NOTE     62  // D4  
#define RELAY_4_NOTE     63  // D#4

// Global state
static bool relay_states[4] = {false, false, false, false};
static bool bluetooth_connected = false;

// Function prototypes
static void init_relays(void);
static void set_relay(int relay_num, bool state);
static void process_midi_message(uint8_t status, uint8_t data1, uint8_t data2, bool from_bluetooth);
static void setup_bluetooth_midi(void);
static void bt_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void print_relay_states(void);

// Initialize relay GPIO pins
static void init_relays(void)
{
    gpio_init(RELAY_1_PIN);
    gpio_init(RELAY_2_PIN);
    gpio_init(RELAY_3_PIN);
    gpio_init(RELAY_4_PIN);
    
    gpio_set_dir(RELAY_1_PIN, GPIO_OUT);
    gpio_set_dir(RELAY_2_PIN, GPIO_OUT);
    gpio_set_dir(RELAY_3_PIN, GPIO_OUT);
    gpio_set_dir(RELAY_4_PIN, GPIO_OUT);
    
    // Start with all relays off
    gpio_put(RELAY_1_PIN, false);
    gpio_put(RELAY_2_PIN, false);
    gpio_put(RELAY_3_PIN, false);
    gpio_put(RELAY_4_PIN, false);
    
    printf("Relays initialized on pins 16-19\r\n");
}

// Set relay state
static void set_relay(int relay_num, bool state)
{
    if (relay_num < 1 || relay_num > 4) return;
    
    relay_states[relay_num - 1] = state;
    
    switch(relay_num) {
        case 1: gpio_put(RELAY_1_PIN, state); break;
        case 2: gpio_put(RELAY_2_PIN, state); break;
        case 3: gpio_put(RELAY_3_PIN, state); break;
        case 4: gpio_put(RELAY_4_PIN, state); break;
    }
    
    printf("Relay %d: %s\r\n", relay_num, state ? "ON" : "OFF");
    print_relay_states();
}

// Process MIDI message and control relays
static void process_midi_message(uint8_t status, uint8_t data1, uint8_t data2, bool from_bluetooth)
{
    const char *source = from_bluetooth ? "BT" : "USB";
    uint8_t msg_type = status & 0xF0;
    uint8_t channel = status & 0x0F;
    
    switch (msg_type) {
        case MIDI_NOTE_ON:
            if (data2 > 0) {  // Velocity > 0 means note on
                printf("[%s] Note On: Ch%d Note%d Vel%d\r\n", source, channel + 1, data1, data2);
                
                // Map MIDI notes to relays
                switch (data1) {
                    case RELAY_1_NOTE: set_relay(1, true); break;
                    case RELAY_2_NOTE: set_relay(2, true); break;
                    case RELAY_3_NOTE: set_relay(3, true); break;
                    case RELAY_4_NOTE: set_relay(4, true); break;
                    default:
                        printf("Note %d not mapped to relay\r\n", data1);
                        break;
                }
            } else {
                // Velocity 0 = note off
                printf("[%s] Note Off: Ch%d Note%d\r\n", source, channel + 1, data1);
                switch (data1) {
                    case RELAY_1_NOTE: set_relay(1, false); break;
                    case RELAY_2_NOTE: set_relay(2, false); break;
                    case RELAY_3_NOTE: set_relay(3, false); break;
                    case RELAY_4_NOTE: set_relay(4, false); break;
                }
            }
            break;
            
        case MIDI_NOTE_OFF:
            printf("[%s] Note Off: Ch%d Note%d Vel%d\r\n", source, channel + 1, data1, data2);
            switch (data1) {
                case RELAY_1_NOTE: set_relay(1, false); break;
                case RELAY_2_NOTE: set_relay(2, false); break;
                case RELAY_3_NOTE: set_relay(3, false); break;
                case RELAY_4_NOTE: set_relay(4, false); break;
            }
            break;
            
        case MIDI_CC:
            printf("[%s] CC: Ch%d CC%d Val%d\r\n", source, channel + 1, data1, data2);
            // Use CC 1-4 to control relays
            if (data1 >= 1 && data1 <= 4) {
                set_relay(data1, data2 >= 64);  // On if CC value >= 64
            }
            break;
            
        case MIDI_PROGRAM_CHANGE:
            printf("[%s] Program: Ch%d Prog%d\r\n", source, channel + 1, data1);
            // Use program change 0-3 to turn on specific relay, others turn all off
            if (data1 >= 0 && data1 <= 3) {
                // Turn all off first
                set_relay(1, false);
                set_relay(2, false);  
                set_relay(3, false);
                set_relay(4, false);
                // Turn on selected relay
                set_relay(data1 + 1, true);
            } else {
                // Turn all off for other program numbers
                set_relay(1, false);
                set_relay(2, false);
                set_relay(3, false);
                set_relay(4, false);
            }
            break;
            
        default:
            printf("[%s] Unknown MIDI: 0x%02X 0x%02X 0x%02X\r\n", source, status, data1, data2);
            break;
    }
}

// Print current relay states
static void print_relay_states(void)
{
    printf("Relay States: [1:%s] [2:%s] [3:%s] [4:%s]\r\n",
           relay_states[0] ? "ON " : "OFF",
           relay_states[1] ? "ON " : "OFF", 
           relay_states[2] ? "ON " : "OFF",
           relay_states[3] ? "ON " : "OFF");
}

// BLE MIDI advertisement data - MidiMiti
const uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Service class list - MIDI Service UUID
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS, 
    0x00, 0xc7, 0xc4, 0x4e, 0xe3, 0x6c, 0x51, 0xa7, 
    0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03,
};

const uint8_t scan_resp_data[] = {
    // Complete local name "MidiMiti"
    0x09, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'M', 'i', 'd', 'i', 'M', 'i', 't', 'i'
};

// Setup Bluetooth MIDI
static void setup_bluetooth_midi(void)
{
    printf("Setting up BLE MIDI as 'MidiMiti'...\r\n");
    
    // Initialize BLE MIDI server with MidiMiti profile
    ble_midi_server_init(profile_data, scan_resp_data, sizeof(scan_resp_data),
        IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
        SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);
    
    printf("BLE MIDI server initialized as 'MidiMiti'\r\n");
    printf("Device should be discoverable in Bluetooth MIDI settings\r\n");
    
    bluetooth_connected = false;
}

// Bluetooth packet handler (simplified)
static void bt_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    // Simplified packet handler for basic functionality
    (void)packet_type;
    (void)channel;
    (void)packet;
    (void)size;
    
    // Full BLE MIDI would handle GATT notifications here
}

// TinyUSB device descriptor
uint8_t const desc_device[] =
{
    18,                         // bLength
    TUSB_DESC_DEVICE,          // bDescriptorType
    0x00, 0x02,                // bcdUSB 2.00
    0x00,                      // bDeviceClass (Composite)
    0x00,                      // bDeviceSubClass
    0x00,                      // bDeviceProtocol
    CFG_TUD_ENDPOINT0_SIZE,    // bMaxPacketSize0
    0xA0, 0xCA,                // idVendor (0xCAFE)
    0x01, 0x42,                // idProduct (0x4201)
    0x00, 0x01,                // bcdDevice 1.00
    0x01,                      // iManufacturer
    0x02,                      // iProduct
    0x03,                      // iSerialNumber
    0x01                       // bNumConfigurations
};

uint8_t const * tud_descriptor_device_cb(void)
{
    return desc_device;
}

// Configuration descriptor
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)

uint8_t const desc_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    
    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(0, 0, 0x01, 0x81, 64),
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;
    return desc_configuration;
}

// String descriptors
char const* string_desc_arr [] =
{
    (const char[]) { 0x09, 0x04 }, // 0: Language (English)
    "MidiMiti",                    // 1: Manufacturer
    "MidiMiti",                    // 2: Product
    "123456",                      // 3: Serials
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (!(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];
        chr_count = (uint8_t) strlen(str);
        if (chr_count > 31) chr_count = 31;

        for(uint8_t i=0; i<chr_count; i++) {
            _desc_str[1+i] = str[i];
        }
    }

    _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

    return _desc_str;
}

// Main function
int main(void)
{
    // Initialize standard library
    stdio_init_all();
    
    printf("\r\n=== MIDI Relay Controller ===\r\n");
    printf("Controls 4 relays via MIDI messages\r\n");
    printf("USB & Bluetooth MIDI supported\r\n\r\n");
    
    // Initialize hardware
    init_relays();
    
    // Initialize CYW43 for WiFi/Bluetooth
    if (cyw43_arch_init()) {
        printf("Failed to initialize cyw43\r\n");
        return -1;
    }
    
    // Initialize TinyUSB
    tud_init(0);
    
    // Initialize Bluetooth MIDI
    setup_bluetooth_midi();
    
    printf("\r\nMIDI Mapping:\r\n");
    printf("Notes: C4(60)=Relay1, C#4(61)=Relay2, D4(62)=Relay3, D#4(63)=Relay4\r\n");
    printf("CC: CC1-4 control Relay1-4 (>=64=ON, <64=OFF)\r\n");
    printf("Program: 0-3 select single relay, others=all off\r\n\r\n");
    
    print_relay_states();
    
    uint8_t packet[4];
    
    // Main loop
    while (1) {
        // Handle TinyUSB tasks
        tud_task();
        
        // Check for USB MIDI messages
        if (tud_midi_mounted()) {
            if (tud_midi_packet_read(packet)) {
                // Process USB MIDI message
                process_midi_message(packet[1], packet[2], packet[3], false);
            }
        }
        
        // Check for BLE MIDI messages
        if (ble_midi_server_is_connected()) {
            uint16_t timestamp;
            uint8_t ble_packet[3];
            uint8_t nread = ble_midi_server_stream_read(sizeof(ble_packet), ble_packet, &timestamp);
            if (nread > 0) {
                // Process BLE MIDI message
                if (nread >= 1) {
                    uint8_t data1 = nread >= 2 ? ble_packet[1] : 0;
                    uint8_t data2 = nread >= 3 ? ble_packet[2] : 0;
                    process_midi_message(ble_packet[0], data1, data2, true);
                }
            }
        }
        
        // Handle CYW43 WiFi/Bluetooth
        cyw43_arch_poll();
        
        // Small delay to prevent tight loop
        sleep_ms(1);
    }
    
    return 0;
}