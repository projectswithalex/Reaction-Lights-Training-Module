#include <Adafruit_NeoPixel.h>
#define LED 2
#define LEDNR 16
#define SELBUTTON 14

Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);
unsigned long debounceDelay = 200;
unsigned long longDebounce = 3000;
volatile unsigned long lastDebounce = 0;
volatile uint8_t interruptModeSelection = 0;
volatile bool isbuttonpressed = 0;

ICACHE_RAM_ATTR void handleInterruptSEL() {
  lastDebounce = millis();
  isbuttonpressed = true;
}



bool buttonPushValid(void) {
  //unsigned long start = millis();
  if (isbuttonpressed) {
    if (millis() - lastDebounce > debounceDelay) {
      interruptModeSelection += 10;
      isbuttonpressed = false;
      lastDebounce = millis();
      return true;
    }
  }
  return false;
}


void setup() {
  Serial.begin(115200);

  pinMode(SELBUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(SELBUTTON), handleInterruptSEL, FALLING);

  pixels.begin();  // Initialize NeoPixel ring
  pixels.clear();
  pixels.show();
}

void loop() {


  uint8_t buttonState = digitalRead(SELBUTTON);
  Serial.print("ButtonState:");
  Serial.println(buttonState);

  Serial.print("buttonSelection:");
  Serial.println(interruptModeSelection);

  buttonPushValid();

  for (int i = 0; i < LEDNR; i++) {
    pixels.setPixelColor(i, pixels.Color(0, interruptModeSelection, 0));
    pixels.show();  // Send the updated pixel colors to the hardware.
  }

  if (interruptModeSelection > 200) {
    pixels.clear();
    pixels.show();
    interruptModeSelection = 0;
  }


}
