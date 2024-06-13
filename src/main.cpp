// * Libraries
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// * Pins
#define sensPin 4

// * Variables
float voltage = 0;
float current = 0;
float wattage = 0;

U8G2_ST7567_ENH_DG128064I_F_SW_I2C u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);

void writeLcd();

void setup() {
  Serial.begin(9600);

  u8g2.setI2CAddress(0x3F * 2);
  u8g2.begin();
  u8g2.setFlipMode(1);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  pinMode(sensPin, INPUT);

  writeLcd();
}

void writeLcd() {
  char voltageStr[10];
  char currentStr[10];
  char wattageStr[10];

  dtostrf(voltage, 5, 2, voltageStr);
  dtostrf(current, 5, 2, currentStr);
  dtostrf(wattage, 5, 2, wattageStr);

  u8g2.clearBuffer();
  u8g2.drawStr(12, 25, "Volti (V): ");
  u8g2.drawStr(85, 25, voltageStr);
  u8g2.drawStr(12, 35, "Amperi (A): ");
  u8g2.drawStr(85, 35, currentStr);
  u8g2.drawStr(12, 45, "Vati (W): ");
  u8g2.drawStr(85, 45, wattageStr);
  u8g2.sendBuffer();
}

void clearLcd() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void loop() {
  for (int i = 0; i < 1000; i++) {
    voltage = (voltage + (.0049 * analogRead(4)));
    delay(1);
  }
  voltage = voltage / 1000;

  current = (voltage - 2.5) / 0.185;
  Serial.print("\n Voltage Sensed(V) = ");
  Serial.print(voltage, 2);

  Serial.print("\t Current(A) = ");
  Serial.print(current, 2);

  delay(1000);
}
