#include <Arduino.h>

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "esp32-hal.h"

// board https://www.tindie.com/products/lspoplove/d-duino-32esp32-and-096oled-display/
// graphic arduino lib for oled: https://github.com/squix78/esp8266-oled-ssd1306
// oled 0x3c device is connected via i2c through pins: pin 4 (CLK) and pin 5 (DATA)

//SSD1306 display(0x3c, SDA, CLK);
// Pin 5 is SDA, Pin 4 is SCL
SSD1306 display(0x3c, 5, 4);
int8_t pixelYs[100];
int8_t previousY;

uint8_t analogPin = 25;
float value;
float deltaValue=0.03;

float midVolt = 2.9;
float deltaVolt = 0.3;

float voltage;
char charBuf[12];

int8_t chartHeight = 45;

uint8_t x_offset = 10;
uint8_t y_offset = 60;

long loop_i = 0;

static inline int8_t sgn(float val) {
 if (val < 0) return -1;
 if (val==0) return 0;
 return 1;
}

//init ad7747
void Init_AD7747()
{
  // http://opensourceecology.orglw/images/eJec/Cap_Sensor EmailChain_7-11-2014.pdf
  // http://opensourceecology.org/wiki/PaulLog

  // Pin 2 is SDA, Pin 14 is SCL
  // Pin 5 is SDA, Pin 4 is SCL
  Serial.println("Init_AD7747");
  Wire.begin(5,4); //
  Wire.setClock(400000); // choose 400 kHz I2C rate

  Serial.println("1");
  Wire.beginTransmission(0x48);
  Wire.write(0xBF);
  Wire.write(0x00);
  Wire.endTransmission();
  Serial.println("2");
  delay(4);
  Wire.beginTransmission(0x48); // begins write cycle
  Wire.write(0x07); //address pointer for cap setup register Wire.
  Wire.write(0xA0); //b'10100000' found from datasheet page 16
  Wire.endTransmission(); //ends write cycle
  Serial.println("3");
  delay(4); // Wait for data to clock out? Im not 100% sure why this delay is here (or why its 4ms)
  Wire.beginTransmission(0x48); //begins transmission again
  Wire.write(0x09); //address pointer for capacitive channel excitation register
  Wire.write(0x0E); //recommended value from datasheet
  Wire.endTransmission();
  Serial.println("4");
  delay(4);
  Wire.beginTransmission(0x48);
  Wire.write(0x0A); //address pointer for the configuration register
  Wire.write(0x21); //b'00100001' for continuous conversion, arbitrary VTF setting, and mid-range capacitive conversion time
  Wire.endTransmission();
  Wire.beginTransmission(0x48);
  Wire.write(0x0B); //CAP DAC A Register address (Positive pin data)
  Wire.write(0x80); //b'10111111' for enable Cap DAC A
  Wire.endTransmission();
  Serial.println("Init_AD7747...Done.");
}

void setup() {

	Serial.begin(115200);

  Init_AD7747();

  //DISPLAY INIT
  Serial.println("Display init...");
  display.init();
	display.flipScreenVertically();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
  display.setLogBuffer(5,20);
  Serial.println("Display init...Done.");

}

void readAD7747_loop()
{
  Serial.print("LOOP:");
  Serial.println(loop_i);
  Wire.beginTransmission(0x48); //talking to chip
  Wire.write(byte(0x00)); //status register address
  Wire.endTransmission();
  Wire.requestFrom(0x48, 1); //request status register data
  int readycap;
  //Serial.println("Tryng read..."); //try read
  readycap = Wire.read();
  if ((readycap & 0x1) == 0) // ready?
  {
    Serial.print(millis());
    Serial.println("Data Ready");
    delay(10);
    Wire.beginTransmission(0x48); //arduino asks for data from ad7747
    Wire.write(0x01); //set address point to capacitive DAC register 1
    Wire.endTransmission(); //pointer is set so non we can read the

    Serial.println("Data Incoming");
    delay(10);
    Wire.requestFrom(0x48, 3, false); //reads data from cap DAC registers 1-3
    while (Wire.available())
    {
      //Serial.print(system_get_time());
      //Serial.println("Wire available.");
      unsigned char hi, mid, lo; //1 byte numbers
      long capacitance; //wil be a 3byte number
      float pf; //scaled value of capackance
      hi = Wire.read();
      mid = Wire.read();
      lo = Wire.read();
      capacitance = (hi << 16) + (mid << 8) + lo - 0x800000;
      pf = (float) capacitance * -1 / (float) 0x800000 * 8.192f;
      Serial.print(millis());
      Serial.print(" ");
      dtostrf(voltage, 5, 2, charBuf);
      Serial.println(voltage); //prints the capacitance data in decimal through serial port
    }
  }

}

void loop()
{
  loop_i = loop_i + 1;
  readAD7747_loop();

  //display.clear();
  //dtostrf(loop_i, 5, 2, charBuf);
  //display.drawString(0, 0, charBuf);
  //display.setPixel(0, 0);
  //display.print(loop_i);
  //display.display();

/*
  voltage = 0.1234;
	dtostrf(voltage, 5, 2, charBuf);
	display.clear();
	display.drawString(0, 0, charBuf);
	display.display();
*/
}
