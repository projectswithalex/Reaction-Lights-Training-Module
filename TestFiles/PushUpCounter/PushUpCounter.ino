#include <Wire.h>
#include <VL6180X.h>
#include <Adafruit_NeoPixel.h>

#define LED 2
#define LEDNR 16
#define SCL 5
#define SDA 4
Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);
#define SCALING 2

VL6180X sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);

  sensor.init();
  sensor.configureDefault();
  sensor.setScaling(SCALING);
  sensor.setTimeout(500);

  pixels.begin();  // Initialize NeoPixel ring
  pixels.clear();
  pixels.show();
}
int pushUpCounter = 0;
int flagDown = 0;
int flagUp = 0;
int newSessionCounter = 0;
void loop() {
  int distanceVal;
  if (pushUpCounter < 10) {
    distanceVal = sensor.readRangeSingleMillimeters();

    if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

    Serial.println();

    delay(100);

    if ((distanceVal < 500) && (flagDown == 0)) {
      Serial.println(distanceVal);
      Serial.println("DOWN");

      flagDown = 1;
    }

    if ((flagDown == 1) && (flagUp == 0)) {
      Serial.println("Light one up");
      pushUpCounter++;
      Serial.print("pushUpCounter:");
      Serial.println(pushUpCounter);
      flagUp = 1;
      for (int i = 0; i < pushUpCounter; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));
        pixels.show();
      }
    }

    if (distanceVal > 500 && (flagUp == 1)) {
      Serial.println(distanceVal);
      Serial.println("UP");
      flagUp = 0;
      flagDown = 0;
    }
  } else {
    for (int i = 0; i < LEDNR; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 150));
      pixels.show();
    }
    delay(100);
    pixels.clear();
    pixels.show();
    for (int i = 0; i < LEDNR; i++) {
      pixels.setPixelColor(i, pixels.Color(150, 0, 0));
      pixels.show();
    }
    delay(100);
    pixels.clear();
    pixels.show();
    for (int i = 0; i < LEDNR; i++) {
      pixels.setPixelColor(i, pixels.Color(100, 100, 100));
      pixels.show();
    }
    delay(100);
    pixels.clear();
    pixels.show();
    distanceVal = sensor.readRangeSingleMillimeters();
    if (distanceVal < 100) {
      newSessionCounter++;
    }
    if (newSessionCounter > 10) {
      newSessionCounter = 0;
      pushUpCounter = 0;
      pixels.clear();
      pixels.show();
      delay(100);
      for (int i = 0; i < LEDNR; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 100, 0));
        pixels.show();
        delay(50);
      }
      delay(1000);
      pixels.clear();
      pixels.show();
    }
  }
}
