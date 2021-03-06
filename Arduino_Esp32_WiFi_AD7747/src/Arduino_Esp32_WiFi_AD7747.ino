

  //----------------------------------------------------------------------------------------
  //|     AD7747 Capacitive Sensor                                                         |
  //|   Based on code by  MiG found at:http://forum.arduino.cc/index.php/topic,11882.0.html|                                           |
  //|                                                                                      |
  //|  Author: Aidan Williamson  (advised by Paul Neelands)            |
  //|  Written: 7/3/2014                                                                   |
  //|  ad7747 datasheet:http://www.analog.com/static/imported-files/data_sheets/AD7747.pdf |
  //|                                                                                      |
  //|                                                                                      |
  //|                                                                                      |
  //|                                                                                      |
  //-----------------------------------------------------------------------------------------

  #if CONFIG_FREERTOS_UNICORE
  #define ARDUINO_RUNNING_CORE 0
  #else
  #define ARDUINO_RUNNING_CORE 1
  #endif


  #include "Arduino.h"
  #include <Hash.h>
  #include <WiFi.h>

  #include <WiFiMulti.h>
  #include <WebSocketsServer.h>
  WiFiMulti WiFiMulti;


  #include <Wire.h>                  //include the library for i2c communication
  #include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

  WebSocketsServer webSocket = WebSocketsServer(81);
  IPAddress ipClient= IPAddress(0,0,0,0);

  #define USE_SERIAL Serial


   //int capreadybit = (bitRead(TWDR,0));  //initialize variable readybit to be two wire data register's lsb

  SSD1306 display(0x3c, 5, 4);
  float value;
  char charBuf[16];

  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

      switch(type) {
          case WStype_DISCONNECTED:
              USE_SERIAL.printf("[%u] Disconnected!\n", num);
              ipClient = IPAddress(0,0,0,0);
              break;
          case WStype_CONNECTED:
              {
                  ipClient = webSocket.remoteIP(num);
                  USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ipClient[0], ipClient[1], ipClient[2], ipClient[3], payload);

  				// send message to client
  				webSocket.sendTXT(num, "Connected");
              }
              break;
          case WStype_TEXT:
              USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

              // send message to client
              // webSocket.sendTXT(num, "message here");

              // send data to all connected clients
              // webSocket.broadcastTXT("message here");
              break;
          case WStype_BIN:
              USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
  #ifdef ESP8266
              hexdump(payload, length);
  #endif

              // send message to client
              // webSocket.sendBIN(num, payload, length);
              break;
      }

  }

void i2cloop(void *pvParameters)
{
    display.init();
    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    Wire.begin(5,4,400000);                   //sets up i2c for operation
    Wire.beginTransmission(0x48);
    Wire.write(0xBF);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(4);
    Wire.beginTransmission(0x48);   // begins write cycle
    Wire.write(0x07);              //address pointer for cap setup register
    Wire.write(0xA0);              //b'10100000' found from datasheet page 16
    Wire.endTransmission();        //ends write cycle
    delay(4);                      // Wait for data to clock out? I'm not 100% sure why this delay is here (or why it's 4ms)

    //temperature compensation
    /*
    Wire.beginTransmission(0x48);   // begins write cycle
    Wire.write(0x08);              //address pointer for temp setup register
    Wire.write(B10000000);
    Wire.endTransmission();        //ends write cycle
    delay(4);                      // Wait for data to clock out? I'm not 100% sure why this delay is here (or why it's 4ms)
    */

    Wire.beginTransmission(0x48);   //begins transmission again
    Wire.write(0x09);              //address pointer for capacitive channel excitation register
    Wire.write(B00001111);
    //Wire.write(0x0E);              //recommended value from datasheet
    Wire.endTransmission();
    delay(4);

    Wire.beginTransmission(0x48);
    Wire.write(0x0A);              //address pointer for the configuration register
    //Wire.write(0x21);              //b'00100001' for continuous conversion, arbitrary VTF setting, and mid-range capacitive conversion time
    Wire.write(B01000001);              //b'00100001' for continuous conversion, arbitrary VTF setting, and mid-range capacitive conversion time
    Wire.endTransmission();
    delay(4);

    Wire.beginTransmission(0x48);
    Wire.write(0x0B);              //CAP DAC A Register address (Positive pin data)
    Wire.write(0x80);               //b'10111111' for enable Cap DAC A
    Wire.endTransmission();
    delay(4);

    while(1)
    {
      Wire.beginTransmission(0x48);   //talking to chip
      Wire.write(byte(0x00));                  //status register address
      Wire.endTransmission();
      Wire.requestFrom(0x48,1);       //request status register data
      int readycap;
      readycap=Wire.read();
      if((readycap&0x1)==0)
      {                // ready?
          Wire.beginTransmission(0x48); //arduino asks for data from ad7747
          Wire.write(0x01);             //set address point to capacitive DAC register 1
          Wire.endTransmission();       //pointer is set so now we can read the
          Wire.requestFrom(0x48,3);     //reads data from cap DAC registers 1-3
          while(Wire.available())
          {
            unsigned char hi,mid,lo;      //1 byte numbers
            long capacitance;            //will be a 3byte number
            float pf;                    //scaled value of capacitance
            hi=Wire.read();
            mid=Wire.read();
            lo=Wire.read();
            capacitance=(hi<<16)+(mid<<8)+lo-0x800000;
            pf=(float)capacitance*-1/(float)0x800000*8.192f;
            //Serial.print(pf, DEC); //prints the capacitance data in decimal through serial port

            dtostrf(pf, 12, 6, charBuf);
            display.clear();
            display.setFont(ArialMT_Plain_16);
            display.drawString(0, 0, charBuf);

            uint8_t progress = (int)((pf - (float)8) * (float)10000);
            display.drawProgressBar(0, 22, 120, 15, progress);
          }
      }
      display.setFont(ArialMT_Plain_10);
      String stringOut = "IP: " + WiFi.localIP().toString();
      display.drawString(0, 40, stringOut);

      String stringOut2 = "SSID: " + WiFi.SSID();
      display.drawString(0, 50, stringOut2);

      display.display();
    }
}


  void setup()
  {
    Serial.begin(115200);             // we will monitor this via serial cable

    xTaskCreatePinnedToCore(i2cloop, "i2cloop", 4096, NULL, 1, NULL, 0);

    Serial.println("Setup ended.");        //test to make sure serial connection is working

    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("linksys", "Doitman1");
    WiFiMulti.addAP("AndroidAP", "Doitman1");
    WiFiMulti.addAP("TP-LINK_7006", "klemendarja");
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(1000);
    }

    USE_SERIAL.print("[SETUP] IP: ");
    USE_SERIAL.println(WiFi.localIP());

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
  }


  void loop()
  {
    webSocket.loop();
    if(ipClient != IPAddress(0,0,0,0))
    {
      char result[25];
      sprintf( result, "Analogue %s", charBuf );
      webSocket.broadcastTXT(result);
    }
    //Serial.println("Loop Done");
    //Serial.println("  ");
    //delay(1000);
  }
