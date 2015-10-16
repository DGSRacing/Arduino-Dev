#ifndef ANALOGSENSORHEADERINCLUDEDFLAG
#define ANALOGSENSORHEADERINCLUDEDFLAG

class AnalogSensor
{
private:
  const char pin;
  const float constant;
  const float coefficient;
  
  float reading;
  
public:
  AnalogSensor(char _pin, float _constant, float _coefficient);
  
  void update();
  float readValue();
};

#endif
