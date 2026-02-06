#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include "btstack.h"
#include <stdbool.h>

/**
 * @brief Initialize the ATT server with the profile data and callbacks.
 * @param att_packet_handler The main HCI event handler to register with the ATT server.
 */
void ble_server_init(btstack_packet_handler_t att_packet_handler);

/**
 * @brief Start advertising the Pico as a peripheral.
 */
void ble_server_start_advertising(void);

/**
 * @brief Stop advertising.
 */
void ble_server_stop_advertising(void);

/**
 * @brief Handle HCI events related to the server role (connection, disconnection).
 */
void ble_server_handle_hci_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

/**
 * @brief Sends a notification with spectral data.
 * Formats the data (e.g., "R:45.2 G:12.1...") and sends it to the app.
 * * @param channels Array of 18 float values from the sensor.
 */
void ble_server_notify_spectral_data(float *channels);

// --- State Management Functions ---
hci_con_handle_t ble_server_get_con_handle(void);

#endif // BLE_SERVER_H