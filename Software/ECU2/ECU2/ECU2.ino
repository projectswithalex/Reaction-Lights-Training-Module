/*

 Project Name Reaction Lights Training Module Project.
 Developed by : projectswithalex.
 Email :projectswithalex@gmail.com
 Github : https://github.com/projectswithalex/Reaction-Lights-Training-Module
 Social Media and stuff : https://linktr.ee/projectswithalex
 
 

Check to code and please add your values/settings where needed. I have commented every line with the following indicator "replaceValueHere" to make it easier for you.

Disclaimer : Code is opensource and can be modified by everyone. If you can improve the code and add new functionality to it i would be happy to get
              in touch with you and update the repository. 
              Also for everything HW related you have a full responsability for your build. I am not a hardware developer and every decision that i have made
              when i build this project was my own understanding over the modules that i have used. Always double check and if you have any questions let me know
              
              
 Thanks for being here and wish you a good training session with this modules.
*/


#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <VL6180X_WE.h>

/******************************** ACTIVATE COUNTER CODE ******************************/
#define TWOCOLORS 1
#if TWOCOLORS

#define COLOR1 0
#define COLOR2 2

/*replaceValueHere*/ #define TIMEBETWEEN  1000 //1sec
uint8_t twoColorsRGBflag = 0;

#endif



/******************************** ACTIVATE COUNTER CODE ******************************/


/******************************** ACTIVATE COUNTER CODE ******************************/
/*
modify COUNTERACTIVE value to 1 to be able to use the COUNTER settings.After a defined number of touches the set is over and the ECU will flash.
Keep the hand over the ECU and it will start a new training set.
*/

/*replaceValueHere*/ #define COUNTERACTIVE 1
#if COUNTERACTIVE
  /*replaceValueHere*/ uint8_t counterExercise = 0;  //Counter !if not ECU1 set to 0!
/*replaceValueHere*/ uint8_t stopExercise = 20;      //How many touches until stop !if not ECU1 set to 0!
#endif

/******************************** ACTIVATE COUNTER CODE ******************************/

/********************************ESP NOW COMMUNICATION CODE ******************************/
#define MY_ROLE ESP_NOW_ROLE_COMBO        // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE ESP_NOW_ROLE_COMBO  // set the role of the receiver
/*replaceValueHere*/ #define MY_ECU 2     //ECU number
#define WIFI_CHANNEL 1
#define MACADDRESSSIZE 6                       //Mac address size
#define NO_ECU 0                               //No ecu with the define MY_ECU 0
#define RGBCLEARDELAY 100                      //delay to be used with RGB clear ?TBD
  /*replaceValueHere*/ #define AVAILABLEECU 3  //Nr of ECUs to be used
#define MAXAVAILABLEECU 10                     // I think ESPNOW supports up to 10 devices




  //Receivers ECUS addreses.Add all of them here.

  /*replaceValueHere*/ uint8_t receiverAddress1[] = { 0xAC, 0x0B, 0xFB, 0xCF, 0xC1, 0x0F };  // ECU 1
/*replaceValueHere*/                                                                         //uint8_t receiverAddress2[] = {0xAC, 0x0B, 0xFB, 0xCF, 0xD8, 0xB1 };     //  this ECU MAC address ,only for example purposes
/*replaceValueHere*/ uint8_t receiverAddress3[] = { 0xF4, 0xCF, 0xA2, 0x79, 0x23, 0x84 };    //  ECU 3
///*replaceValueHere*/ uint8_t receiverAddress4[] = { 0x4C, 0xEB, 0xD6, 0x62, 0x09, 0x54 };    //  ECU 4


