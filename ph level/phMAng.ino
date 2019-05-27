#include <driver/adc.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "SSD1306Wire.h"
#include <Wire.h>
SSD1306Wire  display(0x3c, 5, 4);


#define analogInPin A6
float calibration = 10.00;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10], temp;


//Credentials
const char* ssid = "MANGSAVE";
const char* password = "P455w0rd12345";
const char* mqtt_server = "192.168.0.200";
const char* brokerUser = "mangsave";
const char* brokerPass = "1234";
const char* outTopic = "mang/ph";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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
    if (client.connect("ph", brokerUser, brokerPass)) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "ph Connected");
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

void setupDisp() {
  display.init(); // initialise the OLED
  display.flipScreenVertically(); // does what is says
  display.setFont(ArialMT_Plain_24); // does what is says
  // Set the origin of text to top left
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setupWifi();
  client.setServer(mqtt_server, 1883);
  pinMode(analogInPin,INPUT);
  setupDisp();

}


void loop() {

  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(analogInPin);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];
  float pHVol = (float)avgValue * 5.0 / 4096 / 6;
  float phValue = (-5.70 * pHVol + calibration);
  Serial.print("sensor = ");
  Serial.println(phValue);

  display.clear(); // clear the display
  // prep a string in the screen buffer
  display.drawString(0, 0, "pH: " + String(phValue));
  display.display(); // display whatever i
  delay(500);

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  long now = millis();

  if (now - lastMsg > 5000) {
    lastMsg = now;
    snprintf (msg, 50, "%f", phValue);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);
  }
}
