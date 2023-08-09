/*

 Project Name Reaction Lights Training Module Project.
 Developed by : projectswithalex.
 Email :projectswithalex@gmail.com
 Github : https://github.com/projectswithalex/Reaction-Lights-Training-Module
 Social Media and stuff : https://linktr.ee/projectswithalex
 

Disclaimer : Code is opensource and can be modified by everyone. If you can improve the code and add new functionality to it i would be happy to get
              in touch with you and update the repository. 
              Also for everything HW related you have a full responsability for your build. I am not a hardware developer and every decision that i have made
              when i build this project was my own understanding over the modules that i have used. Always double check and if you have any questions let me know.
              
              
 Thanks for being here and wish you a good training session with this modules.
*/


#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <VL6180X_WE.h>


/******************************** TRAINING MODE SELECTION ******************************/
#define SlaveECU

enum tipeOfTraining_en {
  TRAINING_START = 0,
  TRAINING_SIMPLE = 1,           //GREEN
  TRAINING_ALLONALLOFF,          //BLUE
  TRAINING_RETURNTOMASTER,       //YELLOW
  TRAINING_PARTNERMODE,          //AQUA
  TRAINING_TIMERMODE = 5,  //MAGENTA
  TRAINING_END = 5
};
bool training_startTraining_Flag = false;
bool training_allSettingsSent = false;
void trainingSimpleMain(void);
void trainingAllOnAllOffMain(void);
uint8_t TRAINING_ALLONALLOFF_sendData = 0;
uint8_t trainingAllOnAllOfActiveColorIndex = 0;  //red
void trainingReturnToMasterMain(void);
void trainingPartnerModeMain(void);
void trainingTimerModeMain(void);

bool settingsReceivedFlag = false;
uint8_t counterExercise = 0;
uint8_t startOfNewTrainingCounter = 0;


volatile bool timerIntreruptFlag;

ICACHE_RAM_ATTR void onTimerInt() {
  timerIntreruptFlag = true;
}


/******************************** TRAINING MODE SELECTION ******************************/

/********************************ESP NOW COMMUNICATION CODE ******************************/
#define NEWTRAININGMAXTIME 4
#define MY_ROLE ESP_NOW_ROLE_COMBO        // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE ESP_NOW_ROLE_COMBO  // set the role of the receiver
/*replaceValueHere*/ #define MY_ECU 4     //ECU number
#define WIFI_CHANNEL 1
#define MACADDRESSSIZE 6                       //Mac address size
#define NO_ECU 0                               //No ecu with the define MY_ECU 0
#define RGBCLEARDELAY 100                      //delay to be used with RGB clear ?TBD
  /*replaceValueHere*/ #define AVAILABLEECU 4  //Nr of ECUs to be used
#define MAXAVAILABLEECU 10                     // I think ESPNOW supports up to 10 devices

  //Receivers ECUS addreses.Add all of them here.

  /*replaceValueHere*/ uint8_t receiverAddress1[] = { 0xF4, 0xCF, 0xA2, 0x5D, 0x75, 0x28 };  //  this ECU MAC address ,only for example purposes
/*replaceValueHere*/ uint8_t receiverAddress2[] = { 0xAC, 0x0B, 0xFB, 0xCF, 0xC1, 0x0F };    //  ECU 2
/*replaceValueHere*/ uint8_t receiverAddress3[] = { 0xAC, 0x0B, 0xFB, 0xCF, 0xD8, 0xB1 };    //  ECU 3
///*replaceValueHere*/ uint8_t receiverAddress4[] = { 0xF4, 0xCF, 0xA2, 0x79, 0x23, 0x84 };    //  ECU 4
// /*replaceValueHere*/ uint8_t receiverAddress4[] = { 0x4C, 0xEB, 0xD6, 0x62, 0x09, 0x54 };    //  ECU 5

