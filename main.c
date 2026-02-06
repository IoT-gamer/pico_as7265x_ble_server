/**
 * main.c
 * Pico 2W + AS7265x Spectral Sensor + BLE
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include "hardware/i2c.h"
#include "pico/stdio_usb.h"

#include "as7265x.h"
#include "ble_server.h"

// --- Configuration ---
#define HEARTBEAT_PERIOD_MS 1000  // Poll sensor every 1 second

// I2C Pin Definitions (Matches your library example)
#define I2C_PORT i2c0
#define SDA_PIN  4
#define SCL_PIN  5

// --- Globals ---
// The sensor object must be global so ble_server.c can access it via 'extern'
as7265x_t sensor; 

// Timer for polling the sensor
static btstack_timer_source_t heartbeat;
static btstack_packet_callback_registration_t hci_event_callback_registration;

// --- Helper Functions ---

/**
 * @brief Polling loop. Reads sensor and pushes data to BLE.
 */
static void heartbeat_handler(btstack_timer_source_t *ts) {
    // Check if sensor has new data
    if (as7265x_is_data_ready(&sensor)) {
        
        // Read calibrated data (18 floats)
        float channels[18];
        as7265x_get_all_calibrated(&sensor, channels);

        // Send to BLE Server to notify client
        // (This function handles checking if a client is actually connected)
        ble_server_notify_spectral_data(channels);
        
        // Optional: Print to USB Serial for debugging
        printf("Spec: %.2f, %.2f, %.2f ...\n", channels[0], channels[1], channels[2]);
    }

    // Reschedule timer
    btstack_run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(ts);
}

/**
 * @brief Main HCI Packet Handler
 * Handles the "State = Working" event to start advertising.
 * Other events (Connection/Disconnection) are handled inside ble_server.c
 */
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
            
            // BLE Stack is up -> Start Advertising
            ble_server_start_advertising();
            
            // Print address for verification
            bd_addr_t local_addr;
            gap_local_bd_addr(local_addr);
            printf("BLE Ready. Address: %s\n", bd_addr_to_str(local_addr));
            break;

        case HCI_EVENT_LE_META:
            // Forward connection events to the server logic so it can track state
            ble_server_handle_hci_event(packet_type, channel, packet, size);
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            // Forward disconnection events
            ble_server_handle_hci_event(packet_type, channel, packet, size);
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE: 
            // Forward MTU events
            ble_server_handle_hci_event(packet_type, channel, packet, size);
            break;
    }
}

// --- Main Entry Point ---

int main() {
    // Initialize Standard IO (USB Serial)
    stdio_init_all();
    sleep_ms(2000); // Wait for serial monitor

    // Wait for the USB serial connection to open
    // while (!stdio_usb_connected()) {
    //     sleep_ms(100); // Optional: add a small delay to prevent busy-waiting
    // }

    printf("--- Pico 2W AS7265x BLE ---\n");

    // Initialize Wi-Fi/BLE Chip (CYW43)
    if (cyw43_arch_init()) {
        printf("Error: Failed to init CYW43.\n");
        return -1;
    }

    // Initialize I2C
    i2c_init(I2C_PORT, AS7265X_I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Initialize AS7265x Sensor
    printf("Initializing Sensor...\n");
    if (!as7265x_init(&sensor, I2C_PORT)) {
        printf("Sensor Init Failed! Check wiring.\n");
        // Flash LED rapidly to indicate error
        while(true) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
        }
    }
    printf("Sensor Ready.\n");

    // Visual indicator: Turn on onboard LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // 5. Setup BLE Stack
    l2cap_init();
    sm_init();

    // Initialize the Server Logic (ATT Server)
    // We pass our packet_handler so the server can hook into events if needed
    ble_server_init(packet_handler);

    // Register our main packet handler for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Start the Heartbeat Timer (Sensor Polling)
    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(&heartbeat);

    // Turn on the Bluetooth Radio!
    hci_power_control(HCI_POWER_ON);

    // Run Main Loop
    printf("Starting BTstack...\n");
    btstack_run_loop_execute();

    return 0;
}