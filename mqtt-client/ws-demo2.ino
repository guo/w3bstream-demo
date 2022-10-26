/**
 * @file main.cpp
 * @author IoTeX
 * @brief  The w3bstream example,refer to https://github.com/iotexproject/w3bstream
 * @version 0.1
 * @date 2022-10-19
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>
#include "rpcWiFi.h"
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

using namespace std;
// Config
const char SSID[] = "";
const char PASSWORD[] = "";

IPAddress W3BSTREAM_SERVER_IP(00,00,00,00);
String event_type = "2147483647";
String pub_id = "3892733651480578";
String token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJQYXlsb2FkIjoiMzg4NzE4NjYwODkzMDgxOCIsImlzcyI6InNydi1hcHBsZXQtbWdyIiwiZXhwIjoxNjY2NjAxOTI0fQ.Y7ouTsBiKS6aKV1-tPnTcrVCkDQkufHTZd_OA8JyPxE";
String project_name = "click2earn";

// Config
String srv_applet_mgr_event_uri = "http://" + W3BSTREAM_SERVER_IP.toString() + ":8888/srv-applet-mgr/v0/event/" + project_name;
unsigned long lastMillis = 0;

WiFiClient net;
HTTPClient http;
WiFiUDP ntpUDP;
PubSubClient client(net);
NTPClient timeClient(ntpUDP);

void connect()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
        WiFi.begin(SSID, PASSWORD);
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected mqtt");
        }
        else
        {
            Serial.print("failed connect mqtt, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);

    Serial.print("checking wifi...");
    WiFi.begin(SSID, PASSWORD);
    client.setServer(W3BSTREAM_SERVER_IP, 1883);
    connect();
    Serial.println("\nconnected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    timeClient.begin();
}

void send_event_w3bstream_http()
{

    http.addHeader("Content-Type", "application/json");
    http.addHeader("event_type", event_type);
    http.addHeader("pub_id", pub_id);
    http.addHeader("pub_time", String(timeClient.getEpochTime()));
    http.addHeader("token", token);
    http.begin(srv_applet_mgr_event_uri);
    int httpCode = http.POST("{\"payload\":\"click\"}");
    if (httpCode > 0)
    {
        String resBuff = http.getString();
        Serial.println(resBuff);
    }
    else
    {
        Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
    }
}

void send_event_w3bstream_mqtt()
{
    JSONVar mqtt_payload_json;
    JSONVar header_json;
    // header_json["event_type"] = event_type.toInt();
    // header_json["pub_id"] = pub_id;
    // header_json["pub_time"] = timeClient.getEpochTime();
    // header_json["token"] = token;
    mqtt_payload_json["header"] = header_json;
    mqtt_payload_json["payload"] = "click";
    String jsonString = JSON.stringify(mqtt_payload_json);
    int str_len = jsonString.length() + 1;
    char char_array[str_len];
    jsonString.toCharArray(char_array, str_len);
    Serial.printf(char_array);
    // the project name is the topic
    client.beginPublish(project_name.c_str(), str_len - 1, false);
    for (int i = 0; i < str_len; i++)
    {
        if (char_array[i] != 0)
        {
            client.print(char_array[i]);
        }
    }
    client.endPublish();
    /*
    payload like
    {
         "header" : {
             "event_type" : 2147483647,
             "pub_id" : "",
             "pub_time" : 1666087623393,
             "token" : ""
         },
        "payload" : "from esp32 mqtt"
     }*/
}

void loop()
{
    timeClient.update();
    if (!client.connected())
    {
        reconnect();
    }
    
  if (digitalRead(WIO_5S_PRESS) == LOW) {
     Serial.println("A Key pressed");

    // Get timestamp from Internet
    String timestamp = String(timeClient.getEpochTime()); 

    // Sending the message over MQTT protocol
    // Serial.print("Sending mqtt message: ");
    // Serial.println(message);
    // mqttClient.beginMessage(topic);
    // mqttClient.print(message);
    // mqttClient.endMessage();

    // if (!client.publish(TOPIC, message.c_str())) {
    send_event_w3bstream_mqtt();   
    // send_event_w3bstream_http(); 
  }

  client.loop();
  delay(500);
}
