#include "BluetoothSerial.h"
#define DEBUG true
#include "ELMduino.h"

#define LOAD_GFXFF
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>

#include "Formula1_Bold_web_020pt7b.h"

#define SIMULATION_  0

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

String Srpm = "";
String Svss = "";
String Sgear = "";
String Soil = "";

void IRAM_ATTR handleButtonInterrupt() {
  buttonPressed = true;
}

int digitCount(int num) {
  if (num == 0) return 1;
  int count = 0;
  while (num != 0) {
    num /= 10;
    count++;
  }
  return count;
}

struct car_t {
  float rpm;
} car;


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
    // SerialBT.setPin("1234");
    ELM_PORT.begin("wewe", true);

    if (!ELM_PORT.connect("vLinker FD-Android"))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }

    GUI_MoveSplash();

    GUI_ConnectedSplash("Get Ready ",0,0,1,TFT_BLACK, TFT_DARKGREY);

    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }

    Scheduler_Init();

    DEBUG_PORT.println("Connected to ELM327");

    tft.setTextSize(1);
    myELM327.sendCommand_Blocking(HEADERS_ON);
    tft.fillScreen(TFT_BLACK);


    GUI_DataHeaders();

}

void loop()
{
  static float bk_rpm, rpm, bk_kmh, kmh, gear, oil, cool, fuel ;
  req_states req = REQ_OK;

  static int oil_freq = 30;


  if (buttonPressed) 
  {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > debounceDelay) 
    {
      lastDebounceTime = currentTime;
      buttonPressed = false;

      // Toggle brightness
      if (isFullBrightness) {
        ledcWrite(ledChannel, 100);  // 40
      } else {
        ledcWrite(ledChannel, 255);  // 100%
      }

      isFullBrightness = !isFullBrightness;
      
    } 
    else 
    {
      // Debounce: ignore this press
      buttonPressed = false;
    }
  }


  obd_state = Scheduler_task_calculate(obd_state);

#ifdef SIMULATION
  while(true)
  {
    GuiBox_draw(ENG_RPM, 2600);
    GuiBox_draw(V_ENG_RPM, 2600);
    GuiBox_draw(SPEED, 113); 
    GuiBox_draw(GEAR_C,6);
    GuiBox_draw(OIL, 125);
    GuiBox_draw(COOLANT, 100);
    GuiBox_draw(FUEL_CUSTOM, 49);
  }

#endif

  switch (obd_state)
  {
    case ENG_RPM:
    {
      bk_rpm = rpm;
      rpm = myELM327.rpm();
      car.rpm = rpm; 
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        GuiBox_draw(ENG_RPM, rpm);
        GuiBox_draw(V_ENG_RPM, rpm);
        Scheduler_release(); //obd_state = SPEED;
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
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
      Svss = String(kmh,0);
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        GuiBox_draw(SPEED, kmh); 
        Scheduler_release();
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        Scheduler_release(); 
      } 
      break;
    }
    
    case GEAR_C:
      gear = calculate_gear(kmh,rpm);
      if(gear < 7)
      {
        GuiBox_draw(GEAR_C,gear);
      }
      else
      {
       GuiBox_draw(GEAR_C,UNDEFINED_VALUE);
      }
     
      Scheduler_release();
    break;


    case OIL:
      req = obdcustom_subaru_oil(&oil);
      if(REQ_OK == req)
      {
        
        GuiBox_draw(OIL, oil);
        Scheduler_release(); 
      }
      else if(REQ_E_FAIL == req)
      {
        //no draw
        GuiBox_draw(OIL, 1);
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
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        GuiBox_draw(COOLANT, cool);
        Scheduler_release(); 
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        Scheduler_release(); 
      } 
      break;
    }
    break;

    case FUEL_CUSTOM:
      req = obdcustom_subaru_fuel(&fuel);
      if(REQ_OK == req)
      {
        
        GuiBox_draw(FUEL_CUSTOM, fuel);
        Scheduler_release(); 
      }
      else if(REQ_E_FAIL == req)
      {
        //no draw
        GuiBox_draw(FUEL_CUSTOM, 1);
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
}
