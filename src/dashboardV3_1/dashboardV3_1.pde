import processing.serial.*;

Serial dataPort; //    Input port from arduino.
PImage imgDialVO;
PImage imgDialRG;
PImage imgDialBR;

long lastReceive = 0;
final int FRAMELENGTH = 2000;

dial tempDial;
dial speedDial;
dial rpmDial;
dial voltage1Dial;
dial voltage2Dial;
dial currentDial;
long distance;
String transmissionTime = "never";
boolean fanState;

void setup()
{
  size(1000,700);
  frameRate(6000);
  
  dataPort = new Serial(this, Serial.list()[0], 9600);
  
  imgDialVO = loadImage("DialVO.jpg");
  imgDialRG = loadImage("DialRG.jpg");
  imgDialBR = loadImage("DialBR.jpg");
  
  tempDial = new dial(100, "Motor Temperature", "\u00b0C", 1, 0, "BR");
  speedDial = new dial(50, "Speed", "mph", 1, 0, "RG");
  voltage1Dial = new dial(20000, "Voltage 1", "V", 1000, 2, "VO");
  voltage2Dial = new dial(20000, "Voltage 2", "V", 1000, 2, "VO");
  currentDial = new dial(100000, "Current", "A", 1000, 1, "BR");
  rpmDial = new dial(4000, "RPM", "", 1, 0, "RG");
  
}

void draw()
{
  background(255);
  speedDial.render(500, 450, 200);
  voltage1Dial.render(850, 380, 100);
  voltage2Dial.render(850, 580, 100);
  currentDial.render(150, 220, 120);
  rpmDial.render(150, 530, 120);
  tempDial.render(850, 140, 120);
  textAlign(CENTER, CENTER);
  textSize(16);
  text("Distance Travelled", 500, 40);
  text(int(distance), 500, 60);
  text("metres", 500, 80);
  text("Fan " + (fanState? "ON" : "OFF"), 500, 120);
  update();
  statusInfo();
}



void update()
{
  while (dataPort.available() > 0) 
  {
    String s = dataPort.readString();
    
    String oldsave[] = loadStrings("log.txt");
    int l = oldsave.length;
    String save[] = new String[l+1];
    for (int i = 0; i < l; i++)
    {
      save[i] = oldsave[i];
    }
    save[l] = s;
    saveStrings("log.txt", save);
  
    String[] inputs = split(s, '\n'); // Splits up the separate values by line breaks
    for (int i = 0; i < inputs.length; ++i)
    {
      if (inputs[i].length() > 1)
        {
          switch(inputs[i].charAt(0))
          {
            case 'T':
              if (isNumber(inputs[i].substring(1)))
                tempDial.addData(Integer.parseInt(inputs[i].substring(1)));
              break;
            case 'D':
              if (isNumber(inputs[i].substring(1)))
                distance = Integer.parseInt(inputs[i].substring(1));
              break;
            case 't':
              transmissionTime = inputs[i].substring(1);
              break;
            case 'S':
              if (isNumber(inputs[i].substring(1)))
                speedDial.addData(Integer.parseInt(inputs[i].substring(1)));
              break;
            case 'R':
              if (isNumber(inputs[i].substring(1)))
                rpmDial.addData(Long.parseLong(inputs[i].substring(1)));
              break;
            case 'I':
              if (isNumber(inputs[i].substring(1)))
                currentDial.addData(Long.parseLong(inputs[i].substring(1)));
              break;
            case 'v':
              if (isNumber(inputs[i].substring(1)))
                voltage1Dial.addData(Long.parseLong(inputs[i].substring(1)));
              break;
            case 'V':
              if (isNumber(inputs[i].substring(1)))
                voltage2Dial.addData(Long.parseLong(inputs[i].substring(1)));
              break;
            case 'L':
              break;
              //represents wheel rotations. Only useful for diagnostic purposes.
            case 'l':
              break;
              //represents number of rotations on last frame.
            case 'F':
              if (inputs[i].substring(1) == "1")
                fanState = true;
              if (inputs[i].substring(1) == "0")
                fanState = false;
              break;
            case '<': //indicates notification for the user.
              println(inputs[i].substring(1));
              break;
          }
        }  
    }
    lastReceive = millis();
  }
}

void statusInfo()
{
  noStroke();
  if (millis() - lastReceive < FRAMELENGTH + FRAMELENGTH / 10)
    fill(0,255,0); //green status indicator (top right corner) - displaying live information
  else
  {
    if (millis() - lastReceive < 3 * FRAMELENGTH + FRAMELENGTH / 10)
      fill(255,255,0); //yellow status indicator - up to 3 transmissions lost
    else
    {
      fill(255,0,0); //red status indicator - no longer receiving data.
    }
  }
  
  ellipseMode(CORNER);
  ellipse(10,10,20,20); //status indicator
  textSize(10);
  fill (0,0,0);
  text("last transmission sent: " + transmissionTime + ".", 850, 677);
  if (int((millis() - lastReceive) / 1000) == 1)
  {
    text("time since last receive: 1 second.", 850, 687);
  }
  else
    text("time since last receive: " + str(int((millis() - lastReceive) / 1000)) + " seconds.", 850, 687);
}

boolean isNumber(String checkString)
{
  for (int i = 0; i < checkString.length(); ++i)
  {
    if (checkString.charAt(i) < '0' && checkString.charAt(i) > '9')
      return false;
  }
  return true;
}
