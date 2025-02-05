#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <string.h>


#define DHTPIN D5
#define DHTTYPE DHT11
#define LED_PIN D6
#define FAN_PIN D7
const int lightpin = A0;
int ledState = LOW;
int fanState = LOW;
const char* ssid = "hellocacban";
const char* password = "hicacban";

const char* mqttServer = "192.168.1.25";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqttServer, mqttPort);
  

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT");
      //client.subscribe("DHT11");
      client.subscribe("button");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.setCallback(callback);
  dht.begin();
}


void loop() {
  client.loop();

  static unsigned long previousDHTReadTime = 0;
  unsigned long currentMillis = millis();
  const unsigned long DHTReadInterval = 1000; // Định kỳ đọc dữ liệu từ cảm biến DHT (1 giây)

  if (currentMillis - previousDHTReadTime >= DHTReadInterval) {
    previousDHTReadTime = currentMillis;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int light = analogRead(lightpin);
    String data = "DHT11|" + String(humidity) + "|" + String(temperature) + "|" + String(light);
    Serial.println(data);
    client.publish("sensor", data.c_str());
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "button") == 0) {
    String s = "";
    for (int i = 0; i < length; i++)
      s += (char)payload[i];
    if (s.equalsIgnoreCase("led|on")) {
      ledState = HIGH;
    }
    if (s.equalsIgnoreCase("led|off")) {
      ledState = LOW;
    }
    if (s.equalsIgnoreCase("fan|on")) {
      fanState = HIGH;
    }
    if (s.equalsIgnoreCase("fan|off")) {
      fanState = LOW;
    }
    digitalWrite(LED_PIN, ledState);
    digitalWrite(FAN_PIN, fanState);
    Serial.println(s);
    client.publish("action", s.c_str());
  }
}