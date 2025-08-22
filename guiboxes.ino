/**
 * @file guiboxes.ino.
 * @brief Indexes and draw gui sprites for data informations
 * 
 * 
 * @author AngeloP
 * @date 2025-06-01
 * @version 1.0
 */


#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include "aphud.h"
//#include "Formula1_Bold_web_048pt7b.h"
#include "CardotSemibold7pt7b.h"
//#include "CardotSemibold22pt7b.h"
#include "CardotSemibold12pt7b.h"
#include "CardotSemibold14pt7b.h"
//#include "CardotSemibold42pt7b.h"
#include "CardotSemibold48pt7b.h"


#define TFT_C_DARKBLUE  0x0176
#define TFT_C_YELLORANCE 0xFEE0
#define TFT_C_DARK_GREEN 0x0a60
#define TFT_C_DARK_MAGENTA 0x480a
#define TFT_C_ORANGE_D  0xfa80


#define GB_TEXTBOX_H 16
#define GB_TEXTBOXR_W 34
#define GB_MNUMBOX_H 34

#define GB_MNUMBOX_W 75

#define GB_BNUMBOX_W 85
#define GB_BNUMBOX_H 133 






typedef enum 
{
    GB_BIG,
    GB_MEDIUM,
    GB_MEDIUM_RT,
    GB_SMALL,
    GB_WIDELINE
}boxtypes;


 typedef struct 
 {
    obd_pid_states  pi;
    boxtypes        gb_type;
    int             x;
    int             y;
    int             text_size;
 } guiboxes_t;
 
#define COLOR_LEVELS    4

typedef struct 
 {
    obd_pid_states  pi;
    float thr_values[COLOR_LEVELS];    // index 0 is for lower, 1 default,  other for greater
    uint16_t colours[COLOR_LEVELS];
    uint16_t colours_text[COLOR_LEVELS];
 } color_profiles_t;

#define MAX_GUIBOXES    8

 const color_profiles_t color_profile[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   {1980,1981,3500,6000}, {TFT_C_DARKBLUE, TFT_C_DARK_GREEN, TFT_BLACK, TFT_BLACK}, {TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE} },
    {SPEED,     {0,1,135,210}, {TFT_BLACK, TFT_BLACK, TFT_C_DARK_MAGENTA, TFT_C_YELLORANCE}, {TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_BLACK} },
    {GEAR_C,    {1000,1001,4800,5800}, {TFT_BLACK, TFT_BLACK, TFT_C_YELLORANCE, TFT_C_ORANGE_D}, {TFT_WHITE, TFT_WHITE, TFT_BLACK, TFT_BLACK} },
    {OIL,       {80,81,120,125},        {TFT_C_DARKBLUE, TFT_BLACK, TFT_C_YELLORANCE, TFT_RED}, {TFT_WHITE, TFT_WHITE, TFT_BLACK, TFT_BLACK} },
    {COOLANT,   {80,81,100,110},        {TFT_C_DARKBLUE, TFT_BLACK, TFT_C_YELLORANCE, TFT_RED}, {TFT_WHITE, TFT_WHITE, TFT_BLACK, TFT_BLACK} },
    {FUEL_CUSTOM, {10,11,12,95},      {TFT_C_YELLORANCE, TFT_BLACK, TFT_BLACK, TFT_C_DARK_GREEN}, {TFT_BLACK, TFT_WHITE, TFT_WHITE, TFT_WHITE} },
    {V_ENG_RPM, {2500,2501,5000,6000},      {TFT_BLACK, TFT_BLACK, TFT_BLACK, TFT_BLACK}, {TFT_WHITE, TFT_CYAN, TFT_C_YELLORANCE, TFT_RED} },
 };




