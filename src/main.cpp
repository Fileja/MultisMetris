// * Libraries
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>

#define BUFFER_SIZE 200

volatile uint16_t bufferU[BUFFER_SIZE]; // Voltage buffer
volatile uint16_t bufferI[BUFFER_SIZE]; // Current buffer
volatile uint8_t sampleIndex = 0;
volatile bool bufferReady = false;

void setup() {
  Serial.begin(115200);

  ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2);
  ADCSRB = 0;

  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = 15;
  TIMSK1 = (1 << OCIE1A);

  sei();
}

ISR(TIMER1_COMPA_vect) {
  ADMUX = (1 << REFS0);
  ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
  uint16_t adcValue = ADC;

  if (ADMUX == (1 << REFS0)) {
    bufferU[sampleIndex] = adcValue;
    ADMUX = (1 << REFS0) | (1 << MUX0);
    ADCSRA |= (1 << ADSC);
  } else {
    bufferI[sampleIndex] = adcValue;
    sampleIndex++;
    if (sampleIndex >= BUFFER_SIZE) {
      sampleIndex = 0;
      bufferReady = true;
    }
    ADMUX = (1 << REFS0);
  }
}

int findZCIndex(volatile uint16_t *buffer, float average) {
  for (int i = 1; i < BUFFER_SIZE; i++) {
    if ((buffer[i - 1] - average) * (buffer[i] - average) <=
        0) { // Detect zero crossing
      return i;
    }
  }
  return -1;
}

void processBuffer(volatile uint16_t *bufferU, volatile uint16_t *bufferI) {
  uint32_t sumU = 0;
  uint32_t sumI = 0;
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    sumU += bufferU[i];
    sumI += bufferI[i];
  }
  float averageU = sumU / (float)BUFFER_SIZE;
  float averageI = sumI / (float)BUFFER_SIZE;

  uint32_t sq_sumU = 0;
  uint32_t sq_sumI = 0;
  int64_t realPowerSum = 0;
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    int32_t diffU = ((int32_t)bufferU[i] - (int32_t)averageU) * 0.334f;
    int32_t diffI = ((int32_t)bufferI[i] - (int32_t)averageI) * 0.052f;
    sq_sumU += diffU * diffU;
    sq_sumI += diffI * diffI;
    realPowerSum += (int64_t)diffU * (int64_t)diffI;
  }
  float rmsU = sqrt(sq_sumU / (float)BUFFER_SIZE);
  float rmsI = sqrt(sq_sumI / (float)BUFFER_SIZE);
  float realPower = realPowerSum / (float)BUFFER_SIZE;
  float apparentPower = rmsU * rmsI;
  float reactivePower =
      sqrt(apparentPower * apparentPower - realPower * realPower);
  float powerFactor = realPower / apparentPower;

  int zeroCrossingIndexU = findZCIndex(bufferU, averageU);
  int zeroCrossingIndexI = findZCIndex(bufferI, averageI);

  // AC
  Serial.println("AC");
  Serial.print("Avg U: ");
  Serial.print(averageU);
  Serial.print(" | Avg I): ");
  Serial.print(averageI);
  Serial.print("| RMS U: ");
  Serial.print(rmsU);
  Serial.print(" | RMS I: ");
  Serial.print(rmsI);
  Serial.print(" | P: ");
  Serial.print(realPower);
  Serial.print("| S: ");
  Serial.print(apparentPower);
  Serial.print(" | Q): ");
  Serial.print(reactivePower);
  Serial.print(" | cos φ: ");
  Serial.println(powerFactor);

  // DC
  Serial.println("DC");
  Serial.print("Avg U: ");
  Serial.print(averageU * 0.029f);
  Serial.print(" | Avg I): ");
  Serial.print(averageI * 0.004f);
  Serial.print(" | P: ");
  Serial.print((averageU * 0.029f) * (averageI * 0.004f));

  if (zeroCrossingIndexU >= 0 && zeroCrossingIndexI >= 0) {
    int phaseDifference = zeroCrossingIndexI - zeroCrossingIndexU;
    Serial.print(" Phase Difference: ");
    Serial.println(phaseDifference);

    if (phaseDifference > 0) {
      Serial.println("Inductive.");
    } else if (phaseDifference < 0) {
      Serial.println("Capacitive.");
    } else {
      Serial.println(" ");
    }
  } else {
    Serial.println("Zero crossing error");
  }
}

void loop() {
  if (bufferReady) {
    bufferReady = false;
    processBuffer(bufferU, bufferI);
  }
}