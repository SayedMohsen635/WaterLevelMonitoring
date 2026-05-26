#include "config.h"
#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>

int tankHeight = SENSOR_TO_BOTTOM_DISTANCE - SENSOR_TO_TOP_DISTANCE;

long duration = 0;
int distance = 0;
int sensorReading = 0;
int waterLevel = 0;
int ledLevel = 0;

int notified = 0; // To determine if notification is sent or not

BlynkTimer timer;

int readUltrasonic();
void timerEvent();

const int ledPins[LED_NUMBERS] = {LED1, LED2, LED3, LED4, LED5};

#define BLYNK_PRINT Serial

void setup() {
  Serial.begin(9600);
  delay(100);
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI, PASS);
  timer.setInterval(1000L, timerEvent);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < LED_NUMBERS; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  digitalWrite(BUZZER, LOW);
}

void loop() {
  Blynk.run();
  timer.run();

  if (!Blynk.connected()) {
    while (!Blynk.connected()) {
      Serial.println("Connecting ...");
      delay(100);
      Blynk.begin(BLYNK_AUTH_TOKEN, WIFI, PASS);
    }
    Serial.println("Connected !");
    delay(500);
  }
}

int readUltrasonic() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH);
  distance = duration * 0.017;

  if (distance >= 4) {
    return distance;
  }

  return 0;
}

void timerEvent() {
  Blynk.virtualWrite(V0, waterLevel);
  sensorReading = readUltrasonic();

  if (sensorReading < SENSOR_TO_TOP_DISTANCE) {
    sensorReading = SENSOR_TO_TOP_DISTANCE;
  } else if (sensorReading > SENSOR_TO_BOTTOM_DISTANCE) {
    sensorReading = SENSOR_TO_BOTTOM_DISTANCE;
  }

  sensorReading = sensorReading - SENSOR_TO_TOP_DISTANCE;
  waterLevel = ((tankHeight - sensorReading) * 100) / tankHeight;
  waterLevel = ((waterLevel + 5) / 10) * 10;
  Serial.printf("Water level is %d\n", waterLevel);

  ledLevel = waterLevel / 20; // nunmber of leds to be turned ON or OFF (0-5)

  for (int i = 0; i < LED_NUMBERS; i++) {
    digitalWrite(ledPins[i], (i < ledLevel) ? HIGH : LOW);
  }

  if (waterLevel < 20) {
    digitalWrite(BUZZER, HIGH);
    delay(50);
    digitalWrite(BUZZER, LOW);
    delay(30);

    if (notified == 0) {
      Blynk.logEvent("alert", "The water level is low.");
      notified = 1;
    }
  } else {
    notified = 0;
  }
}
