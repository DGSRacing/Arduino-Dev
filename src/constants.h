#ifndef CONSTANTSHEADERINCLUDEDFLAG
#define CONSTANTSHEADERINCLUDEDFLAG

//digital pins:
// SPEEDPIN = 2 cannot be declared here, as interrupts only on certain pins.
const char DISPLEDPIN = 8;
const char LEDPIN = 9;
const char FANPIN = 10;

//analog pins:
const char TEMPERATUREPIN = 6;
const char CURRENTPIN = 7;
const char VOLTAGE1PIN = 8;
const char VOLTAGE2PIN = 9;


const char LEDCOUNT = 8; //number of LEDs on the display.
const long SPEEDDEBOUNCEINTERVAL = 30; //milliseconds. Value of 30 means speeds between 1.5 mph and 96 mph should be measured accurately
const float WHEELCIRCUMFERENCE = 1.3;
//const float GEARRATIO = 60.0/16.0; // for Carbon QT car
const float GEARRATIO = 3.0/1.0; // for new car
const long FRAMELENGTH = 2000;
const int FANONTEMP = 60;
const int FANOFFTEMP = 50;

#endif
