#include <Arduino.h>
#include <WiFi.h>
//#include <WProgram.h>
#include "fauxmoESP.h"
// #include "driver/uart.h"
#include "credentials.h"

//testing
//#define TEST_BROKEN_WIFI

//Device Names
#define NUM_DEVICES 4
#define DEVICE_NAME_1 "Strip"
#define DEVICE_NAME_2 "Rainbow"
#define DEVICE_NAME_3 "Juggle"
#define DEVICE_NAME_4 "BPM"

//Serial
#define SERIAL_BAUDRATE_MONITOR 38400
#define SERIAL_BAUDRATE_UART 38400   
#define RXD2 16
#define TXD2 17

//Commands
//Note: device ID's are encoded into the command.
#define BROKEN_WIFI "404"
#define SET_STRIP_RGB "rgb"
#define SET_ANIM_RAINBOW "rain"
#define SET_ANIM_JUGGLE "jug"
#define SET_ANIM_BPM "bpm"
#define ACK "Z"


//Other
#define WIFI_LOOP_DELAY 500
#ifdef TEST_BROKEN_WIFI
  #define TIMEOUT 20
#else
  #define TIMEOUT 80
#endif

//Pins
#define WIFI_EN_PIN 4

// globals
int gtimeout = TIMEOUT; //total delay time is TIMEOUT*WIFI_LOOP_DELAY, current: 40 seconds
bool gtimeoutflag = false; 
bool gwifi_prev_state = true;

bool gCallbackActionReq = true;
unsigned char gdevice_id;
char gdevice_name[100];
bool gstate;
unsigned char gvalue;
unsigned int ghue;
unsigned int gsaturation;
unsigned int gct;
uint8_t gRedLed;
uint8_t gGreenLed;
uint8_t gBlueLed;


//arduino buffer is 64 bytes. should include checks for this.
char buf[200];
char response[200];

int test_counter = 0;
bool send_success = false;
bool acked = false;

fauxmoESP fauxmo;


// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

    int counter = 0;

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    #ifdef TEST_BROKEN_WIFI    
        WiFi.begin(WIFI_SSID, "wrong");
    #else
        WiFi.begin(WIFI_SSID, WIFI_PASS);
    #endif

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(WIFI_LOOP_DELAY);
        Serial.print(".");
        if(++counter > gtimeout)
        {
            gtimeoutflag = true;
            break;
        }
    }
    Serial.println("");

    if(gtimeoutflag == false)
    {
        // Connected!
        Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    }
    else
    {
        Serial.println("Failed to connect to wifi.");
    }
}

void setup() 
{
    // Setup serial monitor
    Serial.begin(SERIAL_BAUDRATE_MONITOR);

    // Setup UART
    Serial2.begin(SERIAL_BAUDRATE_UART, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Setup UART comms success.");

    // setup digital pins
    pinMode(WIFI_EN_PIN, INPUT);

    // Wifi
    wifiSetup();
    
    String response_str;
    bool sent = false;
    while(gtimeoutflag == true)
    {
        
        write_buffer(BROKEN_WIFI);
        // Notify arduino of absence of wifi
        if(Serial2.availableForWrite() && !sent){
            Serial.println("Sending BROKEN_WIFI command");
            //length is in bytes, so number of indexes of the char array
            Serial2.write(buf, strlen(BROKEN_WIFI));
            sent = true;
        }

        // Confirm response
        if(Serial2.available() && sent){
            response_str = Serial2.readString();
            Serial.println("Message received:" + response_str);
            if(response_str.equals(BROKEN_WIFI))
            {
                //hang once confirmed. await reset or action.
                while(true);
            }
        }
        delay(400);
    }

    // Wifi setup okay, continue

    fauxmo.setPort(80); // This is required for gen3+ devices
    fauxmo.enable(true);
    fauxmo.addDevice(DEVICE_NAME_1); //device id 0
    fauxmo.addDevice(DEVICE_NAME_2); //device id 1
    fauxmo.addDevice(DEVICE_NAME_3); //device id 2
    fauxmo.addDevice(DEVICE_NAME_4); //device id 3
    



    //callback
    //device id, device name, state (light on or off), 
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value, unsigned int hue, unsigned int saturation, unsigned int ct) 
    {
        Serial.printf("--------\n[MAIN:%d] Device #%d (%s) state: %s value: %d hue: %u saturation: %u ct: %u\n", ++test_counter ,device_id, device_name, state ? "ON" : "OFF", value, hue, saturation, ct);
        
        gdevice_id = device_id;
        int strsize = strlen(device_name);
        for(int i =0 ; i < strsize ; i++)
        {
          gdevice_name[i] = device_name[i]; 
        }
        

        gstate = state;
        gvalue = value;
        ghue = hue;
        gsaturation = saturation;
        gct = ct;

        char colormode[3];
        fauxmo.getColormode(device_id, colormode, 3);
        Serial.printf("Colormode: %s\n", colormode);
        
        gRedLed = fauxmo.getRed(device_id);
        gGreenLed = fauxmo.getGreen(device_id);
        gBlueLed = fauxmo.getBlue(device_id);
        
        Serial.printf("HSV: %d %d %d  RGB: %d %d %d\n", hue, saturation, value, gRedLed, gGreenLed, gBlueLed);

        //if we get response, just terminate previous attempt and handle new one
        gCallbackActionReq = true;
        send_success = false;
        acked = false;
      
    });

}