#if 1
// gerar on the LEFT
 const guiboxes_t guiboxes[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   GB_MEDIUM_RT,   GB_BNUMBOX_W + GB_MNUMBOX_W +2,     4,                      4},
    {SPEED,     GB_MEDIUM,      GB_BNUMBOX_W + 1,                   4,                      4},
    {GEAR_C,    GB_BIG,         0,                                  4,                      4}, 
    {OIL,       GB_MEDIUM,      GB_BNUMBOX_W + 1,                   84,                     4},
    {COOLANT,   GB_MEDIUM,      GB_BNUMBOX_W + 1 + GB_MNUMBOX_W +2 ,84,                     4},
    {FUEL_CUSTOM, GB_SMALL,    GB_BNUMBOX_W + 1+  42,               57,                     2},
    {V_ENG_RPM, GB_WIDELINE,    0,                                  0,                      4}
 };
#endif


#if 0
// gerar on the right
 const guiboxes_t guiboxes[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   GB_MEDIUM,  0,4,    4},
    {SPEED,     GB_MEDIUM,  GB_MNUMBOX_W,4,    4},
    {GEAR_C,    GB_BIG,     154,4,    4}, //ok
    {OIL,       GB_MEDIUM,  0,82,    4},
    {COOLANT,   GB_MEDIUM,  GB_MNUMBOX_W,82,    4},
    {V_ENG_RPM, GB_WIDELINE, 0,0, 4}
 };
#endif


#if 0 //original centered

 const guiboxes_t guiboxes[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   GB_MEDIUM,  0,4,    4},     //75x36 original numberbox mid
    {SPEED,     GB_MEDIUM,  165,4,    4},
    {GEAR_C,    GB_BIG,     76,4,    4},    //86x133 original numberboxbig
    {OIL,       GB_MEDIUM,  0,94,    4},
    {COOLANT,   GB_MEDIUM,  165,94,    4},
    {V_ENG_RPM, GB_WIDELINE, 0,0, 4}
 };
#endif

#if 0
 const guiboxes_t guiboxes[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   GB_MEDIUM,  0,4,    4},
    {SPEED,     GB_MEDIUM,  76,4,    4},
    {GEAR_C,    GB_BIG,     152,4,    4},
    {OIL,       GB_MEDIUM,  0,90,    4},
    {COOLANT,   GB_MEDIUM,  76,90,    4},
    {V_ENG_RPM,   GB_WIDELINE, 0,0, 4}
 };
#endif

void GUI_FirstSplash() 
{
    img.createSprite(240, 135);
    img.setTextWrap(false); 
    img.fillSprite(TFT_BLACK);
    img.setSwapBytes(true);
    img.pushImage(0,0,240,135,aphud);
    img.setTextWrap(false); 
    // Set text coordinate datum to middle centre
    img.setTextDatum(MC_DATUM);
    // Draw the number in middle of 80 x 50 sprite
    img.setTextSize(1);           // Font size scaling is x1
    img.setTextColor(TFT_DARKGREY);  // White text, no background colour
    img.setFreeFont(&Orbitron_Light_24);  // Select free font Formula1_Bold_web_020pt7bBitmaps
    img.drawString("APHUD", 180,45);
    img.drawString("BRZ", 200,70);
    img.drawString("v0.15", 190,95);

    // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
    img.pushSprite(0, 0);

    // Delete sprite to free up the RAM
    //img.deleteSprite();

}

void GUI_MoveSplash() 
{
    for(int i = 0; i<200;i++)
    {
         img.pushSprite(i, 0);
    }
    img.deleteSprite();
}

void GUI_Shutdown() 
{
    img.createSprite(240, 135);
    img.setTextWrap(false);
    img.fillSprite(TFT_BLACK);
    img.setTextDatum(MC_DATUM);
    img.setTextSize(1);
    img.setTextColor(TFT_WHITE);
    img.setFreeFont(&Orbitron_Light_24);
    img.drawString("Goodbye!", 120, 67);
    img.pushSprite(0, 0);

}


