#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <LittleFS.h>

// ===================== WIFI =====================
const char* ssid = "UGMURO-INET";
const char* password = "Gepuk15000";

// ===================== THINGSPEAK =====================
unsigned long channelID = 3403065;
const char* writeAPIKey = "9B3JIDR2636B4077";

WiFiClient client;

// ===================== SERVER =====================
AsyncWebServer server(80);

// ===================== LCD =====================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===================== DHT =====================
#define DHTPIN 13
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

// ===================== SOIL =====================
#define SOIL_PIN 34

// ===================== RELAY =====================
#define RELAY1 16   // Pompa
#define RELAY2 17   // Fan

// ===================== TIMER THINGSPEAK =====================
unsigned long lastSend = 0;
const long interval = 15000;

// ===================== RELAY CONTROL =====================
void setRelay(int relay, int state)
{
  int pin = (relay == 1) ? RELAY1 : RELAY2;
  digitalWrite(pin, state ? LOW : HIGH);
}

// ===================== SETUP =====================
void setup()
{
  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi OK");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("IP:");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());

  dht.begin();
  ThingSpeak.begin(client);

  // ================= LITTLEFS =================
  if(!LittleFS.begin()){
    Serial.println("LittleFS Error");
  }

  // ================= WEB PAGE =================
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  // ================= RELAY API =================
  server.on("/relay", HTTP_GET, [](AsyncWebServerRequest *request){

    if(!request->hasParam("pin") || !request->hasParam("state")){
      request->send(400, "text/plain", "Missing");
      return;
    }

    int pin = request->getParam("pin")->value().toInt();
    int state = request->getParam("state")->value().toInt();

    setRelay(pin, state);

    Serial.printf("Relay %d -> %d\n", pin, state);

    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

// ================= LOOP =================
void loop()
{
  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();

  int soilRaw = analogRead(SOIL_PIN);
  int soil = map(soilRaw, 4095, 1500, 0, 100);
  soil = constrain(soil, 0, 100);

  // LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(suhu);
  lcd.print(" H:");
  lcd.print(hum);

  lcd.setCursor(0,1);
  lcd.print("Soil:");
  lcd.print(soil);

  // ThingSpeak NON-BLOCKING
  if(millis() - lastSend > interval)
  {
    lastSend = millis();

    ThingSpeak.setField(1, suhu);
    ThingSpeak.setField(2, hum);
    ThingSpeak.setField(3, soil);

    int x = ThingSpeak.writeFields(channelID, writeAPIKey);

    Serial.println(x == 200 ? "TS OK" : "TS FAIL");
  }
}