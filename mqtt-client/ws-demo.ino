// demo based on Seeed K1100 Terminal
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include "rpcWiFi.h"

// from: https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

#include "Wire.h"
#include "SPI.h"

// WIFI setting
char wifiSsid[] = "";
// Wifi password.
char wifiPass[] = "";

// MQTT setting
const char *ID = "wsdemokit";  // Name of our device, must be unique
const char *TOPIC = "demo";  // Topic to subcribe to, w3bstream project name
const char *subTopic = "inTopic";  // Topic to subcribe to
const char *server = "192.168.1.15"; // Server URL

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(TOPIC, "{\"message\": \"Wio Terminal is connected!\",\"payload\": \"XZY\"}");
      Serial.println("Published connection message successfully!");
      // ... and resubscribe
      client.subscribe(subTopic);
      Serial.print("Subcribed to: ");
      Serial.println(subTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Let's use the led to signal when the device is communicating
  
  Serial.begin(115200);
  while (!Serial);

  Serial.println(".....::::: APP STARTED :::::......");
  Serial.println("W3bstream DEMO");
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  initWiFi();

  Serial.print("Syncing time...");
  timeClient.begin();
  timeClient.update();
  Serial.print(timeClient.getFormattedTime() + "- ");
  Serial.println(timeClient.getEpochTime());


  Serial.print("Connecting to the MQTT broker on " + String(server));
  // if (!initMqtt()) {
  //   Serial.println("Failed to connect to the MQTT broker.");
  //   while (1);
  // }
  client.setServer(server, 1883);
  client.setCallback(callback);

  Serial.print("MQTT Topic: " + String(TOPIC));
}

void loop() {
  // Update steps taken

 if (!client.connected()){
    reconnect();
  }

  if (digitalRead(WIO_5S_PRESS) == LOW) {
     Serial.println("A Key pressed");

    // Get timestamp from Internet
    String timestamp = String(timeClient.getEpochTime()); 
    
    String message = buildMessage(1, timestamp);

    // Sending the message over MQTT protocol
    // Serial.print("Sending mqtt message: ");
    // Serial.println(message);
    // mqttClient.beginMessage(topic);
    // mqttClient.print(message);
    // mqttClient.endMessage();

    // if (!client.publish(TOPIC, message.c_str())) {
    if (!client.publish(TOPIC, "{\"message\": \"pressed!\",\"payload\": \"pressed\"}")) {
      Serial.println("Message failed to send.");
    }
  }

  client.loop();
  delay(500);
}

// Connects to the wifi network.
void initWiFi() {

    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
 
    WiFi.begin(wifiSsid, wifiPass);
    Serial.print(F("Connecting to WiFi .."));
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
        WiFi.begin(wifiSsid, wifiPass);
    }
    Serial.println(F("\r\nConnected. IP: "));
    Serial.println(WiFi.localIP());
}

// Builds a data message given steps taken and timestamp.
String buildMessage(int times, String timestamp) {
  Serial.print("Building message:");
  String message = "{\"header\": {\"event_type\": 1, ";
  message += "\"pub_id\": \"demokey\",";
  message += "\"pub_time\": " + timestamp + ",";
  message +="\"token\": \"token\"},";
  message +="\"payload\": \"x\"}";
  Serial.println(message);
  return message;
}
