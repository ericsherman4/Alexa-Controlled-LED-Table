# Alexa-Controlled-LED-Table
This is for enabling my led table to be controlled by alexa. Goal is for it to be able to set certain animations based on voice commands to my alexa.

## Hardware
ESP32 from Amazon [https://www.amazon.com/gp/product/B09J94HPZB/]
- The board name in the Arduino IDE is ESP32 Wrover Module
- You also need to download the CP2102 driver. I got the download from Polulu (https://www.pololu.com/docs/0J7/all) but there are other sources.
- You also need to add the ESP32 device to arduino IDE: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

Arduino Uno R3, using Fast LED library for the LED strip animations [https://www.amazon.com/ELEGOO-Board-ATmega328P-ATMEGA16U2-Compliant/dp/B01EWOE0UU/]

LED strip, w/ WS2812B controller: https://www.amazon.com/gp/product/B01LSF4Q0A/

## Alexa <> ESP Comms
Credit to https://github.com/vintlabs/fauxmoESP for their examples and ESP <> Alexa interface. 
Their code is included in this repo for personal reference but all credit and respects should go to respective authors.

## Exploration on ways for ESP to talk to Alexa
1. Method one
   - https://github.com/makeratplay/AlexaVoiceControl
   - https://www.youtube.com/watch?v=11NMUUlDJmw
   - https://www.youtube.com/watch?v=7T4hS5EFB_I
   - https://www.youtube.com/watch?v=_AaS32MWL8E&t=0s

2. Method two:
   - https://github.com/Aircoookie/Espalexa

3. Method three:
   - https://github.com/vintlabs/fauxmoESP <--- Selected Method
     - Note: I'm using the Colour branch (not master) in my application. 
     - URL: https://github.com/vintlabs/fauxmoESP/tree/Colour

## ESP32 Documentation
Pinout: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

## Integration Plan
Set up one virtual device for doing all the standard colors on the table. This probably means that we could probably get rid of some of the modes like white or warm right. 
Should test it out to see if it's a good color or not. 

Set up the rest of the virtual devices should just be used as on/off. Make it so setting different colors or brightness has no impact. These will be tied directly to the animations.

ESP32 <> Arduino will be UART (granted the noise isn't too bad.) Send data to Arduino. 
however UART is very blocking and breaks the animations pretty bad. 
Could do like whenever I get a request, stop the animation and handle the request and switch the animation / color immediately. At this point also update the display.

Turning off/on Alexa controlled mode and letting arduino be controlled directly: 
One toggle switch sends a note to both? I think that's the best most sure fire way. Be careful because the ESP32 pins ARE NOT 5V tolerant. 
I think that the Arduino would definitely recognize 3.3V as a one though.
In the Arduino code, using the bool from that switch, determine whether to handle I2C calls or just handle the toggle switches. 