#include "constants.h"
#include "analogSensor.h"
#include <Adafruit_NeoPixel.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DS1307.h>
#include <SPI.h>
#define VERSIONNUMBER "V3.4"

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LEDCOUNT, DISPLEDPIN, NEO_GRB + NEO_KHZ800);

LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

AnalogSensor temperatureSensor(TEMPERATUREPIN, (5000.0 / 1024.0) / 9.31, 0.0);
AnalogSensor currentSensor(CURRENTPIN, 0.07738, -40.532);
AnalogSensor voltage1Sensor(VOLTAGE1PIN, 0.02366, 0);
AnalogSensor voltage2Sensor(VOLTAGE2PIN, 0.02366, 0);

AnalogSensor *sensors[4] = {&temperatureSensor, &currentSensor, &voltage1Sensor, &voltage2Sensor};

boolean fanState = false;
long ledCounter = 0;
unsigned long lastFrame = 0;
char* fileName = "log0.txt";

float current;   //Amps
float voltage1;  //Volts
float voltage2;  //Volts
long rpm;        //revs per minute
int mph;         //miles per hour
long distance;   //metres
int temperature; //deg C

volatile long wheelRotations = 0;
volatile long lastReadTime = 0;
long lastWheelReading = 0;


void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  
  leds.begin();
  clearLEDs();
  leds.show();
  
  pinMode(53, OUTPUT);//required for SD, even if a different pin is used as CS pin
  if (!SD.begin(53))
  {
    Serial1.println("<SD card failed");
    fileName = "ERROR";
  }
  else
  {
    Serial1.println("<SD card initialized");
    for (int i = 0; SD.exists(fileName); ++i)
    {
      String temp = "log";
      temp += i;
      temp += ".txt";
      temp.toCharArray(fileName, temp.length() + 1);
    }
    File dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile)
    {
      String dateRead = dateYYMMDD();
      dataFile.println("Booted at " + timeHHMMSS() + " on " + dateRead.substring(4,6) + "/" + dateRead.substring(2,4) + "/" + dateRead.substring(0,2) + ".");
      dataFile.print("time(HH:MM:SS), ");
      dataFile.print("current(A)");
      dataFile.print(", ");
      dataFile.print("voltage1(V)");
      dataFile.print(", ");
      dataFile.print("voltage2(V)");
      dataFile.print(", ");
      dataFile.print("rpm");
      dataFile.print(", ");
      dataFile.print("speed(miles/hour)");
      dataFile.print(", ");
      dataFile.print("distance(metres)");
      dataFile.print(", ");
      dataFile.print("temperature(C)");
      dataFile.print(", ");
      dataFile.print("wheelRotations");
      dataFile.print(", ");
      dataFile.println("fanState(1/0)");
      dataFile.close();
    }
    else
      Serial1.println("<Error logging");
  }
  
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  
  pinMode(FANPIN, OUTPUT);
  digitalWrite(FANPIN, LOW);
  
  lcd.begin(20,4);
  lcd.backlight();
  
  lcd.setCursor(0,0);
  lcd.print("DGS F24 TELEMETRY");
  lcd.setCursor(0,1);
  lcd.print(VERSIONNUMBER);
  
  //for speed sensor:
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  attachInterrupt(0, wheelCount, FALLING);
  
  String dateRead = dateYYMMDD();
  Serial1.println("<Booted at " + timeHHMMSS() + " on " + dateRead.substring(4,6) + "/" + dateRead.substring(2,4) + "/" + dateRead.substring(0,2) + ".");
    
  lastFrame = millis();
}

void loop()
{
  //LED heartbeat:
  ledCounter+=10;
  analogWrite(LEDPIN, exp(sin((ledCounter/5000.0)*PI) - 0.36787944)*108.0);
  
  for (int i = 0; i < sizeof(sensors)/sizeof(sensors[0]); ++i)
  {
    sensors[i]->update();
  }
  
  if (millis() >= lastFrame + FRAMELENGTH)
  {
    current = currentSensor.readValue();
    temperature = temperatureSensor.readValue();
    voltage1 = voltage1Sensor.readValue();
    voltage2 = voltage2Sensor.readValue();
    
    distance = WHEELCIRCUMFERENCE * float(wheelRotations);
    mph = ((float)(wheelRotations - lastWheelReading) * WHEELCIRCUMFERENCE * 2237.0f)  / (float)FRAMELENGTH;
    rpm = (((wheelRotations - lastWheelReading) *60000.0f) / (FRAMELENGTH)) * GEARRATIO;
    
    int activeLEDs = (voltage1 + voltage2) * 8.0 / (24.0-18.0);
    if (activeLEDs > 8)
      activeLEDs = 8;
    if (voltage1 + voltage2 < 18.0)
      activeLEDs = 8;
    int redVal;
    int greenVal;
    if (voltage1 + voltage2 > 24)
    {
      greenVal = 255;
      redVal = 0;
    }
    else if (voltage1 + voltage2 < 18)
    {
      redVal = 255;
      greenVal = 0;
    }
    else if (voltage1 + voltage2 < 21)
    {
      redVal = 255;
      greenVal = 255 - (voltage1 + voltage2) / (21.0 - 18.0) * 255;
    }
    else
    {
      greenVal = 255;
      redVal = (voltage1 + voltage2)/(24.0-21.0) * 255;
    }
    for (int i = 0; i < activeLEDs; ++i)
    {
      leds.setPixelColor(i, redVal, greenVal, 0);
    }
    leds.show();
    
      
    
    
    if (fanState)
    {
      if (temperature < FANOFFTEMP)
      {
        fanState = false;
        digitalWrite(FANPIN, LOW);
      }
    }
    else
    {
      if (temperature > FANONTEMP)
      {
        fanState = true;
        digitalWrite(FANPIN, HIGH);
      }
    }
    
    //This is NOT intended for race day use - sending data to the arduino during a race is against the Greenpower F24 rules.
    String input = "";
    while(Serial1.available())
      input += Serial1.read();
    //To change time, send down the serial: YYMMDDddHHMMSS  (dd is the day of the week - 1 = sunday, 7 = saturday)
    if (input.length() == 14)
    {
      setRTC(input);
    }
    
    input = "";
    while(Serial.available())
      input += Serial.read();
    if (input.length() == 14)
    {
      setRTC(input);
    }
    
    updateLCD();
    radioSend();
    if (fileName != "ERROR")
      logToSD();
    
    lastWheelReading = wheelRotations;
    lastFrame = millis();
  }
}

