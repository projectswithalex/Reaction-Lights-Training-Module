#include <Adafruit_NeoPixel.h>

#define LED 2
#define LEDNR 16
Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);
#define BATMEAS A0      // Analog input pin
float calibration = 0.14;  // Check Battery voltage using multimeter & add/subtract the value
int bat_percentage;

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};



void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.clear();
  pixels.show();
}

void loop() {
  pixels.clear();
  pixels.show();

  //bat_percentage=10; //Test Value
  int analogVal = 0;
  analogVal = analogRead(BATMEAS);
  /*replaceValueHere*/ float voltage = (((analogVal * 3.3) / 1024) * 1.54);  //1.54 is the constant for me , check out with the multimeter and set the right value for you (trial&error) until correct
  bat_percentage = mapfloat(voltage, 3, 4.2, 0, 100);                        //Real Value

  if ((bat_percentage <= 100) && (bat_percentage >= 90)) {
    for (int i = 0; i < LEDNR; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
    }
  }

  if ((bat_percentage < 90) && (bat_percentage >= 60)) {
    for (int i = 0; i < 12; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
    }
  }

  if ((bat_percentage < 60) && (bat_percentage >= 40)) {
    for (int i = 0; i < 8; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
    }
  }

  if ((bat_percentage < 40) && (bat_percentage >= 20)) {
    for (int i = 0; i < 4; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
    }
  }

  Serial.print("Analog Value = ");
  Serial.print(analogVal);
  Serial.print("\t Output Voltage = ");
  Serial.print(voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(bat_percentage);
  delay(200);
}