uint8_t receiverECU_Address[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //Placeholder for the receiver address

uint8_t receiverArray[MAXAVAILABLEECU][MACADDRESSSIZE];

struct __attribute__((packed)) dataPacket {
  uint8_t LED_Token;  // Token for activating ECUs
  uint8_t counterExerciseData;
  uint8_t stopExerciseData;
};

//state in which the ECU can be found
enum transmissionState_en {
  DATARECEIVED_en,
  SENDDATA_en,
  SENDINGDATA_en,
  TRANSMISIONSUCCESFULL_en,
  ONLYRECEIVE_en
};

/*replaceValueHere*/ dataPacket packet = { 0, 0, stopExercise };  //Package of data to be sent !if not ECU1 set to 0!
transmissionState_en TransmisionStatus = DATARECEIVED_en;         //Transmision Status


/*replaceValueHere*/ void initReceiverAddress(void) {
  // memcpy(&receiverArray[0], NOECU, 6); //no ECU is allowed to be on 0 position
  memcpy(&receiverArray[1], receiverAddress1, 6);
  // memcpy(&receiverArray[2], receiverAddress2, 6); //This is my ECU position doesn't need to be filed.
  memcpy(&receiverArray[3], receiverAddress3, 6);
 // memcpy(&receiverArray[4], receiverAddress4, 6);
  //.......
  //and so on until MAXAVAILABLEECU
}


void initESPNOWcomm(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(MY_ROLE);
  esp_now_register_send_cb(transmissionComplete);  // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);          // this function will get called whenever we receive data

  /*replaceValueHere*/  //add peers here or modify the reciverAddress to the right ECUS
  esp_now_add_peer(receiverAddress1, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(receiverAddress3, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
 // esp_now_add_peer(receiverAddress4, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

  initReceiverAddress();
}
/********************************ESP NOW COMMUNICATION CODE ******************************/




/******************************** RGB LED CODE  ******************************/
//RGB Defines
#define RGBDATAPIN 2
#define RGBLEDNUM 16
#define NUMBEROFCOLORS 7
/*replaceValueHere*/ #define IGNORECOLOR 0  //IGNORECOLOR=1 use the same color everytime, IGNORECOLOR=0 color changes next time
  Adafruit_NeoPixel pixels(RGBLEDNUM, RGBDATAPIN, NEO_GRB + NEO_KHZ800);


// Colors Table , you can add/remove/modify or just simple ignore the colors setting the define IGNORECOLOR=1
//                                            RED   Green  Blue
const uint8_t colors[NUMBEROFCOLORS][3] = { { 150, 0, 0 },
                                            { 0, 150, 0 },
                                            { 0, 0, 150 },
                                            { 150, 150, 0 },
                                            { 0, 150, 150 },
                                            { 150, 0, 150 },
                                            { 150, 150, 150 } };
uint8_t selectColor = 0;

//Select color next ON cycle
void selectColorNextCycle(void) {
  selectColor++;
  if (selectColor > NUMBEROFCOLORS - 1) {
    selectColor = 0;
  }
}

void setRGBcolors(uint8_t colorIndex) {

  for (int i = 0; i < RGBLEDNUM; i++) {  // For each pixel...
    pixels.setPixelColor(i, pixels.Color(colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]));
    pixels.show();  // Send the updated pixel colors to the hardware.
  }
}

void clearRGBcolors() {
  //Serial.println("Clear RGB");
  for (int i = 0; i < RGBLEDNUM; i++) {              // For each pixel...
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));  //clear the RGBs
    pixels.show();                                   // Send the updated pixel colors to the hardware.
  }
}

/******************************** RGB LED CODE  ******************************/


/******************************** BATTERY CHECK CODE  ******************************/

/*replaceValueHere*/ #define CHECKBATTERYOPTION 1  //recomanded but not necesary
#if CHECKBATTERYOPTION
//Battery Check Defines
#define BATMEAS A0       // Analog input pin
  int analogVal = 0;     // Analog Output of Sensor
int bat_percentage = 0;  // Battery in percentage

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

void readBatValue(void) {
  analogVal = analogRead(BATMEAS);
  /*replaceValueHere*/ float voltage = (((analogVal * 3.3) / 1024) * 1.54);  //1.54 is the constant for me , check out with the multimeter and set the right value for you (trial&error) until correct
  bat_percentage = mapfloat(voltage, 3, 4.2, 0, 100);                        //Real Value

  if (bat_percentage >= 100) {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0) {
    bat_percentage = 1;
  }
};

