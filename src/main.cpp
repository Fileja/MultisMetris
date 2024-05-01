#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <math.h>

const double VoltageAmplitude = 25.0;                                                       // Original amplitude of voltage
const double CurrentAmplitude = 25.0;                                                       // Original amplitude of current
const double VoltageMultiplier = VoltageAmplitude / 1023.0;                                 // Voltage multiplier
const double CurrentMultiplier = CurrentAmplitude / 1023.0;                                 // Current multiplier
const int CyclesToMeasure = 1;                                                              // The number of cycles to measure
const int SamplesPerCycle = 300;                                                            // The number of samples per cycle
const double SineWaveFrequency = 50;                                                        // Frequency of sine wave
const int MaxSamples = SamplesPerCycle * CyclesToMeasure;                                   // The number of samples in sample cycle
const int SamplePeriod = ((1 / SineWaveFrequency) * CyclesToMeasure * 100000) / MaxSamples; // Frequency in microseconds of sampling
int Samples = 0;                                                                            // Number of samples taken in sample cycle
long StartMicros = 0;
long EndMicros = 0;
double Voltage[MaxSamples];
double Current[MaxSamples];

LiquidCrystal_I2C lcd(0x27, 16, 4); // adress , char , lines

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage :"); // 10
  lcd.setCursor(0, 1);
  lcd.print("Amperes :"); // 10
  lcd.setCursor(0, 2);
  lcd.print("Watts :");   // 7
  Serial.begin(9600);
}

// The square root of: (x1^2 + x2^2 + x3^2 ... xn^2) / n
double SquareAdd(double arr, int SampleCount) {
  double add = 0;
  double square = 2;
  for (int i = 0; i < SampleCount; i++) {
    add += pow(arr, square);
  }
  return sqrt(add / double(SampleCount));
}

void LCD(float v, float a) {
  // Lai nerādītu error par unused variable , bet taa pagaidam neizmantojas
  lcd.setCursor(10, 0);
  lcd.print(v);
  lcd.setCursor(10, 1);
  lcd.print(a);
  lcd.setCursor(7, 2);
  // lcd.print(RealPower);
}

void Calculations() {
  // RMS calculations
  double rmsVoltage = SquareAdd(Voltage[Samples], Samples); // Voltage
  double rmsCurrent = SquareAdd(Current[Samples], Samples); // Amperes
  LCD(rmsVoltage, rmsCurrent);

  // Power calculations
  double RealPower = 0;
  for (int i = 0; i < Samples; i++) {
    RealPower += (Voltage[Samples] * Current[Samples]);
  }
  RealPower /= SamplesPerCycle; // Watts
}

void loop() {
  StartMicros = micros();
  Voltage[Samples] = (VoltageMultiplier * double(analogRead(A0)) - VoltageAmplitude / 2); // Takes a sample of Voltage and Amperage reading and adds it to the array
  Current[Samples] = (CurrentMultiplier * double(analogRead(A1)) - CurrentAmplitude / 2); // at index samples then adds one to samples.
  Samples++;

  if (Samples >= MaxSamples) {
    Calculations();
    Samples = 0;
  } else {
    EndMicros = micros();
    // check if we have exceeded the sample period.
    // If we have do not bother delaying.
    if (EndMicros - StartMicros < SamplePeriod) {
      delayMicroseconds(SamplePeriod - (EndMicros - StartMicros));
    }
  }
}