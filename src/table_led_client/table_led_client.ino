#include <FastLED.h>
#include <math.h>
#include <Wire.h>
#include <U8g2lib.h>


//////////////////////////////
//// FAST LED DEFINES ////////
//////////////////////////////


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    142
#define BRIGHTNESS         255
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_LEDS];


//function prototpyes
void rainbow();
void rainbowWithGlitter(fract8 chanceOfGlitter);
void confetti();
void sinelon();
void juggle();
void bpm();
void white();
void warmwhite();
void alternatewhite();
void rbg_color(int CRGB_val);


uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t state = 0;
int CRGB_val = 0;
bool firstrun = true;

//Mapping from state num from switches to function
enum ANIM_FUNC_MAP {
    RAINBOW = 0, 
    JUGGLE,
    BPM,
    WHITE,
    WARMWHITE,
    ALTERNATEWHITE,
    RBG_COLOR,
} ANIM_FUNC_MAP_E;

//////////////////////////////
//// INTF W ESP DEFINES //////
//////////////////////////////


/// MESSAGE STRUCTURE ///
/*
For wifi broken command:
    Command
For everything else:
    Command_On/Off_BrightnessValue_RGBValAsInt
If On/Off is '0', the message simplifies and becomes:
    Command_On/Off

Command is 3 characters always.
Delimiter is '_'
On/Off is '1' or '0'
Brightness value
*/

#warning "Ensure use of Serial.print for comms, not Serial.println which appends ASCII(13) and ASCII(10)"

#define DEBUG_COMMS 0
#define WIFI_EN_PIN 3
#define DELIM '_'

// Commands
#define BROKEN_WIFI "404"
#define SET_STRIP_RGB "rgb"
#define SET_ANIM_RAINBOW "rai"
#define SET_ANIM_JUGGLE "jug"
#define SET_ANIM_BPM "bpm"
#define ACK "Z"

//message size
#define MAX_NUM_SEGMENTS 4
#define MAX_SEGMENT_SIZE 5 //include one extra for null terminator


String g_str = "";

//////////////////////////////
//// OLED DEFINES ////////////
//////////////////////////////

U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, PIN_A5, PIN_A4 , 4);

#define NUM_DISPLAY_LINES 6
#define NUM_CHARS_PER_LINE 23 //additional spot for null terminator

#define OLED_LINE_INCREMENT 12

char buff[NUM_DISPLAY_LINES][NUM_CHARS_PER_LINE];

//////////////////////////////
//// CODE SETUP /////////////
/////////////////////////////