void showBatteryPercentage(void) {
  if ((bat_percentage <= 100) && (bat_percentage >= 90))  //Battery Levels between 100%-90% = 16LEDS green
  {
    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 90) && (bat_percentage >= 60))  //Battery Levels between 89%-60% = 12LEDS green
  {
    for (int i = 0; i < 12; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 60) && (bat_percentage >= 40))  //Battery Levels between 59%-40% = 8LEDS green
  {
    for (int i = 0; i < 8; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 40) && (bat_percentage >= 20))  //Battery Levels between 39%-20% = 4LEDS green
  {
    for (int i = 0; i < 4; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 20) && (bat_percentage >= 5))  //Battery Levels between 19%-10% = 4LEDS flashing RED leds
  {

    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(20, 0, 0));
      pixels.show();
      i = i + 3;
    }
    delay(200);
    pixels.clear();
    pixels.show();
    delay(200);
    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(20, 0, 0));
      pixels.show();
      i = i + 3;
    }
  }
  // Lower then 10 percent nothing is displayed. As a safety measure since i don't know how much trust i have in the "BMS" that is going to
  // protect the battery of over-discharge.
};



void initBatteryCheck(void) {
  readBatValue();
  showBatteryPercentage();
}
#endif
/******************************** BATTERY CHECK CODE  ******************************/

/******************************** TOF SENSOR CODE  ******************************/
#define VL6180X_ADDRESS 0x29
#define TOF_INT 12
#define SCL_PIN 5
#define SDA_PIN 4

VL6180xIdentification identification;
VL6180x TOFsensor(VL6180X_ADDRESS);

volatile bool interruptReceived = false;
volatile int flag = 0;

ICACHE_RAM_ATTR void handleInterrupt() {
  interruptReceived = true;
#if COUNTERACTIVE
  if (counterExercise >= stopExercise) {
    flag++;
  }
#endif
}


void initTOFSensor(void) {
  pinMode(TOF_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOF_INT), handleInterrupt, FALLING);

  delay(500);  //do i really need this here
  while (TOFsensor.VL6180xInit() == VL6180x_FAILURE_RESET) {
    Serial.println("FAILED TO INITALIZE");  //Initialize device and check for errors
  }
  TOFsensor.VL6180xDefautSettings();                         //Load default settings to get started.
  delay(500);                                                //do i really need this here
  /*replaceValueHere*/ TOFsensor.VL6180xSetDistInt(20, 20);  //it detects a movement when it lower than 2cm. With the current initialization should work for values up until 20cm .
  TOFsensor.getDistanceContinously();
  TOFsensor.VL6180xClearInterrupt();
  interruptReceived = false;
  delay(500);  //do i really need this here
}
/******************************** TOF SENSOR CODE  ******************************/

//Callback function after a transmission has been sent
void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
#if TWOCOLORS
    twoColorsRGBflag++;
#else
    TransmisionStatus = TRANSMISIONSUCCESFULL_en;
#endif
  } else {

#if TWOCOLORS
    TransmisionStatus = SENDDATA_en;
#else
    TransmisionStatus = SENDDATA_en;
#endif
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}
//Callback function after a transmission has been received
void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

  Serial.println();
  Serial.print("Received data from: ");
  Serial.println(macStr);

  memcpy(&packet, data, sizeof(packet));

  counterExercise = data[1];
  stopExercise = data[2];

  TransmisionStatus = DATARECEIVED_en;
}
/******************************** Logic CODE  ******************************/

uint8_t randomECUSelection = 0;

uint8_t randomECUselect(void) {

  randomSeed(millis());
  uint8_t returnValue = 0;
  uint8_t randomNumber = 0;
  while (returnValue == 0) {
    randomNumber = random(1, AVAILABLEECU + 1);
    if ((randomNumber != MY_ECU) && (randomNumber != NO_ECU)) {
      returnValue = randomNumber;
    }
  }

  return returnValue;
}


void selectECU_number(uint8_t ECU) {
  memcpy(&receiverECU_Address, receiverArray[ECU], MACADDRESSSIZE);
  packet.LED_Token = ECU;
  TransmisionStatus = SENDDATA_en;
}

