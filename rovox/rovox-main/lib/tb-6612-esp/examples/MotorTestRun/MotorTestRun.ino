#include <arduino.h>


int ThermistorPin = A0;
int Vo;
//float R1 = 10000; // value of R1 on board
//float logR2, R2, T;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
void setup() {
  Serial.begin(9600);
}
void loop() {
  Vo = analogRead(ThermistorPin); // read raw ADC value
  float voltage = Vo*(5.0/1023.0); // convert raw value to voltage
  float resistance = 10000*(1023.0/(float)Vo - 1.0); // calculates resistance of thermistor in ohms(10K pull-up resistor)
  float temp = (1.0/(c1 + (c2*log(resistance)) + c3*log(resistance)*log(resistance)*log(resistance))); // calculates temperature in Kelvin using the Steinhart-Hart equation
  temp-=273.15; // converts to Celsius
  Serial.print("Temperature: "); 
  Serial.print(temp);
  Serial.println(" F"); 
  delay(10000);
}