void wheelCount()
{
  if (millis() > lastReadTime + SPEEDDEBOUNCEINTERVAL)
    ++wheelRotations;
  lastReadTime = millis();
}

void clearLEDs()
{
  for (int i=0; i<LEDCOUNT; i++)
  {
    leds.setPixelColor(i, 0);
  }
}

void updateLCD()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Speed " + toTwoDigits(mph) + " MPH");
  lcd.setCursor(0,1);
  lcd.print("Motor Temp " + toTwoDigits(temperature) + char(223) + "C");
  lcd.setCursor(0,2);
  lcd.print("Dist " + toThreeDigits((float)distance / 1609.344) + " CURR " + toThreeDigits(current) + "A");
  lcd.setCursor(0,3);
  lcd.print("Voltage " + toFourDigits(voltage1 + voltage2));
}

void radioSend()
{
    String toSend = "T";
    toSend += temperature;
    toSend += "\nD";
    toSend += distance;
    toSend += "\nt";
    toSend += timeHHMMSS();
    toSend += "\nS";
    toSend += mph;
    toSend += "\nR";
    toSend += rpm;
    toSend += "\nI";
    toSend += (long)(current*1000.0);
    toSend += "\nv";
    toSend += (long)(voltage1*1000.0);
    toSend += "\nV";
    toSend += (long)(voltage2*1000.0);
    toSend += "\nL";
    toSend += wheelRotations;
    toSend += "\nl";
    toSend += lastWheelReading;
    toSend += "\nF";
    toSend += fanState? "1":"0"; //sends 1 if fan is on, 0 otherwise
    toSend += "\n\n";
    Serial1.print(toSend);
}

void logToSD()
{
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile)
  {
    dataFile.print(timeHHMMSS() + ", ");
    dataFile.print(current);
    dataFile.print(", ");
    dataFile.print(voltage1);
    dataFile.print(", ");
    dataFile.print(voltage2);
    dataFile.print(", ");
    dataFile.print(rpm);
    dataFile.print(", ");
    dataFile.print(mph);
    dataFile.print(", ");
    dataFile.print(distance);
    dataFile.print(", ");
    dataFile.print(temperature);
    dataFile.print(", ");
    dataFile.print(wheelRotations);
    dataFile.print(", ");
    dataFile.println(fanState? "1":"0");
    dataFile.close();
  }
  else
    Serial1.println("<Error logging");
}

/*-------------------------String formatting-------------------------*/
String toFourDigits (double variable)
{
  variable += 0.005; // for rounding
  if (variable < 0)
    return "00.00";
  if (variable > 99.99)
    return "99.99";
  long i = variable * 100;
  if (i < 1000)
  {
    return ("0" + String(i/100) + "." + String(i % 100));
  }
  else
  {
    return(String(i/100) + "." + (i%100 < 10 ? "0" + String(i%100):String(i%100)));
  }
}

String toThreeDigits(double variable)
{
  variable += 0.05; //for rounding
  if (variable < 0)
    return "00.0";
  if (variable > 99.9)
    return "99.9";
  int i = variable * 10;
  if (i < 100)
  {
    return("0" + String(i / 10) + "." + String(i % 10));
  }
  else
  {
    return(String(i/10) + "." + String(i % 10));
  }
}

String toTwoDigits(int variable)
{
  if (variable < 0)
    return "00";
  if (variable > 99)
    return "99";
  if (variable < 10)
    return ("0" + String(variable));
  return String(variable);
}
/*-------------------------------------------------------------------*/

/*-----------------------------RTC STUFF-----------------------------*/
//These funtions print the time read from the RTC in different formats:
void setRTC(String time)
{
  RTC.stop();
  RTC.set(DS1307_SEC,time.substring(12,13).toInt());
  RTC.set(DS1307_MIN,time.substring(10,11).toInt());
  RTC.set(DS1307_HR, time.substring(8, 9).toInt());
  RTC.set(DS1307_DOW, time.substring(6,7).toInt());
  RTC.set(DS1307_DATE,time.substring(4, 5).toInt());
  RTC.set(DS1307_MTH,time.substring(2, 3).toInt());
  RTC.set(DS1307_YR,time.substring(0, 1).toInt());
  RTC.start();
  Serial1.print("<Time changed");
}
String timeHHMMSS(void)
{
  int rtc[7];
  RTC.get(rtc,true);  
  return (toTwoDigits(rtc[2]) + ":" + toTwoDigits(rtc[1]) + ":" + toTwoDigits(rtc[0]));
}

String dateYYMMDD(void)
{
  int rtc[7];
  RTC.get(rtc,true);  
  return (toTwoDigits(rtc[6] % 100) + toTwoDigits(rtc[5]) + toTwoDigits(rtc[4]));
}
/*-------------------------------------------------------------------*/
