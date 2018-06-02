#include "config.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

const int BUFFER_SIZE = JSON_OBJECT_SIZE(20);
const int LED_PIN = D3;

bool stateOn = false;
byte red = 0;
byte green = 0;
byte blue = 0;
byte brightness = 255;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_mqtt() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

void setup_led() {
  strip.begin();
  setColor(strip.Color(0, 0, 0));
  strip.show();
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if(!processJson(message)) {
    return;
  }

  if(!stateOn) {
    setColor(strip.Color(0, 0, 0));
  } else {
    setColor(strip.Color(red, green, blue));
  }
  sendState();
}

void setColor(uint32_t c) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

bool processJson(char* message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(message);

  if(!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }

  if(root.containsKey("state")) {
    if(strcmp(root["state"], "ON") == 0) {
      stateOn = true;
    } else if(strcmp(root["state"], "OFF") == 0) {
      stateOn = false;
    }
  }

  if (root.containsKey("color")) {
    red = root["color"]["r"];
    green = root["color"]["g"];
    blue = root["color"]["b"];
  }
  
  if (root.containsKey("brightness")) {
    brightness = root["brightness"];
  }
  return true;
}

void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["state"] = (stateOn) ? "ON" : "OFF";

  if(stateOn) {
    JsonObject& color = root.createNestedObject("color");
    color["r"] = red;
    color["g"] = green;
    color["b"] = blue;

    root["brightness"] = brightness;
  }

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));
  client.publish(MQTT_TOPIC_STATE, buffer, true);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWD, MQTT_TOPIC_STATE, 1, 1, MQTT_WILL_MESSAGE)) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_CMD, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void setup() {
  pinMode(4, INPUT_PULLUP);
  pinMode(14, OUTPUT);

  Serial.begin(115200);
  setup_wifi();
  setup_mqtt();
  setup_led();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(100);
}
