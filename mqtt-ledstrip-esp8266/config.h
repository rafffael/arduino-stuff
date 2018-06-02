/*
 * CONFIGURATION FILE
 */

// Wifi
#define WIFI_SSID "Wifi_SSID"
#define WIFI_PASSWD "wifi_password"

// MQTT
#define MQTT_SERVER "mqtt.server.com"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt_user"
#define MQTT_PASSWD "mqtt_password"
#define MQTT_TOPIC_CMD "home/ledstrip/set"
#define MQTT_TOPIC_STATE "home/ledstrip"
#define MQTT_WILL_MESSAGE "{\"state\": \"off\"}"
