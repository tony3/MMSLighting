#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <inttypes.h>
#include <ESP8266WiFi.h>
#include <MCP23S17.h>
#include "MCP_IO.h"
#include "LightZone.h"
#include "Timestamp.h"
#include <AsyncMqttClient.h>

#define LIVE_MAKERSPACE
#include "Credentials.h"

const int Wifi_LED = 5;
const int Mqtt_LED = 4;

const char ota_hostname[] PROGMEM = "LightMaster";

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char Mqtt_Server[] PROGMEM    = MQTT_SERVER;
const char Mqtt_Username[] PROGMEM  = MQTT_USERNAME;
const char Mqtt_Password[] PROGMEM  = MQTT_KEY;

//Cmd will ON/OFF
//Sts will be ON, OVERRIDDEN, PENDING_OFF, OFF

const char LZ1_Cmd_Topic[] PROGMEM = "Lighting/LZ1_Cmd";
const char LZ2_Cmd_Topic[] PROGMEM = "Lighting/LZ2_Cmd";
const char LZ3_Cmd_Topic[] PROGMEM = "Lighting/LZ3_Cmd";
const char LZ4_Cmd_Topic[] PROGMEM = "Lighting/LZ4_Cmd";
const char LZ5_Cmd_Topic[] PROGMEM = "Lighting/LZ5_Cmd";
const char LZ6_Cmd_Topic[] PROGMEM = "Lighting/LZ6_Cmd";
const char LZ7_Cmd_Topic[] PROGMEM = "Lighting/LZ7_Cmd";
const char LZ8_Cmd_Topic[] PROGMEM = "Lighting/LZ8_Cmd";

const char LZ1_Sts_Topic[] PROGMEM = "Lighting/LZ1_Sts";
const char LZ2_Sts_Topic[] PROGMEM = "Lighting/LZ2_Sts";
const char LZ3_Sts_Topic[] PROGMEM = "Lighting/LZ3_Sts";
const char LZ4_Sts_Topic[] PROGMEM = "Lighting/LZ4_Sts";
const char LZ5_Sts_Topic[] PROGMEM = "Lighting/LZ5_Sts";
const char LZ6_Sts_Topic[] PROGMEM = "Lighting/LZ6_Sts";
const char LZ7_Sts_Topic[] PROGMEM = "Lighting/LZ7_Sts";
const char LZ8_Sts_Topic[] PROGMEM = "Lighting/LZ8_Sts";


WiFiClient client;
AsyncMqttClient mqtt;

MCP ssrs(0, 15);
MCP buttons(1, 15);
MCP_IO mcp_io(&ssrs, &buttons);

LightZone lz1(mcp_io, 0, 8,  24, 16);
LightZone lz2(mcp_io, 1, 9,  25, 17);
LightZone lz3(mcp_io, 2, 10, 26, 18);
LightZone lz4(mcp_io, 3, 11, 27, 19);
LightZone lz5(mcp_io, 4, 12, 28, 20);
LightZone lz6(mcp_io, 5, 13, 29, 21);
LightZone lz7(mcp_io, 6, 14, 30, 22);
LightZone lz8(mcp_io, 7, 15, 31, 23);

LightZone* lz[8] = {&lz1, &lz2, &lz3, &lz4, &lz5, &lz6, &lz7, &lz8};

void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT Connected.");
  digitalWrite(Mqtt_LED, HIGH);
  
  mqtt.subscribe(LZ1_Cmd_Topic, 1);
  mqtt.subscribe(LZ2_Cmd_Topic, 1);
  mqtt.subscribe(LZ3_Cmd_Topic, 1);
  mqtt.subscribe(LZ4_Cmd_Topic, 1);
  mqtt.subscribe(LZ5_Cmd_Topic, 1);
  mqtt.subscribe(LZ6_Cmd_Topic, 1);
  mqtt.subscribe(LZ7_Cmd_Topic, 1);
  mqtt.subscribe(LZ8_Cmd_Topic, 1);
  
  mqtt.publish(LZ1_Sts_Topic, 1, true, lz1.GetStatusText());
  mqtt.publish(LZ2_Sts_Topic, 1, true, lz2.GetStatusText());
  mqtt.publish(LZ3_Sts_Topic, 1, true, lz3.GetStatusText());
  mqtt.publish(LZ4_Sts_Topic, 1, true, lz4.GetStatusText());
  mqtt.publish(LZ5_Sts_Topic, 1, true, lz5.GetStatusText());
  mqtt.publish(LZ6_Sts_Topic, 1, true, lz6.GetStatusText());
  mqtt.publish(LZ7_Sts_Topic, 1, true, lz7.GetStatusText());
  mqtt.publish(LZ8_Sts_Topic, 1, true, lz8.GetStatusText());
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("** Disconnected from the broker **");
  digitalWrite(Mqtt_LED, LOW);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
}

