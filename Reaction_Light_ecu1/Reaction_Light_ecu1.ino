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
/********************************ESP NOW COMMUNICATION CODE ******************************/
#define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
#define MY_ECU          1                               //ECU number
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
uint8_t receiverAddress2[] = {0xE8, 0xDB, 0x84, 0xDC, 0xF7, 0xC4};   //  ECU 2
uint8_t receiverAddress3[] = {0xE8, 0xDB, 0x84, 0xE3, 0x19, 0x63};   //  ECU 3


uint8_t receiverECU_Address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //Placeholder for the receiver address

struct __attribute__((packed)) dataPacket {
  int LED_Token;  // Token for activating ECUs
  int Mode;       // Training Mode
  struct partnerMode { 
        int partEcu1;             
        int partEcu2;
        int partEcu3;
        int partEcu4;
  }
};

dataPacket packet = {1, 0 , {0 ,0 ,0 ,0}}; //Package of data to be sent
int TransmisionStatus = 0; //Transmision Status
const int numberOfECU = 3; //Number of ECUs in the network

/********************************ESP NOW COMMUNICATION CODE ******************************/

/******************************** RGB LED CODE  ******************************/
//RGB Defines
#define RGBDATAPIN  14
#define RGBLEDNUM 4
#define NUMBEROFCOLORS 7
Adafruit_NeoPixel pixels(RGBLEDNUM, RGBDATAPIN, NEO_GRB + NEO_KHZ800);
//Button define
#define IRSENSOR 2
volatile bool irsensor;
//                                      RED  Green Blue
const int colors[NUMBEROFCOLORS][3] = { { 100 , 0   , 0  },
                                        { 0   , 100 , 0  },
                                        { 0   , 0   , 100},
                                        { 100 , 0   , 0  },
                                        { 0   , 100 , 100},
                                        { 100 , 100 ,   0},
                                        { 100 , 0   , 100}
};
int selectColor=0;

// interrupt routine for IR sensor Detection
void ICACHE_RAM_ATTR handleIRsensorInterrupt()
{
    // set the flag for movement detection
    irsensor=true;
}

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
  Serial.println("Clear RGB");
  for(int i=0; i<RGBLEDNUM; i++) { // For each pixel...
      
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
}

unsigned long Time = 0;           // the last time the output pin was toggled
unsigned long debounce = 20UL;   // the debounce time, increase if the output flickers
unsigned long randomNumber = 0;
unsigned long randomECUSelection = 0;


/******************************** RGB LED CODE  ******************************/
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


int randomECUselect(void)
{
  randomNumber++;
  if (randomNumber > 65.535) {
    randomNumber = 0;
  }
  return randomNumber;
}
//Random ECU Selection for message transmision
bool isECUselectionValid() {

  randomECUSelection = (randomECUselect()) % (numberOfECU + 1);
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

    case 2:
      memcpy(&receiverECU_Address, receiverAddress2, MACADDRESSSIZE);
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
  esp_now_add_peer(receiverAddress2, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(receiverAddress3, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

  pinMode(IRSENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(IRSENSOR), handleIRsensorInterrupt, RISING);
  irsensor = false;
}

void loop() {

  switch ()
  if ( packet.LED_Token == MY_ECU)
  {
    setRGBcolors(selectColor);
    //Is the sensor active and the ECU is valid ?
    if ((irsensor == true) && (isECUselectionValid()))
    {
      selectColorNextCycle();
      Serial.print("irsensor: ");
      Serial.println(irsensor);
      irsensor = false;
      selectECU_number(randomECUSelection);
      delay(RGBCLEARDELAY);
      clearRGBcolors();
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
        irsensor = false;
      }
      else
      {
        if(TransmisionStatus==ONLYRECEIVE) 
        {
          irsensor = false;
          clearRGBcolors();
        }   
      }
    }
  }
}
