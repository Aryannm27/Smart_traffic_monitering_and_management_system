#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
SoftwareSerial s(D1, D2);

const char* ssid     = "DESKTOP123"; //your ssid
const char* password = "ldceam1234";  //your wifi password

unsigned long myChannelNumber = 1874871;  // thingspeak channal no.
const char * myWriteAPIKey = "FE3CZ6HVN9S06BJT"; // thingspeak write api key

int sensor_count[4] = {0,0,0,0};
WiFiClient client;

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 15L * 1000L;

bool json_serial() {
  s.write(50);
  StaticJsonDocument<1000> jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, s);
  if (error) {
    Serial.println("Invalid...");
    return false;
  }
  sensor_count[0] = jsonBuffer["data1"];
  sensor_count[1] = jsonBuffer["data2"];
  sensor_count[2] = jsonBuffer["data3"];
  sensor_count[3] = jsonBuffer["data4"];

  Serial.println("Json received and parsed");
  serializeJsonPretty(jsonBuffer, Serial);
  Serial.println("");
  Serial.println("------------xxxxxxx-----------");
  return true;
}


void setup() {
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(9600);
  s.begin(9600);
  while (!Serial) {
    Serial.println("...");
    continue;
  }
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);

}

void publishData() {
  if (millis() - lastConnectionTime > postingInterval) {
     PublishFeed();
  }
}

void loop() {
  if (json_serial()) {
    publishData();
  }
}

void PublishFeed(){
  for(int cn = 0; cn<4; cn++){
    ThingSpeak.setField(cn+1, sensor_count[cn]);
  }
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  lastConnectionTime = millis();
}
