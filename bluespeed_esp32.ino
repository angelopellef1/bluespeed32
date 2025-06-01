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


#define TFT_C_DARKBLUE  0x0176
#define TFT_C_YELLORANCE 0xFEE0

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
  OIL
} obd_pid_states;
obd_pid_states obd_state = ENG_RPM;

//statuys
const int X0 = 200;
const int Y0 = 0;

//rpm
const int X1 = 0;
const int Y1 = 0;

//speed
const int X2 = 165;
const int Y2 = 0;

//gear
const int X3 = 75;
const int Y3 = 0;

//oil
const int X4 = 0;
const int Y4 = 50;


const int FONT_N = 6;
const int FONT_BIG = 24;


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

void DrawNumberBox_Big(String  num, int x, int y, int font_size)
{
  // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
  img.createSprite(90, 135);
   img.setTextWrap(false); 
  // Fill it with black
  img.fillSprite(TFT_DARKGREY);

 
  // Set the font parameters
  img.setTextSize(1);           // Font size scaling is x1
  img.setFreeFont(&Orbitron_Light_24);  // Select free font Formula1_Bold_web_020pt7bBitmaps
  img.setTextColor(TFT_YELLOW);  // White text, no background colour

  // Set text coordinate datum to middle centre
  img.setTextDatum(MC_DATUM);

  // Draw the number in middle of 80 x 50 sprite
  img.drawString(num+"\0", 45, 67);

  // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
  img.pushSprite(x, y);

  // Delete sprite to free up the RAM
  img.deleteSprite();
}


void numberBox(String  num, int x, int y, int font_size)
{
  // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
  img.createSprite(75, 45);
   img.setTextWrap(false); 
  // Fill it with black
  img.fillSprite(TFT_C_DARKBLUE);

 
  // Set the font parameters
  img.setTextSize(1);           // Font size scaling is x1
  //img.setFreeFont(&FreeSerifBoldItalic24pt7b);  // Select free font
  img.setTextColor(TFT_WHITE);  // White text, no background colour

  // Set text coordinate datum to middle centre
  img.setTextDatum(MC_DATUM);
  img.drawString(num+"\0", 37, 25,font_size);

  // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
  img.pushSprite(x, y);

  // Delete sprite to free up the RAM
  img.deleteSprite();
}


void setup()
{   

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(X0,Y0,40,5,TFT_BLUE);
  
    DEBUG_PORT.begin(115200);
    // SerialBT.setPin("1234");
    ELM_PORT.begin("wewe", true);

    if (!ELM_PORT.connect("vLinker FD-Android"))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }
    tft.fillRect(X0,Y0,40,5,TFT_GREEN);

    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }

    init_ratios();
    
    tft.fillRect(X0,Y0,40,5,TFT_BLACK);

    DEBUG_PORT.println("Connected to ELM327");

    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    
    //tft.drawString("Rpm", X1, Y1,   4);
    //tft.drawString("----",X1 ,Y1+20, FONT_N);

    //tft.drawString("Vss", X2, Y2,   4);
    //tft.drawString("---", X2, Y2+20, FONT_N);
    myELM327.sendCommand_Blocking(HEADERS_ON);
}

void loop()
{
  static float bk_rpm, rpm, bk_kmh, kmh, gear, oil ;
  req_states req = REQ_OK;

  static int oil_freq = 30;

  switch (obd_state)
  {
    case ENG_RPM:
    {
      bk_rpm = rpm;
      rpm = myELM327.rpm();
      Srpm = String(rpm,0);
      

      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        numberBox(Srpm, X1, Y1, 4);
       
        obd_state = SPEED;
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        obd_state = SPEED;
      }
      break;
    }
    
    case SPEED:
    {
      bk_kmh = kmh;
      kmh = myELM327.kph();
      gear = calculate_gear(kmh,rpm);
      Svss = String(kmh,0);
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        numberBox(Svss, X2, Y2, 4);
        gear = calculate_gear(kmh,rpm);
        if(gear != UNDEFINED_GEAR+1)
        {
          Sgear = String(gear,0);
        }
        else
        {
          Sgear = "-";
        }
        DrawNumberBox_Big(Sgear, X3 ,Y3+0, FONT_BIG);
        oil_freq--;
        if(!oil_freq)
        {
          oil_freq = 30;
          obd_state = OIL;
        }
        else
        {
          obd_state = ENG_RPM;
        }
          
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        obd_state = OIL;
      } 
      break;
    }
    
    case OIL:
      req = obdcustom_subaru_oil(&oil);
      if(REQ_OK == req)
      {
        //DRAW
        Soil = String(oil,0);
        numberBox(Soil, X4, Y4, 4);
        obd_state = ENG_RPM;
      }
      else if(REQ_E_FAIL == req)
      {
        //no draw
        numberBox("--", X4, Y4, 4);
        obd_state = ENG_RPM;
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
