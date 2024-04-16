#include <Arduino.h>
#include <math.h>
const double A0_Amplitude     = 5.0; //Original amplitude of voltage 
const double A1_Amplitude     = 5.0; //Original amplitude of current 
const double V_Multiplier     = A0_Amplitude / 1023.0; // Voltage multiplier
const double A_Multiplier     = A1_Amplitude / 1023.0; // Current multiplier
const int    cyclesToMeasure  = 1; // The number of cycles to measure
const int    samplesPerCycle  = 300; // The number of samples per cycle
const double Freq             = 50 ; // Frequency of sine wave
const int    maxSamples       = samplesPerCycle * cyclesToMeasure; // The number of sampples in sample cycle
const int    samplePeriod     = ((1 / Freq) * cyclesToMeasure * 100000) / maxSamples; // Frequency in microseconds of sampling
int          samples          = 0; // Number of samples taken in sample cycle
const int    powerMin         = -300;
const int    powerMax         = 300;
const int    varMin           = -300;
const int    varMax           = 300;
long         start            = 0;
long         end              = 0;
double       VFinal[maxSamples];
double       AFinal[maxSamples];

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}

void loop() {
  start = micros();
  VFinal[samples] = (V_Multiplier * double(analogRead(A0)) - A0_Amplitude/2);   // Takes a sample of Voltage and Amperage reading and adds it to the array 
  AFinal[samples] = (A_Multiplier * double(analogRead(A1)) - A1_Amplitude/2);   // at index samples then adds one to samples.
  samples++;

  Serial.println(VFinal[samples]);
  
  if (samples >= maxSamples) {
    doCalculation(); // Resets nSamples
    samples = 0;
  } else {
    end = micros();
    pause();
  }
}

void pause(){
  // check if we have exceeded the sample period. 
  // If we have do not bother delaying.
  if (end - start < samplePeriod) {
    delayMicroseconds( samplePeriod - (end - start) ); 
  }
}

void doCalculation() {
  // do RMS calculations
  double Vrms = squareAdd(VFinal, samples); // Voltage
  double Arms = squareAdd(AFinal, samples); // Amperes
/* Serial
  Serial.print("VRMS: ");
  Serial.println(Vrms,3);
  Serial.print("ARMS: ");
  Serial.println(Arms,3);
*/
  //calculate power
  double RealPower = 0;
  for (int i = 0; i++; i < samples) {
    RealPower += (VFinal[samples] * AFinal[samples]);
  }
  RealPower /= 300;
  
  double ApparentPower = Vrms * Arms;
  double PowerFactor = RealPower / ApparentPower;
  double ActivePower = ApparentPower * PowerFactor; // Watts
}
/* Precentage hvz kam domats
void output(double RealPower, double ReactivePower){
  double powerPercentage = (RealPower - powerMin) / (powerMax - powerMin);
  double varPercentage   = (ReactivePower - varMin)   / (ReactivePower - varMin);

  if (powerPercentage > 1) {
    powerPercentage = 1.0;
  }
  if (powerPercentage < 0) {
    powerPercentage = 0;
  }
  if (varPercentage > 1) {
    varPercentage = 1.0;
  }
  if (varPercentage < 0) {
    varPercentage = 0;
  }

  // power goes to 6
  // var goes to 9
  analogWrite(5, powerPercentage * 255); // 980 Hz
  analogWrite(6, varPercentage   * 255); // 980 Hz
}
*/
// The square root of: (x1^2 + x2^2 + x3^2 ... xn^2) / n
double squareAdd(double arr[], int length){
  double add = 0;
  for (int i=0; i < length; i++) {
    add += arr[i] * arr[i];
  }
  return sqrt(add / double(length));
}
  // double ReactivePower = sqrt(ApparentPower*ApparentPower - RealPower*RealPower); Not needed i guess