#include "BluetoothSerial.h"
#define DEBUG true
#include "ELMduino.h"
#include "secrets.h"  // Include sensitive configuration data

#define LOAD_GFXFF
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>

// WiFi and HTTP libraries
#include <WiFi.h>
#include <HTTPClient.h>

// Only include fonts you actually use
// #include "Formula1_Bold_web_020pt7b.h"  // Commented out if not used

#define SIMULATION_ 0

#define S1_PIN 0        // Change to your S1 button GPIO
#define TFT_BL 4        // Backlight control pin (PWM capable)

// PWM settings
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

volatile bool buttonPressed = false;

volatile bool toggleBrightness = false;
bool isFullBrightness = true;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 500;  // 500 ms



TFT_eSPI tft = TFT_eSPI(); 

TFT_eSprite img = TFT_eSprite(&tft);

BluetoothSerial SerialBT;
#define ELM_PORT SerialBT
#define DEBUG_PORT Serial

#define UNDEFINED_VALUE  0x7F7FFFFF 

//Display 240X135
ELM327 myELM327;

enum req_states 
{ 
    REQ_OK,
    REQ_E_WAIT,
    REQ_E_FAIL
} ;


enum req_stages
{ 
    STEP_CHANGE_HEADER,
    STEP_PID,
    STEP_RESTORE_HEADER,
};


typedef enum {
  ENG_RPM,
  SPEED,
  GEAR_C,
  OIL,
  COOLANT,
  FUEL_CUSTOM,
  PID_N,
  V_ENG_RPM
} obd_pid_states;

obd_pid_states obd_state = ENG_RPM;

const int UNDEFINED_GEAR = 9;


#define SYNC_BT_DISCONNECTED 0
#define SYNC_WIFI_CONNECTED 1
#define SYNC_DATA_OK 2
#define SYNC_DATA_FAIL 3
#define SYNC_WIFI_FAILED 4

String Srpm = "";
String Svss = "";
String Sgear = "";
String Soil = "";

void IRAM_ATTR handleButtonInterrupt() 
{
    buttonPressed = true;
}

int digitCount(int num) 
{
    if(num == 0) 
    {
        return 1;
    }
    int count = 0;
    while(num != 0) 
    {
        num /= 10;
        count++;
    }
    return count;
}

struct car_t {
  float rpm;
  float speed;
  float gear;
  float oil;
  float coolant;
  float fuel;
  uint32_t update_flags;  // Bit flags for GUI updates
} car;

void Connect_BT()
{
    // SerialBT.setPin("1234");
    ELM_PORT.begin(BT_LOCAL_NAME, true);

    if (!ELM_PORT.connect(BT_DEVICE_NAME))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }
}

void Connect_ELM()
{
    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }
}

void setup()
{   

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Set up TFT
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(TFT_BL, ledChannel);

    // Initial brightness
    ledcWrite(ledChannel, 255);  // 100%

    // Button setup
    pinMode(S1_PIN, INPUT_PULLUP);  // Assuming active LOW
    attachInterrupt(digitalPinToInterrupt(S1_PIN), handleButtonInterrupt, FALLING);
    
    // Init communication
    //tft.fillRect(0,0,40,5,TFT_BLUE);
    //GUI_Splash("AP Hud BRZ",0,0, 4, TFT_BLACK, 0xFEE0);
    GUI_FirstSplash();

#ifdef SIMULATION
    GUI_MoveSplash();

    GUI_ConnectedSplash("Simulation ",0,0,1,TFT_BLACK, TFT_DARKGREY);
    Scheduler_Init();

    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    GUI_DataHeaders();
    return;
#endif
  
    DEBUG_PORT.begin(115200);

    Connect_BT();

    GUI_MoveSplash();

    GUI_ConnectedSplash("Get Ready ",0,0,1,TFT_BLACK, TFT_DARKGREY);

    Connect_ELM();

    Scheduler_Init();

    DEBUG_PORT.println("Connected to ELM327");

    tft.setTextSize(1);
    myELM327.sendCommand_Blocking(HEADERS_ON);
    tft.fillScreen(TFT_BLACK);


    GUI_DataHeaders();
    car.update_flags = 0;  // Initialize update flags
}