void onMqttUnsubscribe(uint16_t packetId) {
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (!topic || !payload) return;
  int zone = 0;
  char topic_test[sizeof(LZ1_Cmd_Topic)];
  strncpy(topic_test, LZ1_Cmd_Topic, sizeof(LZ1_Cmd_Topic));
  for(zone = 1; zone <= 8; ++zone)
  {
    topic_test[11] = zone + 48;
    if (strncmp(topic, topic_test, sizeof(LZ1_Cmd_Topic)) == 0)
    {
      if (strncasecmp(payload, "On", 2) == 0)
      {
        lz[zone - 1]->TurnOn();
      }
      if (strncasecmp(payload, "Off", 3) == 0)
      {
        lz[zone - 1]->StartPendingOff();
      }
      break;
    }
  }
}

void onMqttPublish(uint16_t packetId) {
}




int strncasecmp(const char* s1, const char* s2, int len)
{
  int i;
  for(i = 0; i < len && s1[i] && s2[i]; ++i)
  {
    if ((s1[i] | 32) != (s2[i] | 32))
    {
      return 1;
    }
  }

  return (!s1[i] && !s2[i] ? 0 : 1);
}

void setup() {

  Serial.begin(115200);
  delay(10);

  Serial.println("");
  Serial.println("");
  Serial.println("----------------------");
  Serial.println("MMS Master Light Panel");
  Serial.println("         v1.0         ");
  Serial.println("----------------------");

  for(int i = 0; i < 8; ++i)
  {
    lz[i]->Setup();
  }

  pinMode(Wifi_LED, OUTPUT);
  pinMode(Mqtt_LED, OUTPUT);
  digitalWrite(Wifi_LED, LOW);
  digitalWrite(Mqtt_LED, LOW);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("WiFi is connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  for(int i = 24; i < 32; ++i)
  {
    mcp_io.inputInvert(i, 1);
    
  }

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
  });
  
  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  Serial.println("Starting");
}

bool IsMQTTConnected()
{
  static Timestamp last_mqtt_attempt;
  static bool initial_mqtt = true;
  
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(Wifi_LED, HIGH);
    if (mqtt.connected()) 
    {
      digitalWrite(Mqtt_LED, HIGH);
      return true;
    }
    else
    {
      digitalWrite(Mqtt_LED, LOW);
      
      if (last_mqtt_attempt.Elapsed() > 10000 || initial_mqtt)
      {
        initial_mqtt = false;
        Serial.println("Trying to connect to MQTT Broker...");
        mqtt.connect();
        last_mqtt_attempt.Update();
      }
    }
  }
  else
  {
    digitalWrite(Wifi_LED, LOW);
    digitalWrite(Mqtt_LED, LOW);
    mqtt.disconnect();
  }

  return false;
}

void PrintState(const int i, const char* state)
{
  Serial.print("Zone #"),
  Serial.print(i, DEC);
  Serial.print(" state is now: ");
  Serial.println(state);
}

void loop()
{

  static bool initial_wifi = true;

  if (WiFi.status() == WL_CONNECTED)
  {
    if (initial_wifi)
    {
      ArduinoOTA.begin();
      Serial.println("OTA Begin");
      Serial.print("My IP address: ");
      Serial.println(WiFi.localIP());
      initial_wifi = false;
    }
    else
    {
      ArduinoOTA.handle();
    }
  }
  
  if (IsMQTTConnected())
  {

  }

  for(int i = 0; i < 8; ++i)
  {
    if (lz[i]->Update())
    {
        //Serial.println("Update");
        if (IsMQTTConnected()) 
        {
          char topic[sizeof(LZ1_Cmd_Topic)];
          strncpy(topic, LZ1_Cmd_Topic, sizeof(LZ1_Cmd_Topic));
          topic[11] = i + 1 + 48;
          mqtt.publish(topic, 1, true, lz[i]->GetStatusText());
        }
        PrintState(i, lz[i]->GetStatusText());
    }
  }
}
