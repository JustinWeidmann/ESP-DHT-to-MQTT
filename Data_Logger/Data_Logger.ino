#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// Update these with values suitable for your network.

const char* ssid = "*************";
const char* password = "****************";
const char* mqtt_server = "192.168.1.11";

DHTesp dht;
int DHTPin = 5; //Sensor Pin
int checkInterval = 30000;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

String tempStr;
String humidityStr;
char tempOut[50];
char humOut[50];

const int led = 2;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      
      client.publish("JustinsRoom/DHT/$name", "Data Logger A.1.0"); //Project title
      client.subscribe("JustinsRoom/DHT/$go");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  String thisBoard= ARDUINO_BOARD;
  dht.setup(DHTPin, DHTesp::DHT11);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
  digitalWrite(BUILTIN_LED, HIGH);  //LED off
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  checkSensor();

  unsigned long now = millis();
}

void checkSensor(){
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float fTemp = dht.toFahrenheit(temperature);
  //float fTemp = dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true);

  humidityStr = String(humidity);
  humidityStr.toCharArray(humOut, humidityStr.length() + 1);

  tempStr = String(fTemp);
  tempStr.toCharArray(tempOut, tempStr.length() + 1);
  //Serial.println(humOut);
  //Serial.println(tempOut);
  
  client.publish("JustinsRoom/DHT/$humidity", humOut, true);
  client.publish("JustinsRoom/DHT/$temp", tempOut, true);

  delay(checkInterval);
}