void setup() {
    
    delay(500);
    Serial.begin(38400); 

    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    //Setup inputs
    pinMode(WIFI_EN_PIN, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(10, INPUT);
    pinMode(11, INPUT);

    //Setup OLED
    u8g2.begin(); 
    u8g2.firstPage();
    do {
        //actual pixel hieght is about 10. i think 15 is about the start of the blue section
        u8g2.setFont(u8g2_font_6x12_mr);
        u8g2.drawStr(0,12,"OP Alexa Ctrl v0.2.3");
        u8g2.drawStr(0,24,"by Eric Sherman");
    } while ( u8g2.nextPage() );
    
    strcpy(buff[0],"OP Alexa Ctrl v0.2.3");
    
    delay(1000);
    
}

//////////////////////////////
//// MAIN LOOP //////////////
/////////////////////////////


void loop()
{

    if(digitalRead(WIFI_EN_PIN))
    {
        //Store command in g_str
        get_message(); 

        if(g_str != "")
        {
            
            String test = g_str.substring(0,3);
            Serial.println(test + " thing " + g_str);
            
            if(test.equals(SET_STRIP_RGB))
            {
//                Serial.print(ACK);
                if(g_str[strlen(SET_STRIP_RGB) + 1] == '1')
                {
                    int i=0, count=0;
                    for( ; i < g_str.length(); i++)
                    {
                        if(g_str[i] == DELIM){
                            if(++count== (MAX_NUM_SEGMENTS-1))
                            {
                                break;
                            }
                        }   
                    }
                    write_buffer(1, SET_STRIP_RGB, true);
                    itoa(g_str.substring(i).toInt(),buff[2],10);
                    CRGB_val = g_str.substring(i).toInt();
                    state = RBG_COLOR;
                }
                else
                {
                    write_buffer(1, SET_STRIP_RGB, false);
                    CRGB_val = CRGB::Black;
                    state = RBG_COLOR;
                }
            }
            else if(test.equals(SET_ANIM_BPM))
            {
//                Serial.print(ACK);
                if(g_str[g_str.length()-1] == '1')
                {
                    write_buffer(1, SET_ANIM_BPM, true);
                    strcpy(buff[2], "");
                    state = BPM;
                }
                else
                {
                    write_buffer(1, SET_ANIM_BPM, false);
                    strcpy(buff[2], "");
                    CRGB_val = CRGB::Black;
                    state = RBG_COLOR;
                }
                //print to LCD display
            }
            else if(test.equals(SET_ANIM_JUGGLE))
            {
//                Serial.print(ACK);
                if(g_str[g_str.length()-1] == '1')
                {
                    write_buffer(1, SET_ANIM_JUGGLE, true);
                    strcpy(buff[2], "");
                    state = JUGGLE;
                }
                else
                {
                    write_buffer(1, SET_ANIM_JUGGLE, false);
                    strcpy(buff[2], "");
                    CRGB_val = CRGB::Black;
                    state = RBG_COLOR;
                }
            }
            else if(test.equals(SET_ANIM_RAINBOW))
            {
//                Serial.print(ACK);
                if(g_str[g_str.length()-1] == '1')
                {
                    write_buffer(1, SET_ANIM_RAINBOW, true);
                    strcpy(buff[2], "");
                    state = RAINBOW;
                }
                else
                {
                    write_buffer(1, SET_ANIM_RAINBOW, false);
                    strcpy(buff[2], "");
                    CRGB_val = CRGB::Black;
                    state = RBG_COLOR;
                }
            }
            else if(test.equals(BROKEN_WIFI))
            {
                //will only pick up on this when esp is booting up and wifi is enabled.
                Serial.print(BROKEN_WIFI);
                strcpy(buff[1], "ESP BROKEN WIFI");
                strcpy(buff[2], "");
            }
            else
            {
                //TODO WRITE A THING TO HANDLE ON ESP
                //and do a define for below too
                Serial.println("NZ");
            }

            //Update display with message
            strcpy(buff[3], "State: ");
            strcat(buff[3], char(state + 48)); //maybe want to print the name of it actually? 
            strcpy(buff[4], "Wifi Enabled");
            for(uint8_t i = 5; i < NUM_DISPLAY_LINES; i++)
            {
                strcpy(buff[i], "");
            }
            u8g2.firstPage();
            
            
            do {
                u8g2.setFont(u8g2_font_6x12_mr);
                for(uint8_t i = 0; i < NUM_DISPLAY_LINES; i++)
                {
                    u8g2.drawStr(0,i*OLED_LINE_INCREMENT,buff[i]);
                }
            } while ( u8g2.nextPage() );
            
        }
    }
    else
    {
        //Use toggle pins to determine output.
        //Have 6 states but only 5 switches, so use last method.

        state = (digitalRead(12) << 4) + (digitalRead(11) << 3) + (digitalRead(10) << 2) + (digitalRead(9) << 1) + digitalRead(8);

    }

    //Execute a nice animation between certain states.
    if((state == WHITE || state== WARMWHITE || state == ALTERNATEWHITE || state == RBG_COLOR) && firstrun){
        for(int i =0; i < NUM_LEDS; i++){
            leds[i] = CRGB::Black;
            FastLED.show();
        }
        firstrun=false;
    }

    #warning "this needs to be fixed I think"

    // Reset if new state is not one of the states below.
    if(state != WHITE && state != WARMWHITE && state != ALTERNATEWHITE && state != RBG_COLOR){
        firstrun = true;
    }

    switch(state){
        case RAINBOW:
            rainbow();
            break;
        case JUGGLE:
            juggle();
            break;
        case BPM:
            bpm();
            break;
        case WHITE:
            white();
            break;
        case WARMWHITE:
            warmwhite();
            break;
        case ALTERNATEWHITE:
            alternatewhite();
            break;
        case RBG_COLOR:
            rbg_color(CRGB_val);
            break;
    }
  

    // send the 'leds' array out to the actual LED strip
    FastLED.show();

    // insert a delay to keep the framerate modest
    // FastLED.delay(2); 
    FastLED.delay(1000/FRAMES_PER_SECOND); 

    // do some periodic updates if gHue is used by the function
    if(state == RAINBOW || state == BPM)
    {
        EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    }
}

/////////////////////////////
//// COMMS //////////////////
/////////////////////////////
#if(DEBUG_COMMS)
    void debug_ascii(String str)
    {
        for(int w=0; w < str.length(); w++)  
        {
            Serial.print(str.charAt(w),DEC);
            Serial.print(" ");
        }
        Serial.println("");
    }
#endif

//TODO CONSIDER SWITCHING TO THIS STRUCTURE SO ITS NOT BLOCKING https://forum.arduino.cc/t/serial-input-basics-updated/382007
void get_message()
{
    if(Serial.available() <= 0 )
    {
        g_str="";
        return;
    }
    else
    {
        //grab the last thing out of the buffer.
        //incoming string must be terminated with a new line char
        bool notdone = true;
        int idx = 0;
        g_str="                       ";
        int timeout = 5000;
        unsigned long start = millis();
        while(notdone) {
            if(Serial.available() > 0)
            {
              g_str[idx] = Serial.read();
              
              if(g_str[idx++] == '\n')
              {
                notdone = false;            
              }
            }
            
            if((millis()-start) > timeout)
            {
              break;
            }
        }
        if(g_str.indexOf(char(32)) < g_str.length()  && g_str.indexOf(char(32)) != -1 )
        {
          g_str = g_str.substring(0,g_str.indexOf(char(32)));
        }
        
        Serial.println("");
        
        for(int w=0; w < g_str.length(); w++)  
        {
            Serial.print(g_str.charAt(w),DEC);
            Serial.print(" ");
        }
        Serial.println("");
    }

    
    #if(DEBUG_COMMS)
        if(!g_str.equals("")){
            Serial.println(g_str);
        }
    #endif

}

void write_buffer(uint8_t index, const char * str, bool enable)
{
    strcpy(buff[index], "Command: ");
    strcat(buff[index], str);
    if(enable)
    {
        strcat(buff[index], ", ON");
    }
    else
    {
        strcat(buff[index], ", OFF");
    }
}






//////////////////////////////
//// ANIMATIONS / STATES ////
/////////////////////////////

void rainbow() 
{
 // FastLED's built-in rainbow generator
 fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

//NOT USED
void rainbowWithGlitter() 
{
 // built-in FastLED rainbow, plus some random sparkly glitter
 rainbow();
 addGlitter(80);
}

//NOT USED
void addGlitter( fract8 chanceOfGlitter) //support function for 1
{
 if( random8() < chanceOfGlitter) {
   leds[ random16(NUM_LEDS) ] += CRGB::White;
 }
}

//NOT USED
void confetti() 
{
 // random colored speckles that blink in and fade smoothly
 fadeToBlackBy( leds, NUM_LEDS, 10);
 int pos = random16(NUM_LEDS);
 leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

//NOT USED
void sinelon() 
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void juggle() 
{
 // eight colored dots, weaving in and out of sync with each other
 fadeToBlackBy( leds, NUM_LEDS, 20);
 byte dothue = 0;
 for( int i = 0; i < 8; i++) {
   leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
   dothue += 32;
 }
}

void bpm() 
{
 // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
 uint8_t BeatsPerMinute = 62;
 CRGBPalette16 palette = PartyColors_p;
 uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
 for( int i = 0; i < NUM_LEDS; i++) { //9948
   leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
 }
}

void white() 
{
  for(int i =0; i < NUM_LEDS; i++){
      leds[i] = CRGB::White;
      delay(15);
      FastLED.show();
    }
    //continiously calls this.
}
void warmwhite() 
{
 for(int i =0; i < NUM_LEDS; i++){
     leds[i] = CRGB::FairyLight;
     delay(15);
     FastLED.show();
   }
   //continiously calls this.
}

void alternatewhite() 
{
 for(int i =0; i < NUM_LEDS; i++){
     leds[i] = CRGB(0xE1A019);
     delay(15);
     FastLED.show();
   }
   //continiously calls this.
}

//alexa mode only
void rbg_color(int CRGB_val)
{
    for(int i =0; i < NUM_LEDS; i++){
        leds[i] = CRGB(CRGB_val);
        delay(15);
        FastLED.show();
    }
}
