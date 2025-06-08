#include "BluetoothSerial.h"
#define DEBUG true
#include "ELMduino.h"

#define LOAD_GFXFF
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>

#include "Formula1_Bold_web_020pt7b.h"

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
  PID_N
} obd_pid_states;

obd_pid_states obd_state = ENG_RPM;

const int UNDEFINED_GEAR = 9;

String Srpm = "";
String Svss = "";
String Sgear = "";
String Soil = "";

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
    
    // Init communication
    //tft.fillRect(0,0,40,5,TFT_BLUE);
    //GUI_Splash("AP Hud BRZ",0,0, 4, TFT_BLACK, 0xFEE0);
    GUI_DrawImage_Splash();

  
    DEBUG_PORT.begin(115200);
    // SerialBT.setPin("1234");
    ELM_PORT.begin("wewe", true);

    if (!ELM_PORT.connect("vLinker FD-Android"))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }

    GUI_Move_Splash();

    //bt OK, comm with ELM327
    //tft.fillRect(0,0,40,5,TFT_GREEN);
    //GUI_Splash("Ready to Race",0,0, 4, 0x0176, TFT_WHITE);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Ready to Race ..", 10, 60,   4);

    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }

    Scheduler_Init();

    DEBUG_PORT.println("Connected to ELM327");

    tft.setTextSize(1);
    //tft.fillScreen(TFT_BLACK);
    //tft.setTextColor(TFT_RED, TFT_BLACK);
    
    //tft.drawString("Rpm", X1, Y1,   4);
    //tft.drawString("----",X1 ,Y1+20, FONT_N);

    //tft.drawString("Vss", X2, Y2,   4);
    //tft.drawString("---", X2, Y2+20, FONT_N);
    myELM327.sendCommand_Blocking(HEADERS_ON);
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(75,0,90,135,TFT_DARKGREY);
}

void loop()
{
  static float bk_rpm, rpm, bk_kmh, kmh, gear, oil, cool ;
  req_states req = REQ_OK;

  static int oil_freq = 30;


  obd_state = Scheduler_task_calculate(obd_state);


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
        Scheduler_release(); //obd_state = ENG_RPM;
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        Scheduler_release(); //obd_state = OIL;
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

    default:
    break;

  }
}
