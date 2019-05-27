#include <driver/adc.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SSD1306Wire.h"
#include <Wire.h>  


SSD1306Wire  display(0x3c, 5, 4);

#define ONE_WIRE_BUS 21

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

//Credentials
const char* ssid = "MANGSAVE";
const char* password = "P455w0rd12345";
const char* mqtt_server = "192.168.0.200";
const char* brokerUser = "mangsave";
const char* brokerPass = "1234";
const char* outTopic = "mang/temp";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setupDisp(){
    display.init(); // initialise the OLED
    display.flipScreenVertically(); // does what is says
    display.setFont(ArialMT_Plain_24); // does what is says
    // Set the origin of text to top left
    display.setTextAlignment(TEXT_ALIGN_LEFT);
}


void setupWifi() {

  delay(100);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.println("Connecting to ");
    Serial.println(mqtt_server);
    // Attempt to connect
    if (client.connect("TDSMeter", brokerUser, brokerPass)) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "TDS Connected");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setupWifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin();
  setupDisp();
}

float read_Temp() {

  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device is: ");
  float temp = sensors.getTempCByIndex(0);
  Serial.println(temp);
  return temp;

}

void loop() {
  float temp = read_Temp();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    snprintf (msg, 50, "%f", temp);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);
  }
   display.clear(); // clear the display
    // prep a string in the screen buffer
    display.drawString(0, 0, "Temp: " + String(temp) + "C");
 
    display.display(); // display whatever i
}
