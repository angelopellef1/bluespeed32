/**
 * @file secrets_template.h
 * @brief Template for sensitive configuration data
 * 
 * Copy this file to secrets.h and fill in your actual values.
 * The secrets.h file will be ignored by git to keep your credentials secure.
 * 
 * @author AngeloP
 * @date 2025-01-XX
 * @version 1.0
 */

#ifndef SECRETS_TEMPLATE_H
#define SECRETS_TEMPLATE_H

// WiFi Configuration
#define WIFI_SSID "your_wifi_ssid_here"
#define WIFI_PASSWORD "your_wifi_password_here"

// Home Assistant Configuration
#define HA_SERVER "your_home_assistant_server_here"  // e.g., "192.168.1.100" or "home.example.com"
#define HA_PORT 8123
#define HA_TOKEN "your_home_assistant_long_lived_token_here"

// Bluetooth Configuration
#define BT_DEVICE_NAME "your_bluetooth_device_name_here"
#define BT_LOCAL_NAME "your_local_bluetooth_name_here"

#endif // SECRETS_TEMPLATE_H 