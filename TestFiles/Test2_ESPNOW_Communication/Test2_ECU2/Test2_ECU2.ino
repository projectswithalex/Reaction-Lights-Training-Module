#include <ESP8266WiFi.h>
#include <espnow.h>
#define MasterECU
#define MY_ECU 1     //ECU number
#define WIFI_CHANNEL 1
#define MY_ROLE ESP_NOW_ROLE_COMBO        // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE ESP_NOW_ROLE_COMBO  // set the role of the receiver

  /*replaceValueHere*/ uint8_t receiverAddress1[] = { 0xF4, 0xCF, 0xA2, 0x5D, 0x75, 0x28 };    //  this ECU MAC address ,only for example purposes
 // /*replaceValueHere*/ uint8_t receiverAddress2[] = { 0x5C, 0xCF, 0x7F, 0x09, 0xBA, 0x5C };  //  ECU 2

struct __attribute__((packed)) dataPacketTransmission {
  uint8_t messageCount;  // MessageCounter
};

dataPacketTransmission TranssmisionPacket={0};

void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    Serial.print("Sent Message NR:");
    Serial.println(TranssmisionPacket.messageCount);
  } else {
     Serial.print("Transmission Error ");
  }
}

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

  Serial.println();
  Serial.print("Received data from: ");
  Serial.println(macStr);

 Serial.print("Received message NR:");
 Serial.println(data[0]);
}

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(MY_ROLE);
  esp_now_register_send_cb(transmissionComplete);  // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);  

  esp_now_add_peer(receiverAddress1, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

}
 
void loop(){
  delay(1000);
  TranssmisionPacket.messageCount++;
  Serial.print("Increment message Count to:");
  Serial.println(TranssmisionPacket.messageCount);
  delay(1000);
  esp_now_send(receiverAddress1, (uint8_t *)&TranssmisionPacket, sizeof(TranssmisionPacket));
  delay(1000);
}
