#include <driver/adc.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "SSD1306Wire.h"
#include <Wire.h>  

#define TurbPin A6
SSD1306Wire  display(0x3c, 5, 4);


//Credentials
const char* ssid = "MANGSAVE";
const char* password = "P455w0rd12345";
const char* mqtt_server = "192.168.0.200";
const char* brokerUser = "mangsave";
const char* brokerPass = "1234";
const char* outTopic = "mang/turb";
  
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
char nmsg[50];
int value = 0;
float ntu = 0.0;

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
    if (client.connect("TurBMeter", brokerUser, brokerPass)) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Turb Connected");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds befo re retrying
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
  client.setServer(mqtt_server,1883);
  pinMode(TurbPin,INPUT);
  setupDisp();
}


float readTurbidity(){
  int sensorValue = analogRead(A6);// read the input on analog pin 0:
  float voltage = (sensorValue * (5.0 / 4095.0)); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  
  Serial.println(voltage); // print out the value you read:
  delay(500);

  

  float x = voltage;
  
  if (x < 2.5){
    ntu = 3000.0;
    }
  else if(x > 4.2){
    ntu = 0;
    } 
   else
    {
    ntu = ((-1120.4 * x * x) + ((5742.3) * x) - (4352.9));
    }
  Serial.println(ntu);
  
  return voltage;
  }

void loop() {
  // put your main code here, to run repeatedly:
  float volt = 0.0;
 
  float turb = ntu;
  if(!client.connected()){
    reconnect();
  }
  volt = readTurbidity();
  Serial.println(volt);
  client.loop();

 long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    snprintf (msg, 50, "%f",volt);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);
    snprintf (nmsg, 50, "%f",turb);
     client.publish("mang/ntu", nmsg);
  }

    display.clear(); // clear the display
    // prep a string in the screen buffer
    display.drawString(0, 0, "Voltage" + String(volt));
    display.drawString(0, 24, "Turbidity" + String(turb));
    display.display(); // display whatever is in the buffer

}
