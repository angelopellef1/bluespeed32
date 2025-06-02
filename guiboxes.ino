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

#define TFT_C_DARKBLUE  0x0176
#define TFT_C_YELLORANCE 0xFEE0
#define TFT_C_DARK_GREEN 0x0a60
#define TFT_C_DARK_MAGENTA 0x480a




typedef enum 
{
    GB_BIG,
    GB_MEDIUM,
    GB_SMALL,
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

#define MAX_GUIBOXES    5

 const color_profiles_t color_profile[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   {2500,2501,3500,6000}, {TFT_C_DARKBLUE, TFT_C_DARK_GREEN, TFT_BLACK, TFT_BLACK}, {TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE} },
    {SPEED,     {0,1,135,210}, {TFT_BLACK, TFT_BLACK, TFT_C_DARK_MAGENTA, TFT_C_YELLORANCE}, {TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_BLACK} },
    {GEAR_C,    {1000,1001,5500,6000}, {TFT_BLACK, TFT_BLACK, TFT_C_YELLORANCE, TFT_RED}, {TFT_WHITE, TFT_WHITE, TFT_BLACK, TFT_BLACK} },
    {OIL,       {80,81,118,125},        {TFT_C_DARKBLUE, TFT_BLACK, TFT_C_YELLORANCE, TFT_RED}, {TFT_WHITE, TFT_WHITE, TFT_BLACK, TFT_BLACK} },
    {COOLANT,   {85,86,105,110},        {TFT_C_DARKBLUE, TFT_BLACK, TFT_C_YELLORANCE, TFT_RED}, {TFT_WHITE, TFT_WHITE, TFT_BLACK, TFT_BLACK} }
 };





 const guiboxes_t guiboxes[MAX_GUIBOXES] = 
 {
    {ENG_RPM,   GB_MEDIUM,  0,0,    4},
    {SPEED,     GB_MEDIUM,  165,0,    4},
    {GEAR_C,    GB_BIG,     76,1,    4},
    {OIL,       GB_MEDIUM,  0,90,    4},
    {COOLANT,   GB_MEDIUM,  165,90,    4}
 };


void DrawNumberBox_Big(String  num, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
    // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
    img.createSprite(88, 133);
    img.setTextWrap(false); 
    // Fill it with black
    img.fillSprite(color_bkg);



    // Set the font parameters
    img.setTextSize(3);           // Font size scaling is x1
    img.setFreeFont(&Orbitron_Light_24);  // Select free font Formula1_Bold_web_020pt7bBitmaps
    img.setTextColor(color_text);  // White text, no background colour

    // Set text coordinate datum to middle centre
    img.setTextDatum(MC_DATUM);

    // Draw the number in middle of 80 x 50 sprite
    img.drawString(num+"\0", 30, 67);

    // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
    img.pushSprite(x, y);

    // Delete sprite to free up the RAM
    img.deleteSprite();
}

void numberBox(String  num, int x, int y, int font_size, uint16_t color_bkg, uint16_t color_text)
{
  // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
    img.createSprite(75, 45);
    img.setTextWrap(false); 
    // Fill it with black
    img.fillSprite(color_bkg);


    // Set the font parameters
    img.setTextSize(1);           // Font size scaling is x1
    //img.setFreeFont(&FreeSerifBoldItalic24pt7b);  // Select free font
    img.setTextColor(color_text);  // White text, no background colour

    // Set text coordinate datum to middle centre
    img.setTextDatum(MC_DATUM);
    img.drawString(num+"\0", 37, 25,font_size);

    // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
    img.pushSprite(x, y);

    // Delete sprite to free up the RAM
    img.deleteSprite();
}

void GuiColors_get(obd_pid_states pi, float in_value, uint16_t * bkg, uint16_t * forec)
{
    int pid_s = (int) pi;

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
                Sdata = "-";
            }
            else
            {
                Sdata = String(data,0);
            }


            if( guiboxes[i].gb_type == GB_BIG)
            {
                DrawNumberBox_Big(Sdata, guiboxes[i].x,  guiboxes[i].y,  guiboxes[i].text_size, bkgr, textc ) ;
            }
            else
            {
                numberBox(Sdata, guiboxes[i].x,  guiboxes[i].y,  guiboxes[i].text_size, bkgr, textc );
            }
            
            break;
        }
    }
    
 }