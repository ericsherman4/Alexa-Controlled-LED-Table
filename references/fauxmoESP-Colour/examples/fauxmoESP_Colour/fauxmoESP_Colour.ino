#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif

#include "fauxmoESP.h"
#include "driver/uart.h"

// Rename the credentials.sample.h file to credentials.h and 
// edit it according to your router configuration
#include "credentials.h"
// or alternately set SSID and passphrase below
//#define WIFI_SSID "----"
//#define WIFI_PASS "----"

fauxmoESP fauxmo;

#define SERIAL_BAUDRATE     115200

#define DEVICE_NAME "LED Test"

// Set RGB pins
#define REDPIN 21
#define GREENPIN 22
#define BLUEPIN 23
// LED channels
#define REDC 1
#define GREENC 2
#define BLUEC 3

//UART
#define RXD2 16
#define TXD2 17





// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}



// HSV-RGB stuff 
uint8_t redLed;
uint8_t greenLed;
uint8_t blueLed;


void setup() 
{
    Serial.begin(SERIAL_BAUDRATE);

    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    Serial.println("sucess setup uart");

    // Wifi
    wifiSetup();

    fauxmo.setPort(80); // This is required for gen3 devices
    fauxmo.enable(true);
    fauxmo.addDevice(DEVICE_NAME);  
    fauxmo.addDevice("static thing");

    // LED strip testing - ledc only works for ESP32
//#ifdef ESP32
//	    ledcAttachPin(REDPIN, REDC);
//	    ledcAttachPin(GREENPIN, GREENC);
//	    ledcAttachPin(BLUEPIN, BLUEC);
//	    ledcSetup(REDC, 12000, 8);  // 12kHz 8 bit
//	    ledcSetup(GREENC, 12000, 8);
//	    ledcSetup(BLUEC, 12000, 8);
//#endif

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value, unsigned int hue, unsigned int saturation, unsigned int ct) 
    {
      Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d hue: %u saturation: %u ct: %u\n", device_id, device_name, state ? "ON" : "OFF", value, hue, saturation, ct);

      char colormode[3];
      fauxmo.getColormode(device_id, colormode, 3);
      Serial.printf("Colormode: %s\n", colormode);
      
      redLed = fauxmo.getRed(device_id);
      greenLed = fauxmo.getGreen(device_id);
      blueLed = fauxmo.getBlue(device_id);
      
      Serial.printf("HSV: %d %d %d  RGB: %d %d %d\n", hue, saturation, value, redLed, greenLed, blueLed);
      
//#ifdef ESP32
//      if (state)
//      {
//        ledcWrite(REDC, redLed);
//        ledcWrite(GREENC, greenLed);
//        ledcWrite(BLUEC, blueLed);
//      }
//      else
//      {
//        
//        ledcWrite(REDC, 0);
//        ledcWrite(GREENC, 0);
//        ledcWrite(BLUEC, 0);
//      }
//#endif
    });

    
}

char  buff[] = "helloesp";
char response[100];
unsigned long last3 = millis();

void loop() {
    fauxmo.handle();

    // This is a sample code to output free heap every 5 seconds
    // This is a cheap way to detect memory leaks
    static unsigned long last = millis();
//    Serial.println(last);
    if ((millis() - last) > 5000) {
        last = millis();
        Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
        
    }

//references for the uart
//https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.h
//https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

    if((millis() - last3) > 2000){
      last3 = millis();
          //      Sieral.println("entered");
//      Serial.println("Data Received");
//      Serial2.readString().toCharArray(response, 100);
      Serial.println("message: "+Serial2.readString());
//      if(response != ""){
//             Serial.println(Serial2.readString()); 
//      }
      
        if(Serial2.availableForWrite()){
          Serial.println("sending");
          Serial2.write(buff, sizeof(buff));
    }
    }

    



  
}
