#include <ESP8266WiFi.h>
#include <aJSON.h>
//=============  此处必须修该============
String DEVICEID="*****"; // 你的设备编号 ==
String  APIKEY = "*****"; // 设备密码==
//==================BIGIot===================
unsigned long lastCheckInTime = 0; //记录上次报到时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次

const char* ssid     = "DSP_LAB";//无线名称
const char* password = "19921114";//无线密码

const char* host = "www.bigiot.net";
const int httpPort = 8181;

//===================Led=================
int LED_Pin = 2;
void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_Pin,OUTPUT);
  digitalWrite(LED_Pin,HIGH);
  WiFi.begin(ssid, password);
}

WiFiClient client;

void loop() {

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Use WiFiClient class to create TCP connections
  if (!client.connected()) {
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      delay(5000);
      return;
    }
  }

  if(millis() - lastCheckInTime > postingInterval || lastCheckInTime==0) {
    checkIn();
  }
  
  // Read all the lines of the reply from server and print them to Serial
  if (client.available()) {
    String inputString = client.readStringUntil('\n');
    inputString.trim();
    Serial.println(inputString);
    int len = inputString.length()+1;
    if(inputString.startsWith("{") && inputString.endsWith("}")){
      char jsonString[len];
      inputString.toCharArray(jsonString,len);
      aJsonObject *msg = aJson.parse(jsonString);
      processMessage(msg);
      aJson.deleteItem(msg);          
    }
  }
}

void processMessage(aJsonObject *msg){
  aJsonObject* method = aJson.getObjectItem(msg, "M");
  aJsonObject* content = aJson.getObjectItem(msg, "C");     
  aJsonObject* client_id = aJson.getObjectItem(msg, "ID");
  if (!method) {
    return;
  }
    String M = method->valuestring;
    if(M == "say"){
      String C = content->valuestring;
      String F_C_ID = client_id->valuestring;
      if(C == "play"){
        Serial.println("********play********");
        digitalWrite(LED_Pin,LOW);
        sayToClient(F_C_ID,"LED All on!");    
      }else if(C == "stop"){
        Serial.println("********stop********");
        digitalWrite(LED_Pin,HIGH);
        sayToClient(F_C_ID,"LED All off!");    
      }else{
        int pin = C.toInt();
        Serial.println("********show********");             
        sayToClient(F_C_ID,"LED pin:"+pin); 
      }
    }
}

void checkIn() {
    String msg = "{\"M\":\"checkin\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";
    client.print(msg);
    lastCheckInTime = millis(); 
}

void sayToClient(String client_id, String content){
  String msg = "{\"M\":\"say\",\"ID\":\"" + client_id + "\",\"C\":\"" + content + "\"}\n";
  client.print(msg);
  lastCheckInTime = millis();
}