uint8_t receiverECU_Address[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //Placeholder for the receiver address

uint8_t receiverArray[MAXAVAILABLEECU][MACADDRESSSIZE];

struct __attribute__((packed)) dataPacketAlone {
  uint8_t LED_Token;  // Token for activating ECUs
  uint8_t counterExerciseData;
};

struct __attribute__((packed)) dataPacketPartner {
  uint8_t LED_Token_Partner;
  uint8_t activeECU;
  uint8_t counterExercisePartner;
};

struct __attribute__((packed)) dataPacketSettings {
  uint8_t training_NrOfEcus;
  uint8_t training_trainingType;
  uint8_t training_nrOfColors;
  uint8_t training_counterValStop;
  uint16_t training_stopTimeDuration;
  uint8_t training_partnerMode_P1Color;
  uint8_t training_partnerMode_P2Color;
  uint8_t winnerPartner;
};


//state in which the ECU can be found
enum transmissionState_en {
  DATARECEIVED_en,
  SENDDATA_en,
  SENDINGDATA_en,
  TRANSMISIONSUCCESFULL_en,
  ONLYRECEIVE_en
};

/*replaceValueHere*/ dataPacketAlone packetAlone = { 1, 0 };  //Package of data to be sent !if not ECU1 set to 0!
dataPacketPartner packetPartner;
dataPacketSettings packetSettings;
dataPacketPartner partnerLocal;
transmissionState_en TransmisionStatus = DATARECEIVED_en;  //Transmision Status



void initReceiverAddress(void) {

  switch (packetSettings.training_NrOfEcus) {

      //  case 2:
      //
      //    memcpy(&receiverArray[2], receiverAddress2, 6);
      //    esp_now_add_peer(receiverAddress2, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
      //    break;
      //
    case 3:

      memcpy(&receiverArray[2], receiverAddress2, 6);
      memcpy(&receiverArray[3], receiverAddress3, 6);
      esp_now_add_peer(receiverAddress2, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
      esp_now_add_peer(receiverAddress3, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
      break;

    case 4:

      memcpy(&receiverArray[2], receiverAddress2, 6);
      memcpy(&receiverArray[3], receiverAddress3, 6);
      //to add 5
      esp_now_add_peer(receiverAddress2, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
      esp_now_add_peer(receiverAddress3, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
      //to add 5
      break;
  }
  //and so on until MAXAVAILABLEECU
}


void initESPNOWcomm(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  esp_now_set_self_role(MY_ROLE);
  esp_now_register_send_cb(transmissionComplete);  // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);          // this function will get called whenever we receive data

  /*replaceValueHere*/                                                       //add peers here or modify the reciverAddress to the right ECUS
  esp_now_add_peer(receiverAddress1, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);  // this is the master and we need to add it before everyone else because the commands come from it.
  memcpy(&receiverArray[1], receiverAddress1, 6);
}
/********************************ESP NOW COMMUNICATION CODE ******************************/

/******************************** RGB LED CODE  ******************************/
//RGB Defines
#define RGBDATAPIN 2
#define RGBLEDNUM 16
#define NUMBEROFCOLORS 7
Adafruit_NeoPixel pixels(RGBLEDNUM, RGBDATAPIN, NEO_GRB + NEO_KHZ800);

//                                            RED   Green  Blue
const uint8_t colors[NUMBEROFCOLORS][3] = { { 150, 0, 0 },        //RED
                                            { 0, 150, 0 },        //GREEN
                                            { 0, 0, 150 },        //BLUE
                                            { 150, 150, 0 },      //YELLOW
                                            { 0, 150, 150 },      //AQUA
                                            { 150, 0, 150 },      //MAGENTA
                                            { 150, 150, 150 } };  //WHITE
uint8_t selectColor = 0;

//Select color next ON cycle
void selectColorNextCycle(void) {
  selectColor++;
  if (selectColor > NUMBEROFCOLORS - 1) {
    selectColor = 0;
  }
}

uint8_t generateRandomColor(void) {
  randomSeed(millis());
  uint8_t flaglocal = 0;
  uint8_t returnValue = 0;
  uint8_t randomColor = 0;
  while (flaglocal == 0) {
    randomColor = random(packetSettings.training_nrOfColors);

    if (packetSettings.training_trainingType == TRAINING_ALLONALLOFF) {
      if (randomColor != trainingAllOnAllOfActiveColorIndex) {  //BLUE is the active LIGHT for ALLONALLOFF
        returnValue = randomColor;
        flaglocal = 1;
      }
    } else {
      flaglocal = 1;
      returnValue = randomColor;
    }
  }
  return returnValue;
}

void setRGBcolors(uint8_t colorIndex) {

  for (int i = 0; i < RGBLEDNUM; i++) {  // For each pixel...
    pixels.setPixelColor(i, pixels.Color(colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]));
    pixels.show();  // Send the updated pixel colors to the hardware.
  }
}

void setRGBColorsNumber(uint8_t number) {
  for (int i = 0; i < number; i++) {  // For each pixel...
    pixels.setPixelColor(i, pixels.Color(colors[packetSettings.training_trainingType][0], colors[packetSettings.training_trainingType][1], colors[packetSettings.training_trainingType][2]));
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
//Battery Check Defines
#define BATMEAS A0       // Analog input pin
int analogVal = 0;       // Analog Output of Sensor
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
/******************************** BATTERY CHECK CODE  ******************************/

/******************************** TOF SENSOR CODE  ******************************/
#define VL6180X_ADDRESS 0x29
#define TOF_INT 12
#define SCL_PIN 5
#define SDA_PIN 4

VL6180xIdentification identification;
VL6180x TOFsensor(VL6180X_ADDRESS);

volatile bool intrerruptTOF = false;
volatile int resTOFflag = 0;

ICACHE_RAM_ATTR void handleInterruptTOF() {

  intrerruptTOF = true;
  if (counterExercise >= packetSettings.training_counterValStop) {
    resTOFflag++;
  }
}


void initTOFSensor(void) {
  pinMode(TOF_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOF_INT), handleInterruptTOF, FALLING);

  delay(500);  //do i really need this here

  while (TOFsensor.VL6180xInit() == VL6180x_FAILURE_RESET) {
    Serial.println("FAILED TO INITALIZE");  //Initialize device and check for errors
  }
    delay(500);   
  TOFsensor.VL6180xDefautSettings();  //Load default settings to get started.
  delay(500);                                               //do i really need this here
  /*replaceValueHere*/ TOFsensor.VL6180xSetDistInt(20, 20);  //it detects a movement when it lower than 2cm. With the current initialization should work for values up until 20cm .

  TOFsensor.getDistanceContinously();
  TOFsensor.VL6180xClearInterrupt();
  intrerruptTOF = false;
  //delay(500);  //do i really need this here
}
/******************************** TOF SENSOR CODE  ******************************/

//Callback function after a transmission has been sent
void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    if (packetSettings.training_trainingType == TRAINING_ALLONALLOFF) {
      TRAINING_ALLONALLOFF_sendData++;
    } else {
      TransmisionStatus = TRANSMISIONSUCCESFULL_en;
    }

  } else {
    TransmisionStatus = SENDDATA_en;
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

  switch (dataLength) {
    case 2:
      memcpy(&packetAlone, data, sizeof(packetAlone));
      break;

    case 3:
      Serial.println("local");
      memcpy(&partnerLocal, data, sizeof(partnerLocal));
      break;

    case 9:
      memcpy(&packetSettings, data, sizeof(packetSettings));
      settingsReceivedFlag = false;
      break;
  }
  TransmisionStatus = DATARECEIVED_en;
}
/******************************** Logic CODE  ******************************/

uint8_t randomECUSelection = 0;

uint8_t randomECUselect(void) {

  randomSeed(millis());
  uint8_t returnValue = 0;
  uint8_t randomNumber = 0;
  while (returnValue == 0) {
    randomNumber = random(0, packetSettings.training_NrOfEcus + 2);  //we have +2 because 1 is master and the function is exclusive

    if ((randomNumber != MY_ECU) && (randomNumber != NO_ECU)) {
      returnValue = randomNumber;
    }
  }

  return returnValue;
}

void selectECU_number(uint8_t ECU) {
  memcpy(&receiverECU_Address, receiverArray[ECU], MACADDRESSSIZE);
  packetAlone.LED_Token = ECU;
  TransmisionStatus = SENDDATA_en;
}

void endOfTrainingLightPartner(void) {
  if (packetSettings.winnerPartner == 1) {
    setRGBcolors(packetSettings.training_partnerMode_P1Color);
    delay(500);
    pixels.clear();
    pixels.show();
    delay(500);
  } else {
    setRGBcolors(packetSettings.training_partnerMode_P2Color);
    delay(500);
    pixels.clear();
    pixels.show();
    delay(500);
  }
  startOfNewTrainingCounter++;
  if (startOfNewTrainingCounter > NEWTRAININGMAXTIME) {
    ESP.restart();
  }
}

void endOfTrainingLight(void) {
 setRGBcolors(0);
  delay(500);
  pixels.clear();
  pixels.show();
  delay(500);

  if (intrerruptTOF) {
    delay(300);
    intrerruptTOF = false;
    TOFsensor.VL6180xClearInterrupt();
    Serial.print("Flag endOfTrainingLight:");
    Serial.println(resTOFflag);
    if (resTOFflag > 5) {
      restartTrainingModeSingle();
    }
  }
}

void restartTrainingModeSingle(void) {

  unsigned long start = millis();
  while (millis() - start < 3000) {

    Serial.print("Broadcast");
    packetAlone.LED_Token = MY_ECU;
    packetAlone.counterExerciseData = 0;
    esp_now_send(NULL, (uint8_t *)&packetAlone, sizeof(packetAlone));

    pixels.clear();
    pixels.show();

    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
    intrerruptTOF = false;
    TOFsensor.VL6180xClearInterrupt();

    TransmisionStatus = DATARECEIVED_en;
    packetAlone.LED_Token = MY_ECU;
    counterExercise = 0;
    resTOFflag = 0;
  }
}




/******************************** Logic CODE  ******************************/

void setup() {
  Serial.begin(115200);          // initialize serial port
  Wire.begin(SDA_PIN, SCL_PIN);  //Initialize I2C for VL6180x (TOF Sensor)
  pinMode(BATMEAS, INPUT);       //measure Battery Pin

  timer1_attachInterrupt(onTimerInt); // Add ISR Function
	timer1_enable(TIM_DIV256 , TIM_EDGE, TIM_SINGLE);

  Serial.println("pixels");
  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();  // Set all pixel colors to 'off'
  pixels.show();
  Serial.println("initESPNOWcomm");
  initESPNOWcomm();

  Serial.println("initTOFSensor");
  initTOFSensor();
  Serial.println("initBatteryCheck");
  initBatteryCheck();
  Serial.println("almost Ready");
  delay(1000);
  // clearRGBcolors();

  Serial.println("Ready");
}

unsigned long timeFlag = 0;
unsigned long countTime = 0;
void loop() {

  if (packetSettings.training_trainingType != 0 && settingsReceivedFlag == false) {

    Serial.println("Start Training");
    initReceiverAddress();
    clearRGBcolors();
    settingsReceivedFlag = true;
    TOFsensor.VL6180xClearInterrupt();
    intrerruptTOF = false;
  }

  switch (packetSettings.training_trainingType) {
    case TRAINING_SIMPLE:
      trainingSimpleMain();
      break;

    case TRAINING_ALLONALLOFF:
      trainingAllOnAllOffMain();
      break;

    case TRAINING_RETURNTOMASTER:
      trainingReturnToMasterMain();
      break;

    case TRAINING_PARTNERMODE:
      trainingPartnerModeMain();
      break;

    case TRAINING_TIMERMODE:
      trainingTimerModeMain();
      break;

    case TRAINING_START:
      if (packetSettings.training_NrOfEcus != 0) {
        endOfTrainingLightPartner();
      }
      break;
  }
}


void trainingSimpleMain(void) {
  if (counterExercise < packetSettings.training_counterValStop) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();
      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(selectColor);
      //Is the sensor active and the ECU is valid ?
      if (intrerruptTOF) {
        selectColor = generateRandomColor();
        Serial.println("Intrerupt received");
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
        counterExercise = packetAlone.counterExerciseData;
        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
      } else {
        //do nothing but wait
      }
    } else {
      if (TransmisionStatus == SENDDATA_en) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
        esp_now_send(receiverECU_Address, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;
      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {
            intrerruptTOF = false;
            clearRGBcolors();
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {
      endOfTrainingLight();
  }
}

void trainingAllOnAllOffMain(void) {
  if (TransmisionStatus == DATARECEIVED_en || TRAINING_ALLONALLOFF_sendData >= (packetSettings.training_NrOfEcus - 1)) {
    countTime = millis();
    while (millis() - countTime < packetSettings.training_stopTimeDuration) {
      clearRGBcolors();
    }
    TRAINING_ALLONALLOFF_sendData = 0;
    TransmisionStatus = ONLYRECEIVE_en;
    selectColor = generateRandomColor();
    setRGBcolors(selectColor);
  }

  if (counterExercise < packetSettings.training_counterValStop) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();

      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(trainingAllOnAllOfActiveColorIndex);
      //Is the sensor active and the ECU is valid ?
      if (intrerruptTOF) {
        Serial.println("Intrerupt received");
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
        counterExercise = packetAlone.counterExerciseData;
        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
      } else {
        //do nothing but wait
      }
    } else {

      if (TransmisionStatus == SENDDATA_en) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
        esp_now_send(NULL, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;

      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {

            intrerruptTOF = false;
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {
      endOfTrainingLight();
  }
}

void trainingReturnToMasterMain(void) {

  if (counterExercise < packetSettings.training_counterValStop) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = 1;  //that is the master
      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(selectColor);
      //Is the sensor active and the ECU is valid ?
      if (intrerruptTOF) {
        selectColor = generateRandomColor();
        Serial.println("Intrerupt received");
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
        counterExercise = packetAlone.counterExerciseData;
        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
      } else {
        //do nothing but wait
      }
    } else {
      if (TransmisionStatus == SENDDATA_en) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
        esp_now_send(receiverECU_Address, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;
      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {
            intrerruptTOF = false;
            clearRGBcolors();
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {
      endOfTrainingLight();
  }
}

void trainingPartnerModeMain(void) {
  if (partnerLocal.activeECU == MY_ECU) {

    if (partnerLocal.LED_Token_Partner == 1) {
      setRGBcolors(packetSettings.training_partnerMode_P1Color);
    } else {
      if (partnerLocal.LED_Token_Partner == 2) {
        setRGBcolors(packetSettings.training_partnerMode_P2Color);
      }
    }


    //Is the sensor active and the ECU is valid ?
    if (intrerruptTOF) {
      Serial.println("Intrerupt received p1");
      intrerruptTOF = false;
      memcpy(&receiverECU_Address, receiverArray[1], MACADDRESSSIZE);
      //delay(RGBCLEARDELAY);  //why did i used this ???
      clearRGBcolors();
      TOFsensor.VL6180xClearInterrupt();
      partnerLocal.counterExercisePartner++;
      TransmisionStatus = SENDDATA_en;
    }
    if (TransmisionStatus == SENDDATA_en) {
      char macStr[18];
      snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
      Serial.print("sending to:");
      Serial.println(macStr);
      esp_now_send(receiverECU_Address, (uint8_t *)&partnerLocal, sizeof(partnerLocal));
      TransmisionStatus = SENDINGDATA_en;
    } else {
      if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

        Serial.println("Transmision succesful");
        TransmisionStatus = ONLYRECEIVE_en;
        intrerruptTOF = false;
        TOFsensor.VL6180xClearInterrupt();
        partnerLocal.activeECU = 0;
        partnerLocal.LED_Token_Partner = 0;
        clearRGBcolors();
      }
    }
  }
}

void trainingTimerModeMain(void) {
  if (counterExercise < packetSettings.training_counterValStop) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();
      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(selectColor);
      //Is the sensor active and the ECU is valid ?
      if (timerIntreruptFlag) {
        selectColor = generateRandomColor();
        Serial.println("Intrerupt received");
        timerIntreruptFlag=false;
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
        counterExercise = packetAlone.counterExerciseData;
        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
        Serial.print("counter:");
        Serial.println(counterExercise);

      } else {
        //do nothing but wait
      }
    } else {
      if (TransmisionStatus == SENDDATA_en) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        // Serial.print("sending to:");
        // Serial.println(macStr);
        esp_now_send(receiverECU_Address, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;
      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {
            intrerruptTOF = false;
            clearRGBcolors();
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {

      endOfTrainingLight();

  }
}
