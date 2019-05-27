#include <driver/adc.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "GravityTDS.h"

#include "SSD1306Wire.h"
#include <Wire.h>

#define TurbPin A0
SSD1306Wire  display(0x3c, 5, 4);

#define TdsSensorPin A0
GravityTDS gravityTds;

void setupDisp() {
  display.init(); // initialise the OLED
  display.flipScreenVertically(); // does what is says
  display.setFont(ArialMT_Plain_24); // does what is says
  // Set the origin of text to top left
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

//Credentials
const char* ssid = "MANGSAVE";
const char* password = "P455w0rd12345";
const char* mqtt_server = "192.168.0.200";
const char* brokerUser = "mangsave";
const char* brokerPass = "1234";
const char* outTopic = "mang/tds";

float temperature = 25.0, tdsValue = 0;


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
  pinMode(TdsSensorPin, INPUT);
  client.setServer(mqtt_server, 1883);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_6);
  setupDisp();
}


void loop() {

  //temperature = readTemperature();  //add your temperature sensor and read it
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tdsValue = gravityTds.getTdsValue();  // then get the value
  Serial.print(tdsValue, 0);
  Serial.println("ppm");
  delay(1000);
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    snprintf (msg, 50, "%f", tdsValue);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);

  }

  display.clear(); // clear the display
  // prep a string in the screen buffer
  display.drawString(0, 0, "TDS" + String(tdsValue));
  display.drawString(0, 24, "TDS" + String(analogRead(A0)));
  display.display(); // display whatever is in the buffer
}