void loop() {

  String response_str1 = "";

    //handle wifi enable
    if(digitalRead(WIFI_EN_PIN))
    {
        if(gwifi_prev_state == false){
            fauxmo.enable(true);
            gwifi_prev_state = true;
            Serial.println("Wifi re-enabled.");
        }
        fauxmo.handle();
    }
    else
    {
        gwifi_prev_state = false;
        fauxmo.enable(false);
    }

    //handle callback
    if(gCallbackActionReq)
    {
        turn_others_off(gdevice_id);
        if(send_success == false){
          
        
          switch(gdevice_id){
              //device: strip
              // looks ct, saturation, hue dont really matter
              case 0:
                  send_success = send_strip_command(SET_STRIP_RGB);
                  break;
              //device rainbow
              case 1:
                  send_success = send_anim_command(SET_ANIM_RAINBOW);
                  break;
              //device juggle
              case 2:
                  send_success = send_anim_command(SET_ANIM_JUGGLE);
                  break;
              //device bpm
              case 3:
                  send_success = send_anim_command(SET_ANIM_BPM);
                  break;
          }
        }

        if(send_success == true)
        {
            acked= false;
        }

        if(!acked && send_success)
        {
            // Confirm response
            if(Serial2.available()){
                response_str1 = Serial2.readString();
                Serial.println("here22: " + response_str1);

                Serial.print("before: ");
                for(int i =0 ; i < response_str1.length(); i++)
                {
                  Serial.print(response_str1[i], DEC);
                  Serial.print(" ");
                }
                Serial.println("");


                
                if(response_str1.indexOf(char(13)) < response_str1.length()  && response_str1.indexOf(char(13)) != -1 )
                {
                  response_str1 = response_str1.substring(0,response_str1.indexOf(char(13)));
                }
                Serial.print("after: ");
                for(int w=0; w < response_str1.length(); w++)
                {
                  Serial.print(response_str1.charAt(w),DEC);
                  Serial.print(" ");
                }
                Serial.println("");
                Serial.println("Message received:" + response_str1);
                if(response_str1.equals(ACK))
                {
                    acked == true;
                }
            }
        }

        if(send_success && acked)
        {
            gCallbackActionReq = false;
            send_success = false;
            acked = false;
        }

        
    }

}


bool send_anim_command(String command)
{
    //command (device id encoded), state
    unsigned int buffer_pos = 0;
    buffer_pos = write_buffer_idx(command, buffer_pos);
    buffer_pos= write_buffer_idx(gstate ? "1" : "0", buffer_pos);

    return uart_send(command, buffer_pos-1);

}


// the values of all the variables to send will be placed in global variables by the handler.
bool send_strip_command(String command)
{
    //command (device id encoded), state, value, (rval, gval, and bval concatenated)
    unsigned int buffer_pos = 0;
    buffer_pos = write_buffer_idx(command, buffer_pos);
    buffer_pos= write_buffer_idx(gstate ? "1" : "0", buffer_pos);
    if(gstate)
    {
        buffer_pos = write_buffer_idx(String(gvalue), buffer_pos);
        buffer_pos = write_buffer_idx(String(int(fauxmo.getRed(gdevice_id) << 16) | int(fauxmo.getGreen(gdevice_id) << 8) | int(fauxmo.getBlue(gdevice_id))), buffer_pos); 
    }

    return uart_send(command, buffer_pos-1);
}

bool uart_send(String command, unsigned int len)
{
    Serial.print("len: " + String(len) + ", here: ");
    for(int i =0 ; i < len; i++)
    {
      Serial.print(buf[i]);
    }
    Serial.println("");
    buf[len++] ='\n';
    
    Serial.print("len: " + String(len) + ", here: ");
    for(int i =0 ; i < len; i++)
    {
      Serial.print(buf[i]);
    }
    Serial.println("");
    
    if(Serial2.availableForWrite()){
        Serial.println("Sending " + command + " command");
        //length is in bytes, so number of indexes of the char array
        Serial2.write(buf, len);
        return true;
    }
    return false;
}

//returns the index of where a new string could begin. 
//index of where the null terminator is plus one
unsigned int write_buffer(String str)
{
    unsigned int len = str.length();
    for(int i =0; i < len; i++)
    {
        buf[i] = str[i];
    }
    buf[len] = '_';
    return len+1;
}

//returns the index of where a new string could begin. 
//index of where the null terminator is plus one
unsigned int write_buffer_idx(String str, unsigned int idx)
{
    
    unsigned int i, len = str.length();
    for(i = 0; i < len; i++)
    {
        buf[idx+i] = str.charAt(i);
    }
    buf[i+idx] = '_';
    return i+idx+1;
}

void turn_others_off(unsigned char id)
{
    for(int device = 0; device < NUM_DEVICES; device++)
    {
        if(device != id)
        {
            fauxmo.setState(device, 0, (unsigned char)(254)); //alexa doesnt like the value 255 for some reason.
        }
    }

}

// String get_device_name()
// {
//     String temp = "";
//     int i = 0;
//     while(gdevice_name[i] != '\0')
//     {
//         temp += gdevice_name[i++];
//     }
//     return temp;
// }


// If your device state is changed by any other means (MQTT, physical button,...) 
// you can instruct the library to report the new state to Alexa on next request:
// fauxmo.setState(ID_YELLOW, true, 255);

// You have to call enable(true) once you have a WiFi connection
// You can enable or disable the library at any moment
// Disabling it will prevent the devices from being discovered and switched
//fauxmo.enable(true);

// This is a sample code to output free heap every 5 seconds
// This is a cheap way to detect memory leaks
// static unsigned long last = millis();
// if ((millis() - last) > 5000) {
//     last = millis();
//     Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());

// }


// if((millis() - last3) > 2000){
//     last3 = millis();
//     Serial.println("message: "+ Serial2.readString());

//     if(Serial2.availableForWrite()){
//         Serial.println("sending");
//         Serial2.write(buff, sizeof(buff));
//     }
// }
