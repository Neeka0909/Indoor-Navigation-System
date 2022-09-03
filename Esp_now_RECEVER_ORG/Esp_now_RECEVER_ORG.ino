#include <esp_now.h>
#include <WiFi.h>


#include <ArduinoJson.h>
String jsonData;
StaticJsonDocument<256> doc;
String UUID;
int devID;
int device;
int rssi;

int prvRssi;
int prvDevID;
char prvMacStr[18];

int LED_BUILTIN = 2;

void ledBlink(int timeDelay){
  digitalWrite(LED_BUILTIN,HIGH);
  delay(timeDelay);
  digitalWrite(LED_BUILTIN,LOW);
  delay(timeDelay);
}


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  ledBlink(5);
  char macStr[18];
  //char prvMacStr[18];
  //int prvRssi;
  //int prvDevID;
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  char* buff=(char*) incomingData;
  jsonData = String(buff);
  Serial.print("Recieved");
  Serial.println(jsonData);
  DeserializationError error = deserializeJson(doc, jsonData);
  String UUID = doc["uuid"];
  int devID = doc["pointer"];
  int rssi = doc["rssi"];
  Serial.print("Device no: ");
  Serial.println(devID);
  Serial.print("UUID: ");
  Serial.println(UUID);
  Serial.print("RSSI Value: ");
  Serial.println(rssi);
  Serial.println();





  
  if (prvMacStr == macStr){
    //data recive from same device 
  }else if(UUID == "60e71442-cf25-4e8e-aaf7-f56aaa650303"){
      if(prvRssi >= rssi){
        Serial.printf(" if 1 Closest pointer is :%d", devID);
      }else{
        Serial.printf("if 2 Closest pointer is :%d", prvDevID);
      }
      ledBlink(500);
    }
  Serial.println("");
  Serial.println("before convert");
  Serial.println(prvRssi);
  Serial.println(prvDevID);
  prvMacStr[18] = macStr[18];
  prvRssi = rssi;
  prvDevID = devID;
  Serial.println("after convert");
  Serial.println(prvRssi);
  Serial.println(prvDevID);
  jsonData.clear();
  
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);

  pinMode(LED_BUILTIN,OUTPUT);
  
  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_STA);



  
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  
}
 
void loop() {
  // Acess the variables for each board
  /*int board1X = boardsStruct[0].x;
  int board1Y = boardsStruct[0].y;
  int board2X = boardsStruct[1].x;
  int board2Y = boardsStruct[1].y;
*/


  
}