void endOfTrainingLight(void) {
  for (int i = 0; i < RGBLEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(50, 0, 0));
    pixels.show();
    i = i + 3;
  }
  delay(200);
  pixels.clear();
  pixels.show();
  delay(200);
  for (int i = 1; i < RGBLEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 50, 0));
    pixels.show();
    i = i + 3;
  }
  delay(200);
  pixels.clear();
  pixels.show();
  delay(200);
  for (int i = 2; i < RGBLEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 50));
    pixels.show();
    i = i + 3;
  }
  delay(200);
  pixels.clear();
  pixels.show();
  delay(200);

  if (interruptReceived) {
    Serial.print("Flag:");
    Serial.println(flag);
    interruptReceived = false;
    TOFsensor.VL6180xClearInterrupt();
  }
}

void startOfNewTraining(void) {

  unsigned long start = millis();
  while (millis() - start < 3000) {

    Serial.print("Broadcast");
    packet.LED_Token = MY_ECU;
    packet.counterExerciseData = 0;
    esp_now_send(NULL, (uint8_t *)&packet, sizeof(packet));

    pixels.clear();
    pixels.show();

    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
    interruptReceived = false;
    TOFsensor.VL6180xClearInterrupt();

    TransmisionStatus = DATARECEIVED_en;
    packet.LED_Token = MY_ECU;
    counterExercise = 0;
    flag = 0;
  }
}

/******************************** Logic CODE  ******************************/

void setup() {
  Serial.begin(115200);          // initialize serial port
  Wire.begin(SDA_PIN, SCL_PIN);  //Initialize I2C for VL6180x (TOF Sensor)
  pinMode(BATMEAS, INPUT);       //measure Battery Pin

  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();  // Set all pixel colors to 'off'
  pixels.show();

  initESPNOWcomm();

  initTOFSensor();
#if CHECKBATTERYOPTION
  initBatteryCheck();
#endif
  delay(2000);
  clearRGBcolors();
}

unsigned long timeFlag = 0;
unsigned long countTime = 0;
unsigned long sendingPeriod = 0;  //TEST
unsigned long sendingCouter = 0;
void loop() {
#if TWOCOLORS
  if (TransmisionStatus == DATARECEIVED_en || twoColorsRGBflag>=(AVAILABLEECU-1)) {
    countTime = millis();
    while (millis() - countTime < TIMEBETWEEN) {
      clearRGBcolors();
    }
    twoColorsRGBflag=0;
    TransmisionStatus = ONLYRECEIVE_en;
  }
#endif
#if COUNTERACTIVE
  if (counterExercise < stopExercise) {
#endif
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();
      timeFlag = millis();
    }

    if (packet.LED_Token == MY_ECU) {
#if TWOCOLORS
      setRGBcolors(COLOR1);
#else
    setRGBcolors(selectColor);
#endif

      //Is the sensor active and the ECU is valid ?
      if (interruptReceived) {
        interruptReceived = false;
        selectColorNextCycle();
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
#if COUNTERACTIVE
        counterExercise++;
        packet.counterExerciseData = counterExercise;
        packet.stopExerciseData = stopExercise;
        Serial.print("counter:");
        Serial.println(counterExercise);
#endif
      } else {
        // Serial.print("Intrerupt received : ");
        // Serial.println(interruptReceived);
      }
    } else {
#if TWOCOLORS

        setRGBcolors(COLOR2);

#endif
      if (TransmisionStatus == SENDDATA_en) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
#if TWOCOLORS
   //     sendingPeriod = millis();
   //     sendingCouter = millis();
   //     while (millis() - sendingPeriod < 300) {
   //       if (millis() - sendingCouter > 50) {
            esp_now_send(NULL, (uint8_t *)&packet, sizeof(packet));
    //        sendingCouter = millis();
    //      }
    //    }
        
#else
       esp_now_send(receiverECU_Address, (uint8_t *)&packet, sizeof(packet));
      
#endif
    TransmisionStatus = SENDINGDATA_en;

        
      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          interruptReceived = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {

            interruptReceived = false;
#if TWOCOLORS

#else
          clearRGBcolors();
#endif
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }
#if COUNTERACTIVE
  } else {
    if (flag < 5) {
      endOfTrainingLight();
    } else {
      startOfNewTraining();
      Serial.print("Reset");
    }
  }
#endif
}