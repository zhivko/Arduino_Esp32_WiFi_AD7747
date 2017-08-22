

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
  #include <Wire.h>                  //include the library for i2c communication
  #include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

   //int capreadybit = (bitRead(TWDR,0));  //initialize variable readybit to be two wire data register's lsb

  SSD1306 display(0x3c, 5, 4);
  float value;
  char charBuf[16];

  void setup()
  {
    Serial.begin(115200);             // we will monitor this via serial cable

    display.init();
  	display.flipScreenVertically();
  	display.setTextAlignment(TEXT_ALIGN_LEFT);
  	display.setFont(ArialMT_Plain_16);

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
    Wire.beginTransmission(0x48);   //begins transmission again
    Wire.write(0x09);              //address pointer for capacitive channel excitation register
    Wire.write(0x0E);              //recommended value from datasheet
    Wire.endTransmission();
    delay(4);
    Wire.beginTransmission(0x48);
    Wire.write(0x0A);              //address pointer for the configuration register
    Wire.write(0x21);              //b'00100001' for continuous conversion, arbitrary VTF setting, and mid-range capacitive conversion time
    Wire.endTransmission();
    Wire.beginTransmission(0x48);
    Wire.write(0x0B);              //CAP DAC A Register address (Positive pin data)
    Wire.write(0x80);               //b'10111111' for enable Cap DAC A
    Wire.endTransmission();
    Serial.println("Setup ended.");        //test to make sure serial connection is working
  }


  void loop()
{
    Wire.beginTransmission(0x48);   //talking to chip
    Wire.write(byte(0x00));                  //status register address
    Wire.endTransmission();
    Wire.requestFrom(0x48,1);       //request status register data
    int readycap;
    readycap=Wire.read();
    if((readycap&0x1)==0){                // ready?
      //Serial.println("Data Ready");
      //delay(500);
      delay(5);
      Wire.beginTransmission(0x48); //arduino asks for data from ad7747
      Wire.write(0x01);             //set address point to capacitive DAC register 1

      Wire.endTransmission();       //pointer is set so now we can read the

      //Serial.println("Data Incoming");
      //delay(500);
      delay(5);
      Wire.requestFrom(0x48,3);     //reads data from cap DAC registers 1-3

      while(Wire.available()){
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
        display.drawString(0, 0, charBuf);
        display.display();
     }
     Serial.println();
  }

  //Serial.println("Loop Done");
  //Serial.println("  ");
  //delay(1000);
}
