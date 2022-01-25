#include <fauxmoESP.h>
#include "credentials.h"

fauxmoESP fauxmo;

void setup() {

    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

     // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }

    Serial.printf("Wifi connected, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

    fauxmo.addDevice("");


    fauxmo.setPort(80); // required for gen3 devices
    fauxmo.enable(true);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    });

}

void loop() {
    fauxmo.handle(); //this function call is not an infinite loop, you can run things after.
}


    // CAN DO WHATS BELOW OR YOU CAN DISABLE FAUXMO BY PASSING FALSE TO ENABLE. THIS WILL PREVENT ALEXA FROM CONTROLLING IT. MAYBE YOU CAN TIE A SWITCH TO THAT? NOT SURE WHATS BEST.

    // If your device state is changed by any other means (MQTT, physical button,...) 
    // you can instruct the library to report the new state to Alexa on next request:
    // fauxmo.setState(ID_YELLOW, true, 255);


    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    //fauxmo.enable(true);