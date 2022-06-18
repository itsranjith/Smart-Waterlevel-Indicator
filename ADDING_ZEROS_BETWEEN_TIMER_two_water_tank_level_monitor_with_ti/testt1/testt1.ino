int sensorValue = 0;
float currentTime,previousTime,exactTime;
float RadiusOfTheWheel = 1.5;

void WindSpeed();
void WindDirection();

void setup() 
{
  pinMode(34,INPUT);
  pinMode(4,INPUT);
  pinMode(2,OUTPUT);
  Serial.begin(9600);
}

void loop() 
{
  if(digitalRead(4)== HIGH)
  {
   WindSpeed();
   //WindDirection();
   digitalWrite(2,HIGH);
  }
  else
  {
   //WindDirection(); 
   digitalWrite(2,LOW);
  }
}