void GUI_ConnectedSplash(String  text, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{

    img.createSprite(200, 135);
    img.setTextWrap(false); 
    // Fill it with black
    img.fillSprite(color_bkg);
    img.setTextSize(1);           // Font size scaling is x1
    img.setTextColor(color_text);  

    img.setTextDatum(MC_DATUM);
    img.setFreeFont(&CardotSemibold12pt7b);  
    img.drawString(text, 10, 67,font_size);
    img.pushSprite(x, y);
    img.deleteSprite();
}

void GUI_DataHeaders()
{
    textBox("KMH",
            GB_BNUMBOX_W +1 , 
            4 + GB_MNUMBOX_H , 
            1, 
            TFT_DARKGREY,TFT_BLACK);

    textBox("RPM",
            GB_BNUMBOX_W +1+ GB_MNUMBOX_W+1 , 
            4 + GB_MNUMBOX_H , 
            1, 
            TFT_DARKGREY,TFT_BLACK);


    textBox("Oil",
            GB_BNUMBOX_W +1 , 
            81 + 4 + GB_MNUMBOX_H , 
            1, 
            TFT_DARKGREY,TFT_BLACK);

    textBox("Cool",
            GB_BNUMBOX_W +1+ GB_MNUMBOX_W+1 , 
            81 + 4 + GB_MNUMBOX_H , 
            1, 
            TFT_DARKGREY,TFT_BLACK);        
    
    textBox("Fuel",
            GB_BNUMBOX_W + 1 , 
            61 , 
            1, 
            TFT_BLACK, TFT_DARKGREY);     
}


void DrawNumberBox_Big(String  num, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(85, 133);
    img.setTextWrap(false); 

    static int flip = 0;
    flip++;

    if(flip % 2)
    {
        if(color_bkg == TFT_C_ORANGE_D)
        {
            color_bkg = TFT_BLACK;
            color_text = TFT_C_ORANGE_D;
        }

    }



    img.fillSprite(color_bkg);

    img.setTextSize(1);                             // Font size scaling is x1
    img.setFreeFont(&CardotSemibold48pt7b);    //Orbitron_Light_24); 

    

    img.setTextColor(color_text);  


    img.setTextDatum(MC_DATUM);


    img.drawString(num + String(" ") , 0, 50);

    img.setFreeFont(&CardotSemibold7pt7b);    //Orbitron_Light_24); 
    img.drawString("Gear", 0, 81 + 4 + GB_MNUMBOX_H);

    // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
    img.pushSprite(x, y);

    img.deleteSprite();
}

void textBox(String  txt, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(GB_MNUMBOX_W, GB_TEXTBOX_H);
    img.setTextWrap(false); 
    img.fillSprite(color_bkg);
    img.setTextDatum(MC_DATUM);

    img.setTextSize(1);
    img.setFreeFont(&CardotSemibold7pt7b);
    img.setTextColor(color_text);  

    img.drawString(txt, 2, 6);

    img.pushSprite(x, y);

    img.deleteSprite();
}

void textBox_r(String  txt, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(GB_TEXTBOXR_W, GB_TEXTBOX_H);
    img.setTextWrap(false); 
    img.fillSprite(color_bkg);
    img.setTextDatum(MC_DATUM);
    img.setTextSize(1);
    img.setFreeFont(&CardotSemibold7pt7b);
    img.setTextColor(color_text);  
    img.drawString(txt, 2, 6);
    img.pushSprite(x, y);
    img.deleteSprite();
}


void wideline(float  data, int x, int y,  uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(240, 4);
    img.fillSprite(color_bkg);

    //to be aligned with this
    //{V_ENG_RPM, {1980,2300,5000,6000},      {TFT_BLACK, TFT_BLACK, TFT_BLACK, TFT_BLACK}, {TFT_WHITE, TFT_C_YELLORANCE, TFT_C_YELLORANCE, TFT_C_YELLORANCE} },


    int outputValue_low = map((int)data, 2300, 3500, 0, 240);
    int outputValue_high = map((int)data, 5000, 6500, 0, 240);  

    if((data > 3500) && (data < 5000))
    {
        // no darw range
        outputValue_low = 0;
        outputValue_high = 0;
    }

    if(data > 5000)
    {
        img.fillRect(x, y, outputValue_high, 4, color_text);
    }
    else
    {
        img.fillRect(x, y, outputValue_low, 4, color_text);
    }
   
    img.pushSprite(x, y);

    img.deleteSprite();
}



void numberBox_rt(String  num, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(GB_MNUMBOX_W, GB_MNUMBOX_H);
    img.setTextWrap(false); 
    img.fillSprite(color_bkg);
    img.setFreeFont(&CardotSemibold12pt7b);    
    img.setTextColor(color_text);  
    img.setTextDatum(MC_DATUM);
    img.drawString(num, 34, 14);
    img.pushSprite(x, y);
    img.deleteSprite();
}

void numberBox_rtf(String  num, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(GB_MNUMBOX_W-15, GB_MNUMBOX_H-8);
    img.setTextWrap(false); 
    img.fillSprite(color_bkg);
    img.setFreeFont(&CardotSemibold12pt7b);    
    img.setTextColor(color_text);  
    img.setTextDatum(MC_DATUM);
    img.drawString(num, 30, 10);
    img.pushSprite(x, y);
    img.deleteSprite();
}


void numberBox(String  num, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    img.createSprite(GB_MNUMBOX_W, GB_MNUMBOX_H);
    img.setTextWrap(false); 
    img.fillSprite(color_bkg);
    img.setFreeFont(&CardotSemibold14pt7b);    
    img.setTextColor(color_text);  
    img.setTextDatum(MC_DATUM);
    img.drawString(num, 34, 14);
    img.pushSprite(x, y);
    img.deleteSprite();
}


void GuiColors_get(obd_pid_states pi, float in_value, uint16_t * bkg, uint16_t * forec)
{
    int pid_s = 0;

    for(pid_s = 0; pid_s< MAX_GUIBOXES; pid_s++ )
    {
        if ((int)pi == color_profile[pid_s].pi)
        {
            break;
        }
    }

    if(pid_s == MAX_GUIBOXES)
    {
        pid_s = 0;
    }

    //default colors
    *bkg = color_profile[pid_s].colours[1];
    *forec = color_profile[pid_s].colours_text[1];

    if(in_value < color_profile[pid_s].thr_values[0])
    {
        *bkg = color_profile[pid_s].colours[0];
        *forec = color_profile[pid_s].colours_text[0];
        return;
    }
    else
    {
        for(int thrp = 1; thrp < COLOR_LEVELS; thrp++)
        {
            if(in_value > color_profile[pid_s].thr_values[thrp])
            {
                *bkg = color_profile[pid_s].colours[thrp];
                *forec = color_profile[pid_s].colours_text[thrp];
            }
        }
    }

    return;
}


 void GuiBox_draw(obd_pid_states pi,  float data)
 {
    String Sdata = "";
    uint16_t bkgr, textc;

    for(int i = 0; i < MAX_GUIBOXES; i++)
    {
        if(pi == guiboxes[i].pi)
        {
            if(pi == GEAR_C)
            {
                // gear colors are related to rpm
                GuiColors_get(pi, car.rpm, &bkgr, &textc);
            }
            else
            {
                GuiColors_get(pi, data, &bkgr, &textc);
            }

            if(data == UNDEFINED_VALUE)
            {
                Sdata = "";
            }
            else
            {
                Sdata = String(data,0);
            }


            if( guiboxes[i].gb_type == GB_BIG)
            {
                DrawNumberBox_Big(Sdata, guiboxes[i].x,  guiboxes[i].y,  guiboxes[i].text_size, bkgr, textc ) ;
            }
            else if(guiboxes[i].gb_type == GB_MEDIUM)
            {
                numberBox(Sdata, guiboxes[i].x,  guiboxes[i].y,  guiboxes[i].text_size, bkgr, textc );
            }
            else if(guiboxes[i].gb_type == GB_MEDIUM_RT)
            {
                numberBox_rt(Sdata, guiboxes[i].x,  guiboxes[i].y,  guiboxes[i].text_size, bkgr, textc );
            }
            else if(guiboxes[i].gb_type == GB_WIDELINE)
            {
                wideline(data, guiboxes[i].x,  guiboxes[i].y, bkgr, textc );
            }
            else if(guiboxes[i].gb_type == GB_SMALL)
            {
                numberBox_rtf(Sdata, guiboxes[i].x,  guiboxes[i].y,  guiboxes[i].text_size, bkgr, textc );
            }
            
            break;
        }
    }
    
 }