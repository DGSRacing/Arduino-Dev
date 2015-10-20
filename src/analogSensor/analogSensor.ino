#include "analogSensor.h"

AnalogSensor::AnalogSensor(char _pin, float _coefficient, float _constant):
  pin(_pin), coefficient(_coefficient), constant(_constant) //initialisation list
{
  reading = 0;
}

void AnalogSensor::update()
{
  analogRead(pin); //allows ADC to settle
  reading = 0.99 * reading + 0.01 * analogRead(pin);
}

float AnalogSensor::readValue()
{
  return reading * coefficient + constant;
}