#define SYSTEM_SHUTDOWN 0
#define SYSTEM_RUNNING 1

unsigned long lowRpmStartTime = 0;
int System_Mode = SYSTEM_RUNNING;

// Variables for triple button press tracking
unsigned long firstPressTime = 0;
unsigned long secondPressTime = 0;
unsigned long thirdPressTime = 0;
int pressCount = 0;
bool triplePressDetected = false;
const unsigned long FIVE_SECONDS = 5000; // 5 seconds in milliseconds




// Function to handle WiFi connection and HTTP data transmission to Home Assistant
void sendFuelDataViaWiFi(float fuelValue) 
{
    int attempts = 0;
    
    DEBUG_PORT.println("Disconnecting from Bluetooth...");
    ELM_PORT.disconnect();
    GUI_DrawSyncStatus(SYNC_BT_DISCONNECTED);
    delay(1000);
    
    DEBUG_PORT.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while(WiFi.status() != WL_CONNECTED && attempts < 10) 
    {
        delay(500);
        DEBUG_PORT.print(".");
        attempts++;
    }
    
    if(WiFi.status() == WL_CONNECTED) 
    {
        // Status bar turns GREEN for WiFi connected
        GUI_DrawSyncStatus(SYNC_WIFI_CONNECTED);
        
        DEBUG_PORT.println("\nWiFi connected!");
        DEBUG_PORT.print("IP address: ");
        DEBUG_PORT.println(WiFi.localIP());
        
        // Send HTTP request to Home Assistant
        HTTPClient http;
        String url = "http://" + String(HA_SERVER) + ":" + String(HA_PORT) + "/api/states/input_number.fuel_level";
        http.begin(url);
        
        // Add authorization header
        http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
        http.addHeader("Content-Type", "application/json");
        
        // Create JSON payload
        String payload = "{\"state\":\"" + String(fuelValue, 1) + "\",\"attributes\":{\"unit_of_measurement\":\"L\"}}";
        
        int httpResponseCode = http.POST(payload);
        
        if(httpResponseCode > 0) 
        {
            String response = http.getString();
            DEBUG_PORT.println("Home Assistant Response code: " + String(httpResponseCode));
            DEBUG_PORT.println("Response: " + response);
            GUI_DrawSyncStatus(SYNC_DATA_OK);
        } 
        else 
        {
            DEBUG_PORT.println("Home Assistant request failed");
            GUI_DrawSyncStatus(SYNC_DATA_FAIL);
        }

        http.end();
        // Disconnect from WiFi
        WiFi.disconnect();
        DEBUG_PORT.println("WiFi disconnected");
    } 
    else 
    {
        GUI_DrawSyncStatus(SYNC_WIFI_FAILED);
        DEBUG_PORT.println("WiFi connection failed");
        WiFi.disconnect();

    }
    
    // Reconnect to Bluetooth and ELM
    DEBUG_PORT.println("Reconnecting to Bluetooth and ELM...");
    
    Connect_BT();  // This will handle the Bluetooth connection
    Connect_ELM(); // This will re-establish the ELM327 connection
    
    DEBUG_PORT.println("Bluetooth and ELM reconnected successfully");
    
    // Re-initialize ELM327 settings
    myELM327.sendCommand_Blocking(HEADERS_ON);

}


