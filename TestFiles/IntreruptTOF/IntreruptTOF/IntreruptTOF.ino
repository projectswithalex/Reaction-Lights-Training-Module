#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <VL6180X_WE.h>
#define VL6180X_ADDRESS 0x29
#define LED 2
#define LEDNR 16
#define TOF_INT 12
#define SCL_PIN 5
#define SDA_PIN 4

VL6180xIdentification identification;
VL6180x TOFsensor(VL6180X_ADDRESS);

Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);
int i = 0;

volatile bool interruptReceived = false;

ICACHE_RAM_ATTR void handleInterrupt() {
  interruptReceived = true;
}




void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);  //Initialize I2C for VL6180x (TOF Sensor)
  pixels.begin();                // Initialize NeoPixel ring

  pixels.clear();
  pixels.show();

  pinMode(TOF_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOF_INT), handleInterrupt, FALLING);

  delay(1000);

  while(TOFsensor.VL6180xInit() == VL6180x_FAILURE_RESET){
    Serial.println("FAILED TO INITALIZE"); //Initialize device and check for errors

  }
  TOFsensor.VL6180xDefautSettings(); //Load default settings to get started.
  delay(1000);
    TOFsensor.VL6180xSetDistInt(10 ,10); 
    TOFsensor.getDistanceContinously();
}

void loop() {
  if (interruptReceived) {
    Serial.println("Intrerupt received");
    Serial.println(TOFsensor.getLastDistanceFromHistory());
    for (int i = 0; i < LEDNR; i++) {
      pixels.setPixelColor(i, pixels.Color(50, 0, 0));
      pixels.show();
      delay(50);
    }
    Serial.println("Light Set");
    interruptReceived = false;
    TOFsensor.VL6180xClearInterrupt();
    Serial.println("Intrerupt cleared");
  }

  for (int i = 16; i >= 0; i--) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
    
}
