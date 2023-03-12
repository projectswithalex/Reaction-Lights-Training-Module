/*

 Project Name Reaction Lights Training Module Project.
 Developed by : projectswithalex.
 Email :projectswithalex@gmail.com
 Github : https://github.com/projectswithalex/Reaction-Lights-Training-Module
 Instragram :https://www.instagram.com/projectswithalex/
 
 Disclaimer : Code is opensource and can be modified by everyone. If you can improve the code and add new functionality to it i would be happy to get
              in touch with you and update the repository. Thanks for being here and wish you a good training session with this modules.

*/


#include <Adafruit_NeoPixel.h>
#include<ESP8266WiFi.h>
#include<espnow.h>
#include <Wire.h>
#include <VL6180X_WE.h> //This library is modified.

/********************************ESP NOW COMMUNICATION CODE ******************************/
#define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
#define MY_ECU          2                               //ECU number
#define WIFI_CHANNEL    1
#define TRANSMISIONSUCCESFUL   2                        //Transmision was succesful  
#define SENDDATA               1                        //Transmision stopped
#define DATARECEIVED           0                        // Data has been received
#define ONLYRECEIVE            3                        //Waiting for Data
#define MACADDRESSSIZE         6                        //Mac address size
#define NOECU                  0                        //No ecu with the define MY_ECU 0
#define RGBCLEARDELAY          100                      //delay to be used with RGB clear ?TBD
//#define MY_NAME         "SENSOR NODE"
//uint8_t receiverAddress1[] = {0x3C, 0x61, 0x05, 0xE4, 0x8C, 0x7B};       // ECU 1
#define MY_NAME         "Master NODE"
uint8_t receiverAddress1[] = {0xC8, 0xC9, 0xA3, 0x5D, 0x87, 0x2A};   //  ECU 1
uint8_t receiverAddress3[] = {0xC8, 0xC9, 0xA3, 0x5D, 0x91, 0x8E};   //  ECU 3


uint8_t receiverECU_Address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //Placeholder for the receiver address

struct __attribute__((packed)) dataPacket {
  int LED_Token;  // Token for activating ECUs
  int Mode;       // Training Mode
  struct partnerMode { 
        int partEcu1;             
        int partEcu2;
        int partEcu3;
        int partEcu4;
  };
};

dataPacket packet = {1, 0 }; //Package of data to be sent
int TransmisionStatus = ONLYRECEIVE; //Transmision Status
const int numberOfECU = 3; //Number of ECUs in the network

/********************************ESP NOW COMMUNICATION CODE ******************************/

/******************************** RGB LED CODE  ******************************/
//RGB Defines
#define RGBDATAPIN  2
#define RGBLEDNUM 16
#define NUMBEROFCOLORS 7
Adafruit_NeoPixel pixels(RGBLEDNUM, RGBDATAPIN, NEO_GRB + NEO_KHZ800);

//                                      RED   Green  Blue
const int colors[NUMBEROFCOLORS][3] = { { 150, 0, 0 },
                                        { 0, 150, 0 },
                                        { 0, 0, 150 },
                                        { 150, 0, 0 },
                                        { 0, 150, 150 },
                                        { 150, 150, 0 },
                                        { 150, 0, 150 } };

int selectColor=0;



//Select color next ON cycle
void selectColorNextCycle(void)
{
  selectColor++;
  if(NUMBEROFCOLORS>selectColor)
  {
    selectColor=0; 
  }
  
}

void setRGBcolors (int colorIndex)
{
  
  for(int i=0; i<RGBLEDNUM; i++) { // For each pixel...
      
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(colors[colorIndex][0],colors[colorIndex][1], colors[colorIndex][2]));
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
}

void clearRGBcolors ()
{
  //Serial.println("Clear RGB");
  for(int i=0; i<RGBLEDNUM; i++) { // For each pixel...
      
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
}

unsigned long Time = 0;           // the last time the output pin was toggled
unsigned long debounce = 20UL;   // the debounce time, increase if the output flickers
unsigned long randomNumber = 3;
unsigned long randomECUSelection = 0;


/******************************** RGB LED CODE  ******************************/

/******************************** BATTERY CHECK CODE  ******************************/
//Battery Check Defines
#define BATMEAS A0                    // Analog input pin
int analogVal=0;                      // Analog Output of Sensor
int bat_percentage=0;                         // Battery in percentage
float calibrationValue=0.14;           // Check Battery voltage using multimeter & add/subtract the value

void readBatValue(void)
{
  analogVal = analogRead(BATMEAS);
  float voltage = (((analogVal * 3.3) / 1024)  * 1.54 ); 
  bat_percentage = mapfloat(voltage, 3, 4.2, 0, 100);           //Real Value

  if (bat_percentage >= 100)
  {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0)
  {
    bat_percentage = 1;
  }
};

void showBatteryPercentage(void)
{
  if((bat_percentage<=100) && (bat_percentage>=90)) //Battery Levels between 100%-90% = 16LEDS green
  {
    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
      
    }
  }

  if((bat_percentage<90) && (bat_percentage>=60)) //Battery Levels between 89%-60% = 12LEDS green
  {
    for (int i = 0; i < 12; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
      
    }
  }

  if((bat_percentage<60) && (bat_percentage>=40)) //Battery Levels between 59%-40% = 8LEDS green
  {
    for (int i = 0; i < 8; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
      
    }
  }

  if((bat_percentage<40) && (bat_percentage>=20)) //Battery Levels between 39%-20% = 4LEDS green
  {
    for (int i = 0; i < 4; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if((bat_percentage<20) && (bat_percentage>=10)) //Battery Levels between 19%-10% = 4LEDS flashing RED leds
  {

    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(20, 0, 0));
      pixels.show();
      i=i+3;
    }
    delay(200);
     pixels.clear();
     pixels.show();
     delay(200);
    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(20, 0, 0));
      pixels.show();
      i=i+3;
    }
  }
  // Lower then 10 percent nothing is displayed. As a safety measure since i don't know how much trust i have in the "BMS" that is going to
  // protect the battery of over-discharge.
};

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};


