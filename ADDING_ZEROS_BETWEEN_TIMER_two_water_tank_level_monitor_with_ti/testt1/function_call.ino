void WindDirection()
{
  sensorValue = analogRead(34);
  if(sensorValue > 0 && sensorValue < 793)
  {
    Serial.println("EAST TO SOUTH");
  }
  else if(sensorValue >= 793 && sensorValue < 3575)
  {
    Serial.println("SOUTH TO WEST");
  }
  else if(sensorValue >= 3575 && sensorValue < 4095)
  {
    Serial.println("WEST TO NORTH");
  }
  else
  {
    Serial.println("NORTH TO EAST");
  }  
}

void WindSpeed()
{
    currentTime =  millis();
    exactTime = currentTime - previousTime;  // MilliSeconds
    float RotationPerSec = (1000.00 / exactTime) ;
    float KMPH = ( RotationPerSec * RadiusOfTheWheel * 22.608 );
    Serial.print("WindSpeed: ");
    Serial.println(KMPH);
    previousTime = currentTime;
}