void SystemMonitoring(float fuel)
{
    // Check if engine has shut down
    //checkEngineShutdown();
    
    // Handle system shutdown mode
    if(System_Mode == SYSTEM_SHUTDOWN) 
    {
        GUI_Shutdown();
        while(true) 
        {
            delay(1000); // Keep the display on
        }
    }
    
    // ========================================
    // TRIPLE BUTTON PRESS DETECTION LOGIC
    // ========================================
    
    if(buttonPressed) 
    {
        unsigned long currentTime = millis();
        
        // Check if enough time has passed since last button press (debounce)
        if(currentTime - lastDebounceTime > debounceDelay) 
        {
            lastDebounceTime = currentTime;
            buttonPressed = false;
            
            // ========================================
            // TRACK BUTTON PRESSES FOR TRIPLE DETECTION
            // ========================================
            
            if(pressCount == 0) 
            {
                // First press - start timing
                firstPressTime = currentTime;
                pressCount = 1;
            } 
            else if(pressCount == 1) 
            {
                // Second press - continue timing
                secondPressTime = currentTime;
                pressCount = 2;
            } 
            else if(pressCount == 2) 
            {
                // Third press - check if triple press completed
                thirdPressTime = currentTime;
                pressCount = 3;
                
                // Check if all three presses happened within 5 seconds
                if((thirdPressTime - firstPressTime) <= FIVE_SECONDS) 
                {
                    DEBUG_PORT.println("Triple press detected within 5 seconds!");
                    triplePressDetected = true;
                    
                    // Execute WiFi data upload
                    sendFuelDataViaWiFi(fuel);

                    // Restore normal GUI display
                    tft.fillScreen(TFT_BLACK);
                    GUI_DataHeaders();
                    Scheduler_Init();
                    
                    // Reset all tracking variables
                    pressCount = 0;
                    firstPressTime = 0;
                    secondPressTime = 0;
                    thirdPressTime = 0;
                    triplePressDetected = false;
                    return; // Exit early to prevent brightness toggle
                } 
                else 
                {
                    // Time exceeded - reset to second press as new first press
                    pressCount = 1;
                    firstPressTime = secondPressTime;
                    secondPressTime = thirdPressTime;
                    thirdPressTime = 0;
                }
            }
            
            // ========================================
            // NORMAL BRIGHTNESS TOGGLE (if not triple press)
            // ========================================
            
            if(!triplePressDetected) 
            {
                if(isFullBrightness) 
                {
                    ledcWrite(ledChannel, 100);  // Dim brightness
                } 
                else 
                {
                    ledcWrite(ledChannel, 255);  // Full brightness
                }
                isFullBrightness = !isFullBrightness;
            }
        } 
        else 
        {
            // Debounce: ignore this press (too soon after last press)
            buttonPressed = false;
        }
    } 
    else 
    {
        // Button released - check if we need to reset triple press tracking
        unsigned long currentTime = millis();
        
        // Reset tracking if more than 5 seconds have passed since first press
        if(pressCount > 0 && (currentTime - firstPressTime) > FIVE_SECONDS) 
        {
            pressCount = 0;
            firstPressTime = 0;
            secondPressTime = 0;
            thirdPressTime = 0;
            triplePressDetected = false;
        }
    }
}


void process_gui_updates()
{
    // Check and process each state's update flag
    for (uint8_t i = 0; i < PID_N; i++)
    {
        if (car.update_flags & (1UL << i))
        {
            // Clear the flag first
            car.update_flags &= ~(1UL << i);
            
            // Call appropriate GUI update based on state
            switch (i)
            {
                case ENG_RPM:
                    GuiBox_draw(ENG_RPM, car.rpm);
                    break;
                case V_ENG_RPM:
                    GuiBox_draw(V_ENG_RPM, car.rpm);
                    break;
                case SPEED:
                    GuiBox_draw(SPEED, car.speed);
                    break;
                case GEAR_C:
                    GuiBox_draw(GEAR_C, car.gear);
                    break;
                case OIL:
                    GuiBox_draw(OIL, car.oil);
                    break;
                case COOLANT:
                    GuiBox_draw(COOLANT, car.coolant);
                    break;
                case FUEL_CUSTOM:
                    GuiBox_draw(FUEL_CUSTOM, car.fuel);
                    break;
            }
        }
    }
}

