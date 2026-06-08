#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

// =====================
// WIFI CONFIG
// =====================
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

// =====================
// THINGSPEAK CONFIG
// =====================
unsigned long channelID = 3403065;
const char* writeAPIKey = "WRITE_API_KEY";

WiFiClient client;

// =====================
// SENSOR CONFIG
// =====================
#define DHTPIN 4
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_PIN 34

// =====================
// RELAY PIN
// =====================
#define RELAY1 26
#define RELAY2 27
#define RELAY3 14
#define RELAY4 12

// =====================
// WIFI CONNECT
// =====================
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());
}

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, HIGH);

  dht.begin();
  connectWiFi();

  ThingSpeak.begin(client);
}

// =====================
// LOOP
// =====================
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  float suhu = dht.readTemperature();
  float hum  = dht.readHumidity();

  // VALIDASI DHT (INI PENTING!)
  if (isnan(suhu) || isnan(hum)) {
    Serial.println("DHT ERROR - skip send");
    delay(2000);
    return;
  }

  int soilRaw = analogRead(SOIL_PIN);

  // mapping soil (kalibrasi umum ESP32)
  int soil = map(soilRaw, 4095, 1500, 0, 100);
  soil = constrain(soil, 0, 100);

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" | Hum: ");
  Serial.print(hum);
  Serial.print(" | Soil: ");
  Serial.println(soil);

  // =====================
  // THINGSPEAK SEND
  // =====================
  ThingSpeak.setField(1, suhu);  // FIELD 1 = SUHU
  ThingSpeak.setField(2, hum);   // FIELD 2 = HUMIDITY
  ThingSpeak.setField(3, soil);  // FIELD 3 = SOIL

  int status = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (status == 200) {
    Serial.println("ThingSpeak OK");
  } else {
    Serial.println("ThingSpeak FAIL");
  }

  delay(15000); // jangan kurang dari 15 detik (limit TS)
}