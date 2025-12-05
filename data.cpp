#include <ESP8266WiFi.h> 
#include <PubSubClient.h> 
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <Wire.h> 
#include "MAX30100_PulseOximeter.h" 
#include "UbidotsESPMQTT.h" 
#define WIFISSID "Keerthiii" // WiFi SSID 
#define PASSWORD "qwerty12345678" // WiFi password 
#define TOKEN "BBUS-t3FPlKXprOMiaoTgH0RKeqao7hS7vp" // Ubidots TOKEN 
#define MQTT_CLIENT_NAME "1234a5d6798" // MQTT client name 
#define VARIABLE_LABEL "ecg" // Variable label 
#define DEVICE_LABEL "keerthi" // Device label 
#define SENSOR A0 // Analog sensor connected to A0 
#define LO_MINUS D5 // LO- connected to pin D5 
#define LO_PLUS D6 // LO+ connected to pin D6 
#define DATA_PIN D4 // Pin where the DS18B20 data line is connected 
#define BUZZER_PIN D7 // Pin connected to the buzzer 
char mqttBroker[]  = "industrial.api.ubidots.com"; 
char payload[100]; 
17 
char topic[150]; 
char str_sensor[10]; 
float temp; 
OneWire ourWire(DATA_PIN); 
DallasTemperature sensors(&ourWire); 
WiFiClient ubidots; 
Ubidots ubidotsClient(TOKEN, MQTT_CLIENT_NAME); 
PulseOximeter pox; 
bool max30100_initialized = false; 
void onBeatDetected() { 
Serial.println("Beat!"); 
} 
void callback(char* topic, byte* payload, unsigned int length) { 
// Handle callback 
} 
void reconnect() { 
while (!ubidotsClient.connected()) { 
Serial.println("Attempting MQTT connection..."); 
if (ubidotsClient.connect()) { 
Serial.println("Connected"); 
18 
} else { 
19 
 
      Serial.print("Failed to connect to Ubidots, try again in 2 seconds"); 
      delay(2000); 
    } 
  } 
} 
 
void setup() { 
  Serial.begin(115200); 
  WiFi.begin(WIFISSID, PASSWORD); 
  pinMode(SENSOR, INPUT); 
  pinMode(LO_MINUS, INPUT); 
  pinMode(LO_PLUS, INPUT); 
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output 
  sensors.begin(); 
 
  Serial.println(); 
  Serial.print("Waiting for WiFi..."); 
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print("."); 
    delay(500); 
  } 
   
  Serial.println(""); 
  Serial.println("WiFi Connected"); 
  Serial.println("IP address: "); 
Serial.println(WiFi.localIP()); 
ubidotsClient.setDebug(true); // Enable debug prints to serial monitor 
ubidotsClient.wifiConnection(WIFISSID, PASSWORD); 
ubidotsClient.begin(callback); 
// Initialize the MAX30100 sensor 
if (pox.begin()) { 
max30100_initialized = true; 
pox.setOnBeatDetectedCallback(onBeatDetected); 
Serial.println("MAX30100 initialized successfully."); 
} else { 
Serial.println("MAX30100 failed to initialize. Continuing without it."); 
} 
} 
void loop() { 
if (!ubidotsClient.connected()) { 
reconnect(); 
} 
sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL); 
sprintf(payload, "%s", ""); // Clean the payload 
20 
sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Add the variable label 
21 
 
 
  float sensor = analogRead(SENSOR); 
  dtostrf(sensor, 4, 2, str_sensor); 
   
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Add the value 
  Serial.println("Publishing data to Ubidots Cloud"); 
  ubidotsClient.add(VARIABLE_LABEL, sensor); // Insert your variable label and the 
value to be sent 
   
  sensors.requestTemperatures(); 
  temp = sensors.getTempCByIndex(0); 
  ubidotsClient.add("temp", temp); // Insert your temperature variable label and value 
   
  if (temp > 37.0) { 
    digitalWrite(BUZZER_PIN, HIGH); // Activate the buzzer 
  } else { 
    digitalWrite(BUZZER_PIN, LOW); // Deactivate the buzzer 
  } 
 
  // Read data from the MAX30100 sensor if initialized 
  if (max30100_initialized) { 
    pox.update(); 
    float BPM = pox.getHeartRate(); 
    float SPO2 = pox.getSpO2(); 
     
ubidotsClient.add("heart_rate", BPM); 
ubidotsClient.add("spo2", SPO2); 
} else { 
Serial.println("Skipping MAX30100 data as it is not initialized."); 
} 
ubidotsClient.ubidotsPublish(DEVICE_LABEL); // Publish the data to Ubidots under 
device name 'keerthi' 
delay(500); 
Serial.println("Loop end"); 
}