void loop()
{
  static float bk_rpm, rpm, bk_kmh, kmh, gear, oil, cool, fuel;
  req_states req = REQ_OK;
  
  // Handle system monitoring (engine shutdown, button presses, etc.)
  SystemMonitoring(fuel);
  
  obd_state = Scheduler_task_calculate(obd_state);

#ifdef SIMULATION
    obd_state =(obd_pid_states) 0xFF;
    car.speed = 0;
    fuel = random(0, 50);
    car.rpm = 2600;

    GuiBox_draw(ENG_RPM, 2600);
    GuiBox_draw(V_ENG_RPM, 2600);
    GuiBox_draw(SPEED, 113); 
    GuiBox_draw(GEAR_C,6);
    GuiBox_draw(OIL, 125);
    GuiBox_draw(COOLANT, 100);
    GuiBox_draw(FUEL_CUSTOM, fuel);
    // Don't return here - let button logic execute
#endif

    switch(obd_state)
    {
        case ENG_RPM:
        {
            bk_rpm = rpm;
            rpm = myELM327.rpm();
            car.rpm = rpm; 
            if(myELM327.nb_rx_state == ELM_SUCCESS)
            {
                car.update_flags |= (1UL << ENG_RPM) | (1UL << V_ENG_RPM);  // Set both RPM update flags
                Scheduler_release(); //obd_state = SPEED;
            }
            else if(myELM327.nb_rx_state != ELM_GETTING_MSG)
            {
                myELM327.printError();
                Scheduler_release(); //obd_state = SPEED;
            }
            break;
        }
        
        case SPEED:
        {
            bk_kmh = kmh;
            kmh = myELM327.kph();
            car.speed = kmh; // Store speed in car structure
            Svss = String(kmh,0);
            if(myELM327.nb_rx_state == ELM_SUCCESS)
            {
                car.update_flags |= (1UL << SPEED);
                Scheduler_release();
            }
            else if(myELM327.nb_rx_state != ELM_GETTING_MSG)
            {
                myELM327.printError();
                Scheduler_release(); 
            } 
            break;
        }
        
        case GEAR_C:
            car.gear = calculate_gear(kmh,rpm); 
            car.gear++;
            if(gear < 7)
            {
                car.update_flags |= (1UL << GEAR_C);
            }
            else
            {
                car.update_flags |= (1UL << GEAR_C);
            }
            
            Scheduler_release();
            break;


            case OIL:
            req = obdcustom_subaru_oil(&oil);
            if(REQ_OK == req)
            {
                car.oil = oil;
                car.update_flags |= (1UL << OIL);
                Scheduler_release(); 
            }
            else if(REQ_E_FAIL == req)
            {
                car.oil = 1;
                car.update_flags |= (1UL << OIL);
                Scheduler_release(); 
            }
            else
            {
                //wait..
            }
            break;

        case COOLANT:
        {
            cool = myELM327.engineCoolantTemp();
            if(myELM327.nb_rx_state == ELM_SUCCESS)
            {
                car.coolant = cool;
                car.update_flags |= (1UL << COOLANT);
                Scheduler_release(); 
            }
            else if(myELM327.nb_rx_state != ELM_GETTING_MSG)
            {
                myELM327.printError();
                Scheduler_release(); 
            } 
            break;
        }

        case FUEL_CUSTOM:
            req = obdcustom_subaru_fuel(&fuel);
            if(REQ_OK == req)
            {
                car.fuel = fuel;
                car.update_flags |= (1UL << FUEL_CUSTOM);
                Scheduler_release(); 
            }
            else if(REQ_E_FAIL == req)
            {
                car.fuel = 1;
                car.update_flags |= (1UL << FUEL_CUSTOM);
                Scheduler_release(); 
            }
            else
            {
                //wait..
            }
            break;

        default:
            break;
    }
    
    // Process any pending GUI updates
    process_gui_updates();
}
