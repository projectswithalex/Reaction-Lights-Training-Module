 #include <Adafruit_NeoPixel.h>

#define LED 2
#define LEDNR 16
 Adafruit_NeoPixel pixels(LEDNR, LED, NEO_GRB + NEO_KHZ800);
int analogInPin  = A0;    // Analog input pin
int sensorValue;          // Analog Output of Sensor
float calibration = 0.14; // Check Battery voltage using multimeter & add/subtract the value
int bat_percentage;



 
void setup()
{
  Serial.begin(115200);
  pixels.begin();                
  pixels.clear();
  pixels.show();
}
 
void loop()
{
  pixels.clear();
  pixels.show();
  
 
  

  //bat_percentage=10; //Test Value

  

  if((bat_percentage<=100) && (bat_percentage>=90))
  {
    for (int i = 0; i < LEDNR; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      
    }
  }

  if((bat_percentage<90) && (bat_percentage>=60))
  {
    for (int i = 0; i < 12; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      
    }
  }

  if((bat_percentage<60) && (bat_percentage>=40))
  {
    for (int i = 0; i < 8; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      
    }
  }

  if((bat_percentage<40) && (bat_percentage>=20))
  {
    for (int i = 0; i < 4; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      
    }
  }

   
  }

 
  Serial.print("Analog Value = ");
  Serial.print(sensorValue);
  Serial.print("\t Output Voltage = ");
  Serial.print(voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(bat_percentage);
  delay(200);
 
  
 

}
 
