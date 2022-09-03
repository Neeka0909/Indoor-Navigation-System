#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>
#include <ArduinoJson.h>
String jsonData;
StaticJsonDocument<256> doc;

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

// ESP NOW Communication
#include <esp_now.h>
#include <WiFi.h>


int scanTime = 5; //In seconds
BLEScan *pBLEScan;
void  BleScan(void);
void  DataSending(void);

// REPLACE WITH THE RECEIVER'S MAC Address (78:21:84:8E:24:B4)
uint8_t broadcastAddress[] = {0x78, 0x21, 0x84, 0x8E, 0x24, 0xB4};



int LED_BUILTIN = 2;

const char *UUID;
int RSSI_data;
int device_count = 0;
int i = 0;
String BleAddr[10];
int BleRssi[10];
int Count = 0;
int deviceCount = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
    }
};

// Create peer interface
esp_now_peer_info_t peerInfo;
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("WI-FI Start");
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  BleScan();
  DataSending();

  digitalWrite(LED_BUILTIN, HIGH);
  delay(5);
  digitalWrite(LED_BUILTIN, LOW);
  delay(5);
  delay(100);
  
}

void BleScan() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(500);
  pBLEScan->setWindow(499); // less or equal setInterval value

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  Serial.print("Devices found: ");
  deviceCount = foundDevices.getCount();
  Serial.println(deviceCount);

  for (uint32_t i = 0; i < deviceCount; i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);

    if (device.haveManufacturerData() == true) {
      std::string strManufacturerData = device.getManufacturerData();
      uint8_t cManufacturerData[100];

      strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

      if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)
      {
        //Serial.println("Found an iBeacon!");
        BLEBeacon oBeacon = BLEBeacon();
        oBeacon.setData(strManufacturerData);
        UUID = oBeacon.getProximityUUID().toString().c_str();
        RSSI_data = device.getRSSI(); // CAN GET THE RSSI WITHOUT PROBLEM
        //Serial.printf("iBeacon Frame\n");
        //Serial.printf("DeviceNo: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()),UUID, oBeacon.getSignalPower());
        Serial.printf("DeviceNo =  %d --- ", i);
        Serial.printf("UUID = %s --- ", UUID);
        //converting UUID to String
        Serial.printf("RSSI =  %d --- ", RSSI_data);
        Serial.println("");
        //json
        //                    doc["uuid"] = ID;
        //                    doc["rssi"] = RSSI_data;
        //                    serializeJson(doc,jsonData);
        //                    Serial.println(jsonData);
        //                    jsonData = "";
        //                    String FID = String(UUID);
        //                    Serial.printf("String = --- ");
        //                    Serial.print(FID);
        BleAddr[i] =  UUID;
        // delay(5);
        BleRssi[i] = RSSI_data;
        Count = i + 1;
        Serial.println();
        delay(100);


      }
    }
  }

  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  Serial.println("Scan done!");
  Serial.printf("Ble Device =  %d --- ", Count);
  Serial.println();
}

void DataSending() {
  doc["pointer"] = 5;

  for (byte i = 0; i < Count; i = i + 1) {
    if (deviceCount == 0) {
      memset(BleAddr, 0, sizeof(BleAddr));
      memset(BleRssi, 0, sizeof(BleRssi));
      break;
    }
    doc["uuid"] = BleAddr[i];
    doc["rssi"] = BleRssi[i];
    serializeJson(doc, jsonData); //Serialize data
    delay(10);
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) jsonData.c_str(), sizeof(jsonData) + 60);
    Serial.println(jsonData);
    //Debugging
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    delay(50);
    jsonData = "";
    delay(5);

  }
  Serial.println("BLE scan start");
}
