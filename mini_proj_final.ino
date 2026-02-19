// ESP32 Blynk Smart Plant Monitoring System
#define BLYNK_TEMPLATE_ID "TMPL3V0xXLy8h"
#define BLYNK_TEMPLATE_NAME "Smart Plant"
#define BLYNK_AUTH_TOKEN "nHGbZZgBGppZG4EJjDgRsFzU8KMGV4cT"
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// -------- LCD --------
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// -------- BLYNK --------
char ssid[] = "Tanishka's Galaxy M31";
char pass[] = "krzm0167";

// -------- PINS (ESP32) --------
#define DHTPIN 4
#define DHTTYPE DHT11

#define SOIL_PIN 36
#define PIR_PIN 19
#define RELAY_PIN 18
#define BUTTON_PIN 5

#define VPIN_BUTTON_1 V12

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

int relay1State = LOW;
int pushButton1State = HIGH;
int PIR_ToggleValue;

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, relay1State);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);


  lcd.setCursor(0, 0);
  lcd.print(" Initializing ");
  delay(2000);
  lcd.clear();

  timer.setInterval(2000L, DHT11sensor);
  timer.setInterval(1000L, soilMoistureSensor);
  timer.setInterval(500L, checkPhysicalButton);
}

// -------- DHT SENSOR --------
void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) return;

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);
}

// -------- SOIL SENSOR --------
void soilMoistureSensor() {
  int value = analogRead(SOIL_PIN);
  value = map(value, 0, 4095, 100, 0);

  Blynk.virtualWrite(V3, value);

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);
  lcd.print(" ");
}

// -------- PIR --------
void PIRsensor() {
  if (digitalRead(PIR_PIN)) {
    Blynk.logEvent("pirmotion", "Motion Detected!");
    WidgetLED led(V5);
    led.on();
  } else {
    WidgetLED led(V5);
    led.off();
  }
}

// -------- BLYNK --------
BLYNK_WRITE(V6) {
  PIR_ToggleValue = param.asInt();
}

BLYNK_WRITE(VPIN_BUTTON_1) {
  relay1State = param.asInt();
  digitalWrite(RELAY_PIN, relay1State);
}

void checkPhysicalButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (pushButton1State == HIGH) {
      relay1State = !relay1State;
      digitalWrite(RELAY_PIN, relay1State);
      Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
    }
    pushButton1State = LOW;
  } else {
    pushButton1State = HIGH;
  }
}

// -------- LOOP --------
void loop() {
  if (PIR_ToggleValue) {
    lcd.setCursor(5, 1);
    lcd.print("M:ON ");
    PIRsensor();
  } else {
    lcd.setCursor(5, 1);
    lcd.print("M:OFF");
  }

  lcd.setCursor(11, 1);
  lcd.print(relay1State ? "W:ON " : "W:OFF");

  Blynk.run();
  timer.run();
}
