#include <Arduino.h>

// LEDs
#define LED_R 25
#define LED_G 26
#define LED_B 27

#define LED_BLE 18
// Batterie
#define BATTERY_ADC 32

// BLE Wake
#define AWAKE_BLE 12

// I2C BME
#define SDA_BME 21
#define SCL_BME 22

// UART Balises
#define RX_BALISE1 16
#define TX_BALISE1 17
#define RX_BALISE2 33
#define TX_BALISE2 32

// CONSTANTS
// =======================
const float ADC_REF = 3.3;
const int ADC_RES = 4095;

// Pont diviseur : R1=1M / R2=220k
const float R1 = 1000000.0;
const float R2 = 220000.0;

float readBatteryVoltage() {
  int adcValue = analogRead(BATTERY_ADC);
  float voltage = (adcValue * ADC_REF / ADC_RES);
  voltage = voltage * ((R1 + R2) / R2);
  return voltage;
}

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_R, r);
  digitalWrite(LED_G, g);
  digitalWrite(LED_B, b);
}

void setup() {

   Serial.begin(115200);

  // LEDs
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_BLE, OUTPUT);
  // BLE wake
  pinMode(AWAKE_BLE, INPUT_PULLUP);
 
  // ADC
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_ADC, ADC_11db);
}

void loop() {
  // ---- Batterie ----
  float battery = readBatteryVoltage();
  Serial.print("Batterie : ");
  Serial.print(battery, 2);
  Serial.println(" V");

  // ---- LED Status ----
  if (battery > 7.0) {
    setLED(0, 1, 0); // Vert OK
  } else if (battery > 6.5) {
    setLED(1, 1, 0); // Jaune
  } else {
    setLED(1, 0, 0); // Rouge
  }

}