/******************************** BATTERY CHECK CODE  ******************************/

/******************************** TOF SENSOR CODE  ******************************/
#define VL6180X_ADDRESS 0x29
#define TOF_INT 12
#define SCL_PIN 5
#define SDA_PIN 4

VL6180xIdentification identification;
VL6180x TOFsensor(VL6180X_ADDRESS);

volatile bool interruptReceived = false;
volatile bool flag = false;
ICACHE_RAM_ATTR void handleInterrupt() {
  interruptReceived = true;
  flag=true;
}

/******************************** TOF SENSOR CODE  ******************************/





void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    TransmisionStatus = TRANSMISIONSUCCESFUL;
  } else {
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

  Serial.println();
  Serial.print("Received data from: ");
  Serial.println(macStr);

  memcpy(&packet, data, sizeof(packet));
  TransmisionStatus = DATARECEIVED;

}
/******************************** Logic CODE  ******************************/


void randomECUselect(void)
{
  if(randomNumber==1)
  {
    randomNumber=3;
  }else
  {
    randomNumber=1;
  }
}


//Random ECU Selection for message transmision
bool isECUselectionValid() {

  randomECUSelection = randomNumber ;
  if ((randomECUSelection != NOECU) && (randomECUSelection != MY_ECU))
  {
    Serial.print(" ecu valid");
    return true;
  }
  else
  {
    return false;
  }
}
void selectECU_number(int ECU)
{
  
  switch (ECU) {

    case 1:
      memcpy(&receiverECU_Address, receiverAddress1, MACADDRESSSIZE);
      packet.LED_Token = ECU;
      TransmisionStatus=SENDDATA;
      break;

    case 3:
      memcpy(&receiverECU_Address, receiverAddress3, MACADDRESSSIZE);
      packet.LED_Token = ECU;
      TransmisionStatus=SENDDATA;
      break;
    default:
      Serial.println("SelectECU_Failed");
      break;
  }
  /******************************** Logic CODE  ******************************/

}

void setup() {
  Serial.begin(115200);     // initialize serial port
  Wire.begin(SDA_PIN, SCL_PIN);  //Initialize I2C for VL6180x (TOF Sensor)
  pinMode(A0, INPUT);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(MY_ROLE);
  esp_now_register_send_cb(transmissionComplete);         // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);               // this function will get called whenever we receive data
  esp_now_add_peer(receiverAddress1, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(receiverAddress3, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

  pinMode(TOF_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOF_INT), handleInterrupt, FALLING);
  delay(1000);
  while(TOFsensor.VL6180xInit() == VL6180x_FAILURE_RESET){
    Serial.println("FAILED TO INITALIZE"); //Initialize device and check for errors

  }
  TOFsensor.VL6180xDefautSettings(); //Load default settings to get started.
  delay(1000);
  TOFsensor.VL6180xSetDistInt(20 ,20); 
   TOFsensor.getDistanceContinously();
  TOFsensor.VL6180xClearInterrupt();
  interruptReceived = false;
  delay(500);

  readBatValue();
  showBatteryPercentage();
  delay(2000);

}

void loop() {
  
  if ( packet.LED_Token == MY_ECU)
  {
    randomECUselect();
    setRGBcolors(selectColor);
    //Is the sensor active and the ECU is valid ?
    if ((interruptReceived) && (isECUselectionValid()))
    {
      selectColorNextCycle();
      Serial.println("Intrerupt received");
      interruptReceived = false;
      selectECU_number(randomECUSelection);
      delay(RGBCLEARDELAY);
      clearRGBcolors();
      TOFsensor.VL6180xClearInterrupt();
    }else
    {
      //interruptReceived = false;
      //TOFsensor.VL6180xClearInterrupt();
    }
  }
  else
  {
    if (TransmisionStatus == SENDDATA)
    {
      char macStr[18];
      snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
      Serial.print("sending to:");
      Serial.println(macStr);
      esp_now_send(receiverECU_Address, (uint8_t *) &packet, sizeof(packet));      
    }
     else
     {
      if(TransmisionStatus==TRANSMISIONSUCCESFUL)
      {
        Serial.println("Transmision succesful");
        TransmisionStatus=ONLYRECEIVE;
        interruptReceived = false;
        TOFsensor.VL6180xClearInterrupt();
      }
      else
      {
        if(TransmisionStatus==ONLYRECEIVE) 
        {
          interruptReceived = false;
          clearRGBcolors();
          TOFsensor.VL6180xClearInterrupt();
        }   
      }
    }
  }
}