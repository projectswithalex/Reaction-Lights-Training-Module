#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <VL6180X.h>
#define VL6180X_ADDRESS 0x29
#define LED 2
#define LEDNR 16
#define TOF_INT 12
#define SCL_PIN 5
#define SDA_PIN 4

VL6180X sensor;

Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);


void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);  //Initialize I2C for VL6180x (TOF Sensor)
  pixels.begin();                // Initialize NeoPixel ring

  pixels.clear();
  pixels.show();

  sensor.init();
  sensor.configureDefault();
  sensor.setScaling(0); //different Scalling values
  sensor.setTimeout(500);
}


void loop() {
  pixels.clear();
  uint16_t distanceVal = 0;
  uint8_t mapValue=0;

  distanceVal = sensor.readRangeSingleMillimeters();

   if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  mapValue = map(distanceVal, 10, 200, 0, 16);

  Serial.print("distanceVal: ");
  Serial.println(distanceVal);

  Serial.print("mapValue: ");
  Serial.println(mapValue);

  if (distanceVal < 10) {
    pixels.clear();
   
  } else {
    for (int i = 0; i < mapValue; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 150, 0));
      
    }
  }

  pixels.show();
  delay(50);
}