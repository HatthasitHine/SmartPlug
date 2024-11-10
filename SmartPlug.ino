#include <ESP8266WiFi.h> //เป็นการเรียกใช้ Library โมดูล ESP8266 * บอร์ท esp8266 เลือกเวอร์ชั่นต่ำๆ 2.7.4
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>
//for configure wifi
#include "Arduino.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client; //Client ID
const char* mqtt_username; //Token
const char* mqtt_password; //Secret
const char* Device_name; //Device_name

WiFiClient espClient;
PubSubClient client(espClient);

char msg[50];
/*
#define R1 16  // กำหนด D0 คือขา 16
#define R2 5  // กำหนด  D1 คือขา 5 
#define R3 4
#define R4 0
#define R01 R1
#define R02 R2
#define R03 R3
#define R04 R4*/
//#define ledReStart 14
int R01 = D5;
int R02 = D6;
int R03 = D7;
int R04 = D8;

//configure wifi varialble
//192.168.4.1/setWIFI
ESP8266WebServer server(80);
bool testWifi(void);
int buttonPin = D0; //SD3
int buttonState = 0;

//configure wifi function
//test wifi
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

//POST API
void setWIFI() {
  String postBody = server.arg("plain");
  Serial.println(postBody);

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, postBody);
  if (error) {
    // if the file didn't open, print an error:
    Serial.print(F("Error parsing JSON "));
    Serial.println(error.c_str());

    String msg = error.c_str();

    server.send(400, F("text/html"),
                "Error in parsin json body! <br>" + msg);

  } else {
    JsonObject postObj = doc.as<JsonObject>();

    Serial.print(F("HTTP Method: "));
    Serial.println(server.method());

    if (server.method() == HTTP_POST) {
      if (postObj.containsKey("mqtt_Client") && postObj.containsKey("mqtt_username") && postObj.containsKey("mqtt_password") && postObj.containsKey("ssid") && postObj.containsKey("password")) {
        Serial.println(F("done."));
        String qsid = postObj["ssid"]; //เก็บชื่อWifi
        String qpass = postObj["password"]; //เก็บ password Wifi
        String qmqtt_Client = postObj["mqtt_Client"]; //เก็บClient ID
        String qmqtt_username = postObj["mqtt_username"]; //เก็บToken
        String qmqtt_password = postObj["mqtt_password"]; //เก็บSecret*/
        String qDevice_name = postObj["Device_name"]; //เก็บDevice_name*/
        
        Serial.println("clearing eeprom");
        for (int i = 0; i < 300; ++i) { //for (int i = 0; i < 300; ++i)
          EEPROM.write(i, 0);
        }

        Serial.println("writing eeprom mqtt_Client:");
        for (int i = 0; i < qmqtt_Client.length(); ++i)
        {
          EEPROM.write(i, qmqtt_Client[i]);
          Serial.print("Wrote: ");
          Serial.println(qmqtt_Client[i]);
        }
        Serial.println("writing eeprom mqtt_username:");
        for (int i = 0; i < qmqtt_username.length(); ++i)
        {
          EEPROM.write(36 + i, qmqtt_username[i]);
          Serial.print("Wrote: ");
          Serial.println(qmqtt_username[i]);
        }
        Serial.println("writing eeprom mqtt_password:");
        for (int i = 0; i < qmqtt_password.length(); ++i)
        {
          EEPROM.write(68 + i, qmqtt_password[i]);
          Serial.print("Wrote: ");
          Serial.println(qmqtt_password[i]);
        }
        ///////////////////////////////////////
        Serial.println("writing eeprom Device_name:");
        for (int i = 0; i < qDevice_name.length(); ++i)
        {
          EEPROM.write(101 + i, qDevice_name[i]);
          Serial.print("Wrote: ");
          Serial.println(qDevice_name[i]);
        }
        ///////////////////////////////////////
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(150 + i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }

        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(200 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }

        EEPROM.commit();

        DynamicJsonDocument doc(512);
        doc["status"] = "OK";

        Serial.print(F("Stream..."));
        String buf;
        serializeJson(doc, buf);

        server.send(201, F("application/json"), buf);
        Serial.print(F("done."));

        delay(2000);
        ESP.restart();

      } else {
        DynamicJsonDocument doc(512);
        doc["status"] = "KO";
        doc["message"] = F("No data found, or incorrect!");

        Serial.print(F("Stream..."));
        String buf;
        serializeJson(doc, buf);

        server.send(400, F("application/json"), buf);
        Serial.print(F("done."));
      }
    }
  }
}


