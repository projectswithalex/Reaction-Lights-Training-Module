#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h"
#define LED 2
#define LEDNR 16
#define SCL 5
#define SDA 4
Adafruit_VL6180X vl = Adafruit_VL6180X();
Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);
int i = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA,SCL); //Initialize I2C for VL6180x (TOF Sensor)
  
 delay(1000);
 Serial.println("Adafruit VL6180x test!");
  if (! vl.begin()) {
    Serial.println("Failed to find sensor");
    while (1);
  }
  Serial.println("Sensor found!");

  delay(1000);
  pixels.begin();  // Initialize NeoPixel ring
  pixels.clear();
  pixels.show();
}



void loop() {
  int test=0;
  pixels.clear();
  float lux = vl.readLux(VL6180X_ALS_GAIN_40);

  Serial.print("Lux: ");
  Serial.println(lux);

  uint8_t range = vl.readRange();
  uint8_t status = vl.readRangeStatus();

  Serial.print("range: ");
  Serial.println(range);

  Serial.print("status: ");
  Serial.println(status);

  test=map(range,10,200,0,16);
  if(range<10)
  {
    pixels.clear();
  }else
  {
  for (int i=0; i<test;i++)
  {
    
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
     

  }
  }
 
  pixels.show();  // Send the updated pixel colors to the hardware.

}
