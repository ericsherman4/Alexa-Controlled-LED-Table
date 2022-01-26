#include <FastLED.h>
#include <math.h>
#include <Wire.h>
#include <U8g2lib.h>


//////////////////////////////
//// FAST LED DEFINES ////////
//////////////////////////////


//FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    142
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         255
#define FRAMES_PER_SECOND  120

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


// List of patterns to cycle through.  Each is defined as a separate function below.
// typedef void (*SimplePatternList[])();
// NOT ALL OF THEM ARE CURRENTLY BEING USED.
// SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, white, warmwhite, alternatewhite};
// const int ARRAY_SIZE = sizeof(gPatterns) / sizeof((gPatterns)[0]);

//uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t state = 0;
bool firstrun = true;


//////////////////////////////
//// OLED DEFINES ////////////
//////////////////////////////

//https://github.com/olikraus/u8g2/blob/master/sys/arduino/u8g2_page_buffer/HelloWorld/HelloWorld.ino
//https://github.com/olikraus/u8g2/wiki/u8g2reference refernece manual
U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, PIN_A5, PIN_A4 , 4);




//////////////////////////////
//// CODE SETUP /////////////
/////////////////////////////


void setup() {
//    delay(1000); // 3 second delay for recovery
    Serial.begin(115200);

    u8g2.begin();  


    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(10, INPUT);
    pinMode(11, INPUT);

      u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x12_mr);//actual pixel hieght is about 10. i think 15 is about the start of the blue section
    u8g2.drawStr(0,12,"Hello World!");
  } while ( u8g2.nextPage() );
  delay(1000);
    
}

//////////////////////////////
//// MAIN LOOP //////////////
/////////////////////////////


String thing = "";

void loop()
{

  // send data only when you receive data:
  while (Serial.available() > 0) {
    // read the incoming byte:

    thing += Serial.readString();

  }
//  Serial.println(String(thing.length()));
//  for(int i =0; i < thing.length(); i++)
//  {
////    Serial.print(thing[i]);
//    if(thing[i] == char(13) ||  thing[i] == char(10) || thing[i] == char(0))
//    {
//      thing[i] = '_';
//    }
//  }
  
  if(!thing.equals("")){
    Serial.println(thing);
  }
  thing="";
  delay(500);



    // Call the current pattern function once, updating the 'leds' array
//    Serial.println("in main loop");
    //gPatterns[gCurrentPatternNumber](); //having this line even though its not called breaks the OLED lib, even tho its never called!!
  
    switch(state){
        case 0:
            rainbow();
            u8g2.firstPage();
          do{  u8g2.setCursor(0,24);
             u8g2.drawStr(0,24,("rainbox"));
          }while(u8g2.nextPage());
            break;
        case 1:
            juggle();
           u8g2.firstPage();
          do{  
             u8g2.drawStr(0,24,("juggle"));
          }while(u8g2.nextPage());
            break;
        case 2:
            bpm();
             u8g2.firstPage();
          do{  
             u8g2.drawStr(0,24,("bpm"));
          }while(u8g2.nextPage());
            break;
        case 3:
            white();
             u8g2.firstPage();
          do{  
             u8g2.drawStr(0,24,"white");
          }while(u8g2.nextPage());
            break;
        case 4:
            warmwhite();
            u8g2.firstPage();
          do{  
             u8g2.drawStr(0,24,("warmwhite"));
          }while(u8g2.nextPage());
            break;
        case 5:
            alternatewhite();
            u8g2.firstPage();
          do{  
             u8g2.drawStr(0,24,("alternatewhite"));
          }while(u8g2.nextPage());
            break;
          

    }


  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
//  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

  state = digitalRead(11)*pow(2,3)+digitalRead(10)*pow(2,2)+digitalRead(9)*2+digitalRead(8);

  if((state == 3 || state== 4 || state == 5) && firstrun){
      for(int i =0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Black;
        FastLED.show();
      }
      firstrun=false;
//      Serial.println("execute1");
  }

  if(state != 3 && state != 4 && state != 5){
    firstrun = true;
//    Serial.println("exec2");
  }

  
}

/////////////////////////////
//// COMMS //////////////////
/////////////////////////////

void debug_ascii(String thing)
{
    for(int w=0; w < thing.length(); w++)  
    {
        Serial.print(thing.charAt(w),DEC);
        Serial.print(" ");
    }
    Serial.println("");

}




//////////////////////////////
//// ANIMATIONS / STATES ////
/////////////////////////////

void rainbow() //0
{
 // FastLED's built-in rainbow generator
 fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() //1
{
 // built-in FastLED rainbow, plus some random sparkly glitter
 rainbow();
 addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) //support function for 1
{
 if( random8() < chanceOfGlitter) {
   leds[ random16(NUM_LEDS) ] += CRGB::White;
 }
}

void confetti() //2
{
 // random colored speckles that blink in and fade smoothly
 fadeToBlackBy( leds, NUM_LEDS, 10);
 int pos = random16(NUM_LEDS);
 leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon() //3
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void juggle() //4
{
 // eight colored dots, weaving in and out of sync with each other
 fadeToBlackBy( leds, NUM_LEDS, 20);
 byte dothue = 0;
 for( int i = 0; i < 8; i++) {
   leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
   dothue += 32;
 }
}

void bpm() //5
{
 // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
 uint8_t BeatsPerMinute = 62;
 CRGBPalette16 palette = PartyColors_p;
 uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
 for( int i = 0; i < NUM_LEDS; i++) { //9948
   leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
 }
}


void white() //6
{
  for(int i =0; i < NUM_LEDS; i++){
      leds[i] = CRGB::White;
      delay(15);
      FastLED.show();
    }
    //continiously calls this.
}

void warmwhite() //7
{
 for(int i =0; i < NUM_LEDS; i++){
     leds[i] = CRGB::FairyLight;
     delay(15);
     FastLED.show();
   }
   //continiously calls this.
}

void alternatewhite() //8
{
 for(int i =0; i < NUM_LEDS; i++){
     leds[i] = CRGB(0xE1A019);
     delay(15);
     FastLED.show();
   }
   //continiously calls this.
}
