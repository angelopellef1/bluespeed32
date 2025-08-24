# Secrets Configuration Setup

This project uses a separate `secrets.h` file to store sensitive configuration data like WiFi credentials and API tokens. This keeps your credentials secure and prevents them from being accidentally committed to version control.

## Setup Instructions

### 1. Create Your Secrets File

Copy the template file to create your actual secrets file:

```bash
cp secrets_template.h secrets.h
```

### 2. Configure Your Secrets

Edit `secrets.h` and replace the placeholder values with your actual credentials:

```cpp
// WiFi Configuration
#define WIFI_SSID "your_actual_wifi_ssid"
#define WIFI_PASSWORD "your_actual_wifi_password"

// Home Assistant Configuration
#define HA_SERVER "your_home_assistant_server"  // e.g., "192.168.1.100" or "home.example.com"
#define HA_PORT 8123
#define HA_TOKEN "your_actual_home_assistant_token"

// Bluetooth Configuration
#define BT_DEVICE_NAME "your_bluetooth_device_name"
#define BT_LOCAL_NAME "your_local_bluetooth_name"
```

### 3. Security Notes

- ✅ The `secrets.h` file is automatically ignored by git (see `.gitignore`)
- ✅ Never commit your actual `secrets.h` file to version control
- ✅ The `secrets_template.h` file is safe to commit as it contains no real credentials
- ✅ Keep a backup of your `secrets.h` file in a secure location

### 4. Home Assistant Token Setup

To get your Home Assistant long-lived access token:

1. Go to your Home Assistant instance
2. Navigate to **Settings** → **Users**
3. Click on your user profile
4. Scroll down to **Long-Lived Access Tokens**
5. Click **Create Token**
6. Give it a name (e.g., "ESP32 Fuel Monitor")
7. Copy the generated token and paste it in `secrets.h`

### 5. File Structure

```
bluespeed_esp32/
├── bluespeed_esp32.ino      # Main sketch (includes secrets.h)
├── secrets.h                # Your actual secrets (ignored by git)
├── secrets_template.h       # Template file (safe to commit)
├── .gitignore              # Git ignore rules
└── README_SECRETS.md       # This file
```

### 6. Troubleshooting

If you get compilation errors about missing definitions:

1. Make sure `secrets.h` exists in the same directory as `bluespeed_esp32.ino`
2. Verify that all required constants are defined in `secrets.h`
3. Check that the `#include "secrets.h"` line is present in the main sketch

### 7. Sharing Your Project

When sharing this project with others:

1. **DO NOT** include `secrets.h` in any shared files
2. **DO** include `secrets_template.h` so others know what to configure
3. **DO** include this README file
4. **DO** mention in your project description that users need to set up their own `secrets.h` file

## Example secrets.h

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Configuration
#define WIFI_SSID "MyHomeWiFi"
#define WIFI_PASSWORD "MySecurePassword123"

// Home Assistant Configuration
#define HA_SERVER "192.168.1.100"
#define HA_PORT 8123
#define HA_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."

// Bluetooth Configuration
#define BT_DEVICE_NAME "vLinker FD-Android"
#define BT_LOCAL_NAME "ESP32_HUD"

#endif // SECRETS_H
``` 