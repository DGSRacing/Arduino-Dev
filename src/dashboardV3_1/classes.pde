class dial
{
  private long lastValue;
  private long value;
  private long dataInputTime;
  private long maxValue;
  private boolean error;
  private String label;
  private String unit;
  private long scaleFactor;
  private int dp;
  private long[] markedValues;
  private String colourGrad;
  
  dial(long _max, String _label, String _unit, long _scaleFactor, int _dp, String _colourGrad)
  {
    lastValue = 0;
    value = 0;
    dataInputTime = -2 * FRAMELENGTH;
    maxValue = _max;
    error = false;
    label = _label;
    unit = _unit;
    scaleFactor = _scaleFactor;
    dp = _dp;
    colourGrad = _colourGrad;
    for (int i = 10; i > 1; -- i)
    {
      if (maxValue % i == 0)
      {
        markedValues = new long[i+1];
        for (int j = 0; j <= i; ++j)
        {
          markedValues[j] = j * maxValue / i;
        }
        return;
      }
    }
    markedValues = new long[]{0, maxValue};
  }
  
  void addData(long _value)
  {
    lastValue = value;
    value = _value;
    if (value > maxValue)
    {
      value = maxValue;
      error = true;
    }
    else if (value < 0)
    {
      value = 0;
      error = true;
    }
    else
    {
      error = false;
    }
    dataInputTime = millis();
  }
  
  //positions are of centre.
  void render(float _posX, float _posY, float radius)
  {
    if (millis() - dataInputTime > FRAMELENGTH)
    {
      lastValue = value;
    }
    if (error)
    {
      fill(255, 0, 0);
    }
    else if (millis() - dataInputTime > FRAMELENGTH + FRAMELENGTH / 10)
    {
      fill(231, 231, 37);
    }
    else
    {
      fill(0,0,0);
    }
    if(colourGrad == "BR")
      image(imgDialBR, _posX - radius, _posY - radius, radius * 2, radius * 2);
    if(colourGrad == "VO")
      image(imgDialVO, _posX - radius, _posY - radius, radius * 2, radius * 2);
    if(colourGrad == "RG")
      image(imgDialRG, _posX - radius, _posY - radius, radius * 2, radius * 2);
      
    ellipseMode(RADIUS);
    ellipse(_posX, _posY, radius / 20.0, radius / 20.0); //dot for base of needle.
    float theta = 3.0 / 2.0 * PI * ((value-lastValue)*(millis() - dataInputTime)/((float)FRAMELENGTH) + lastValue)/((float)maxValue) - 0.75 * PI; // angle from upwards vertical of the speedometer to the dial position.
    triangle(_posX + (radius * 4.0 / 5.0) * sin(theta), _posY - (radius * 4.0 / 5.0) * cos(theta), (cos(theta) * 0.032 * radius) + _posX, (sin(theta) * 0.032 * radius) + _posY, (cos(PI + theta) * 0.032 * radius) + _posX, (sin(PI + theta) * 0.032 * radius) + _posY);
    fill(0,0,0);
    textSize(radius / 15);
    textAlign(CENTER, CENTER);
    for (int i = 0; i < markedValues.length; ++i)
    {
      float labelAngle = 3.0 / 2.0 * PI * ((float)markedValues[i]/maxValue) - 0.75 * PI;
      text(longToText(markedValues[i], scaleFactor, dp), _posX + (radius * 5.2 / 5.0) * sin(labelAngle), _posY - (radius * 5.2 / 5.0) * cos(labelAngle));
    }
    textSize(radius / 10);
    text(label, _posX, _posY + radius / 3.0);
    text(longToText(value, scaleFactor, dp), _posX, _posY + 1.3 * radius / 3.0);
    text(unit, _posX, _posY + 1.6 * radius / 3.0);
  }
  
}

String longToText (long val, long scaleFactor, int dp)
{
  String retVal = "";
  if (dp == 0)
  {
    retVal += ((val + scaleFactor / 2) / scaleFactor);
  }
  else
  {
    retVal += val / scaleFactor;
    retVal += ".";
    for (int i = 0; i < dp; ++i)
    {
      val %= scaleFactor;
      val *= 10;
      if (i == dp - 1)
      {
        val += scaleFactor /2;
      }
      if (val/scaleFactor == 10)
        retVal += "9"; //slightly inaccurate but stops 12.10 when should round to 13.0
      else
        retVal += val / scaleFactor;
    }
  }
  return retVal;
}