// Define routing
void restServerRouting() {
  Serial.print("restServerRouting here!");
  // handle post request
  server.on(F("/setWIFI"), HTTP_POST, setWIFI);
}

void resetEEprom() {
  Serial.println("reset eeprom");
  for (int i = 0; i < 300; ++i) { //for (int i = 0; i < 300; ++i)
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  ESP.restart();
}
//------------------------------------------
//netpie function

void reconnect() {
  String emqtt_Client = ""; //mqtt_Client
  for (int i = 0; i < 36; ++i)
  {
    emqtt_Client += char(EEPROM.read(i));
  }
  Serial.print("mqtt_Client: ");
  //Serial.println(emqtt_Client);
  mqtt_Client = emqtt_Client.c_str();
  Serial.println(mqtt_Client);
  
  String emqtt_username = ""; //mqtt_username
  for (int i = 36; i < 68; ++i)
  {
    emqtt_username += char(EEPROM.read(i));
  }
  Serial.print("mqtt_username: ");
  //Serial.println(emqtt_username);
  mqtt_username = emqtt_username.c_str();
  Serial.println(mqtt_username);

  String emqtt_password = ""; //mqtt_password
  for (int i = 68; i < 101; ++i)
  {
    emqtt_password += char(EEPROM.read(i));
  }
  Serial.print("mqtt_password: ");
  //Serial.println(emqtt_password);
  mqtt_password = emqtt_password.c_str();
  Serial.println(mqtt_password);
  
  while (!client.connected()) {
    Serial.println(mqtt_Client);
    //Serial.println(WiFi.localIP());
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
  //////////////////////////////////////////////////////////////////
  String eDevice_name = ""; //Device_name
  for (int i = 101; i < 150; ++i)
  {
    eDevice_name += char(EEPROM.read(i));
  }
  Serial.print("Device_name: ");
  //Serial.println(eDevice_name);
  Device_name = eDevice_name.c_str();
  Serial.println(Device_name);
  ////////////////////////////////////////////////////////////////////
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  tpc = getMsg(topic, message);
  tpc.toCharArray(msg, (tpc.length() + 1));
  client.publish("@shadow/data/update", msg);
}
void ReadEEProm() {
  Serial.println("Reading EEPROM ssid");
  String esid = ""; //Wifi name
  for (int i = 150; i < 200; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = ""; //Wifi password
  for (int i = 200; i < 251; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);


  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");

    Serial.print("SSID: ");
    Serial.println(esid);
    Serial.print("PASS: ");
    Serial.println(epass);

    Serial.println("");
    Serial.println("Wi-Fi connected"); //แสดงว่าเชื่อมต่อ Wi-Fi ได้แล้ว
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP()); //แสดง IP ของบอร์ดที่ได้รับแจกจาก AP
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    return; // **************************************************************************************************************
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    //WiFi.mode(WIFI_STA);
    WiFi.disconnect();
  }

  WiFi.mode(WIFI_AP); // ใช้งาน WiFi ในโหมด AP
  WiFi.softAP("SmartPlug_Configure_WiFi"); // ตั้งให้ชื่อ WiFi เป็น ESP8266_NEPIE

  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");

  while ((WiFi.status() != WL_CONNECTED))
  {
    //Serial.print(".");
    delay(100);
    server.handleClient();
  }
}
//-------------------------------------------------------------------------------
void setup() {
  pinMode(R01, OUTPUT); digitalWrite(R01, LOW); //HIGH
  pinMode(R02, OUTPUT); digitalWrite(R02, LOW);
  pinMode(R03, OUTPUT); digitalWrite(R03, LOW);
  pinMode(R04, OUTPUT); digitalWrite(R04, LOW);
  //pinMode(ledReStart, OUTPUT); // กำหนดขาทำหน้าที่ให้ขา D2 เป็น OUTPUT
  pinMode(buttonPin, INPUT_PULLUP);// กำหนดขาทำหน้าที่ให้ขา D1 เป็น INPUT รับค่าจากสวิตช์
  //digitalWrite(ledReStart, LOW);
  Serial.begin(115200);
  //onAutoConnWifi(""); //เชื่อมต่อAuto และ ตั้งค่าการเชื่อมต่อเมื่อเจอเครือข่ายใหม่ โดย IP Access point 192.168.4.1
  //WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM

  Serial.println("Startup");
  ReadEEProm();
  //---------------------------------------- Read EEPROM for SSID and pass

  //  Serial.println("");
  //  Serial.println("Wi-Fi connected"); //แสดงว่าเชื่อมต่อ Wi-Fi ได้แล้ว
  //  Serial.print("IP Address : ");
  //  Serial.println(WiFi.localIP()); //แสดง IP ของบอร์ดที่ได้รับแจกจาก AP
  //  client.setServer(mqtt_server, mqtt_port);
  //  client.setCallback(callback);
}

void loop() {
  //------Check connection------
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
  //---------------------------------
  //reset wifi
  buttonState = digitalRead(buttonPin); // อ่านค่าสถานะขาD0
  //Serial.println(buttonState);
  if (buttonState == LOW) { //กำหนดเงื่อนไขถ้าตัวแปล buttonState เก็บ ค่า 1(HIGH) ให้ทำในปีกกา **ถ้าใช้กับตัวกล่องให้เปลี่ยนเป็น HIGH**
    //digitalWrite(ledReStart, HIGH); // ไฟ LED 1ติด
    Serial.print("bt : ");
    Serial.println(buttonState);
    resetEEprom();
  }
  else { //ถ้าตัวแปล buttonState ไม่ได้เก็บ ค่า 1(HIGH) คือ ตัวแปล buttonState เก็บค่า 0(LOW) อยู่ ให้ทำปีกกาข้างล่าง
    //digitalWrite(ledReStart, LOW); // ไฟ LED 1ดับ
  }
}

String getMsg(String topic_, String message_) {     //datasources["???"]["shadow"]["Relay1"]     Indicator Light : datasources["???"]["shadow"]["Relay1"]=='1' //datasources["???"]["shadow"]["Relay2"]     Indicator Light : datasources["???"]["shadow"]["Relay2"]=='1'
  String tpc;
  if (topic_ == "@msg/Relay1") {
    if (message_ == "R01_ON") {                  //netpie["???"].publish("@msg/Relay1","R01_ON")
      digitalWrite(R01, HIGH);
      tpc = "{\"data\": {\"Relay1\":1}}";        //datasources["???"]["shadow"]["Relay1"]=='1'
      Serial.println("R01_ON");
    } else if (message_ == "R01_OFF") {          //netpie["???"].publish("@msg/Relay1","R01_OFF")
      digitalWrite(R01, LOW);
      tpc = "{\"data\": {\"Relay1\":0}}";       //datasources["???"]["shadow"]["Relay1"]=='0'
      Serial.println("R01_OFF");
    }
  }
  else if (topic_ == "@msg/Relay2") {
    if (message_ == "R02_ON") {                  //netpie["???"].publish("@msg/Relay2","R02_ON")
      digitalWrite(R02, HIGH);
      tpc = "{\"data\": {\"Relay2\":1}}";        //datasources["???"]["shadow"]["Relay2"]=='1'
      Serial.println("R02_ON");
    } else if (message_ == "R02_OFF") {          //netpie["???"].publish("@msg/Relay2","R02_OFF")
      digitalWrite(R02, LOW);
      tpc = "{\"data\": {\"Relay2\":0}}";       //datasources["???"]["shadow"]["Relay2"]=='0'
      Serial.println("R02_OFF");
    }
  }
  else if (topic_ == "@msg/Relay3") {
    if (message_ == "R03_ON") {                  //netpie["???"].publish("@msg/Relay3","R03_ON")
      digitalWrite(R03, HIGH);
      tpc = "{\"data\": {\"Relay3\":1}}";        //datasources["???"]["shadow"]["Relay3"]=='1'
      Serial.println("R03_ON");
    } else if (message_ == "R03_OFF") {          //netpie["???"].publish("@msg/Relay3","R03_OFF")
      digitalWrite(R03, LOW);
      tpc = "{\"data\": {\"Relay3\":0}}";       //datasources["???"]["shadow"]["Relay3"]=='0'
      Serial.println("R03_OFF");
    }
  }
  else if (topic_ == "@msg/Relay4") {
    if (message_ == "R04_ON") {                  //netpie["???"].publish("@msg/Relay4","R04_ON")
      digitalWrite(R04, HIGH);
      tpc = "{\"data\": {\"Relay4\":1}}";        //datasources["???"]["shadow"]["Relay4"]=='1'
      Serial.println("R04_ON");
    } else if (message_ == "R04_OFF") {          //netpie["???"].publish("@msg/Relay4","R04_OFF")
      digitalWrite(R04, LOW);
      tpc = "{\"data\": {\"Relay4\":0}}";       //datasources["???"]["shadow"]["Relay4"]=='0'
      Serial.println("R04_OFF");
    }
  }
  return tpc;
}
