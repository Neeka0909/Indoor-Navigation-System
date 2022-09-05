#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <Arduino.h>

String jsonData;
StaticJsonDocument<256> doc;
// Json Variable to Hold Sensor Readings
String serverData;
StaticJsonDocument<256> rowData;

String UUID;
int devID;
int device;
int rssi;

int prvRssi = -60;
int prvDevID;
char prvMacStr[18];

int locationCode;

int LED_BUILTIN = 2;

//Main WiFi network credentials (STATION)
const char* ssid = "NCV_FIBER";
const char* password = "n4jq6ule3sz";

//Web UI data Passign
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

String UserUUID;
int UserDesti;
int UserRssi;

AsyncWebServer server(80);
AsyncEventSource events("/events");



//HTML Code,  Web UI code.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Indoor Navigation System</title>
    <style>
        body {
            background: linear-gradient(90deg, #FC466B 0%, #3F5EFB) no-repeat center;
            background-size: cover;
            overflow: scroll;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            background-attachment: fixed;


        }

        .container {
            display: flex;
            justify-content: center;
            align-items: center;
            margin-top: 150px;
            width: 100%;
            height: 100%;
            overflow: visible;

        }

        .login {
            padding-left: 20px;
            padding-right: 20px;
            padding-bottom: 50px;
            border-bottom-width: 0px;
            margin-right: 0px;
            border-radius: 3%;
            background-color: #cec5c5;
        }

        form {
            font-family: Cambria, Cochin, Georgia, Times, 'Times New Roman', serif;
            opacity: 1;
        }

        form h2 {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            text-align: center;
            padding-top: 25px;
        }

        .container p {
            position: relative;
            transform: translate(-50%, -50%);
            left: 50%;
            justify-content: center;
        }

        input[type=text],
        select {
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

        button {
            width: 100%;
            background-color: #4CAF50;
            color: white;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        button:hover {
            background-color: #45a049;
        }

        .map {
            display: block;
            background-color: #ece6e6;
            padding: 2%;
            border-radius: 25px;
            width: 605px;
            height: 800px;
        }

        .map h2 {

            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            text-align: center;
        }

        #Canvas {
            object-fit: contain;
        }
    </style>
</head>

<body>
    <!-- Display Form -->
    <div class="container">
        <div class="login">
            <form name="Bledata" id="Fpage">
                <h2>Welcome To Indoor Navigation System</h2>
                <label for="uuid">Enter Your UUID :</label>
                <input id="uuid" type="text" placeholder="UUID..." name="input1" required>
                <label for="locations">Choose a Loaction:</label>
                <select name="input2" id="locations" form="Fpage" required>
                    <option value="D1">Door 1</option>
                    <option value="D2">Door 2</option>
                    <option value="entrance">Entrance</option>
                    <option value="exit">Exit</option>
                </select>
                <button type="button" onclick="formSubmit()">Submit</button>
            </form>
        </div>
    </div>

    <div class="container">
        <!-- Display Map -->
        <div class="map">
            <h2>Map Of the hall area 1</h2>
            <canvas id="Canvas" width="600px" height="700px"></canvas>
        </div>
    </div>
</body>

</html>
<script>


    var canvas = document.getElementById('Canvas');
    console.log("Start");
    var context = canvas.getContext("2d");
    context.font = "15px Segoe UI";

    // Map sprite
    var mapSprite = new Image();
    mapSprite.src = "https://lielercl.sirv.com/map.jpg";
    mapSprite.width = 400;
    mapSprite.height = 600;

    var endLocation = new Image();
    endLocation.src = "https://img.icons8.com/avantgarde/452/marker.png";
    endLocation.height = 40;
    endLocation.width = 40;

    var marker = new Image();
    marker.src = "https://img.icons8.com/fluency/452/user-location.png";
    marker.height = 40;
    marker.width = 40;

    var livelocation;
    var signalSTR;


    // var Marker = function () {
    //     this.Sprite = new Image();
    //     this.Sprite.src = "https://img.icons8.com/fluency/452/user-location.png"
    //     this.Width = 40;
    //     this.Height = 40;
    //     this.XPos = 0;
    //     this.YPos = 0;
    // }

    var firstLoad = function () {
        context.font = "15px Segoe UI";
        context.textAlign = "center";
    }

    firstLoad();



    var draw = function (data1, data2) {

        context.fillStyle = "#000";

        context.drawImage(mapSprite, 120, 40, mapSprite.width, mapSprite.height);
        
        var input2 = document.forms["Bledata"]["input2"].value;
        destinationMark(input2);
        locationMarker(data1, data2);
    };

    var destinationMark = function (end) {
        if (end == "D1") {
            context.drawImage(endLocation, 162, 141, 40, 40);
        }
        else if (end == "D2") {
            context.drawImage(endLocation, 424, 388, 40, 40);
        }
        else if (end == "entrance") {
            context.drawImage(endLocation, 305, 69, 40, 40);
        }
        else if (end == "exit") {
            context.drawImage(endLocation, 299, 593, 40, 40);
        }

    };

    var locationMarker = function(data1, data2) {
        if (data1 == 1) {
            context.drawImage(marker, 305, 141, 40, 40);
        } else if (data1 == 2) {
            var ycood = (60 - (data2)) * 5;
            console.log(ycood);
            context.drawImage(marker, 305, ycood - 370, 40, 40);
        } else if (data1 == 3) {
            var ycood = (60 - (data2)) * 5;
            console.log(ycood);
            context.drawImage(marker, 305, ycood - 525, 40, 40);
        } else if (data1 == 4) {
            var ycood = (60 - (data2)) * 1.5;
            console.log(ycood);
            context.drawImage(marker, 305, 680 - ycood, 40, 40);

        }
    }

    var formSubmit = function () {
        let input1 = document.forms["Bledata"]["input1"].value;
        let input2 = document.forms["Bledata"]["input2"].value;
        console.log(input1);
        console.log(input2);

        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/get?input1=" + input1 + "&input2=" + input2, true);
        xhr.send();
        console.log("Data send to web server");
    };




    // Create events for the sensor readings
    if (!!window.EventSource) {
        var source = new EventSource('/events');

        source.addEventListener('open', function (e) {
            console.log("Events Connected");
        }, false);

        source.addEventListener('error', function (e) {
            if (e.target.readyState != EventSource.OPEN) {
                console.log("Events Disconnected");
            }
        }, false);

        source.addEventListener('beacon_data', function (e) {
            console.log("beacon_data", e.data);
            var obj = JSON.parse(e.data);
            var livelocation = obj.CurrLocation;
            var signalSTR = obj.Rssi;
            window.livelocation = livelocation;
            window.signalSTR = signalSTR;
            //console.log(livelocation);
            //console.log(signalSTR);
        }, false);
            
    }
    var main = function () {
            //console.log("main");
            //console.log(livelocation);
            //console.log(signalSTR);
        draw(livelocation, signalSTR);

    };

    // Refresh 60 times a second
    setInterval(main, (1000)/500);



</script>
)rawliteral";


void ledBlink(int timeDelay) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(timeDelay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(timeDelay);
}






// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  ledBlink(5);
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  char* buff = (char*) incomingData;
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

//Select closest BLE Scanner and select its data
if(UUID == UserUUID){
  if(prvMacStr == macStr){
    //data recive from same device
    UserDesti = devID;
    UserRssi = rssi;
  }else if(prvMacStr != macStr){
    if(prvRssi >= rssi){
      Serial.printf(" if 1 Closest pointer is :%d", devID);
      UserDesti = prvDevID;
      UserRssi = prvRssi;
    }else{
      Serial.printf("if 2 Closest pointer is :%d", prvDevID);
      UserDesti = devID;
      UserRssi = rssi;
    }
    ledBlink(15);
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
  }
}

//Data sending to users UI for calculations, server events.
String sendData(){
  rowData["CurrLocation"] = UserDesti;
  rowData["Rssi"] = UserRssi;
  serializeJson(rowData, serverData);
  Serial.println(serverData);
  return serverData;
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);

  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
    ledBlink(20);
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());


  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>&state=<inputMessage2>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
    }
    else {
      inputMessage1 = "No Data added";
      inputMessage2 = "No Location added";
    }
    Serial.println(inputMessage1);
    Serial.println(inputMessage2);
    UserUUID = inputMessage1;

    
    request->send(200, "text/plain", "OK");
  });
  

  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();


}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 50;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send(sendData().c_str(),"beacon_data",millis());
    serverData.clear();
    lastEventTime = millis();
  }

}
