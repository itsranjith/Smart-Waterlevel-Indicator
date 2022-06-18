void ultra()
{
  digitalWrite(TriggerPin_1, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin_1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin_1, LOW);
  duration_1 = pulseIn(EchoPin_1, HIGH);
  Distance_1 = duration_1*0.034/2; 

  digitalWrite(TriggerPin_2, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin_2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin_2, LOW);
  duration_2 = pulseIn(EchoPin_2, HIGH);
  Distance_2 = duration_2*0.034/2;   
}

void callingbuzzer()
{
  digitalWrite(BUZZER,HIGH);
  delay(500);
  digitalWrite(BUZZER,LOW);
}
void nonblinkingsymbols()
{
  lcd.setCursor(14, 1);
  lcd.write(byte(6));
  lcd.setCursor(15, 1);
  lcd.write(byte(5));
  lcd.setCursor(12, 1);
  lcd.write(byte(7));
}
void blinkingsymbols()
{
  lcd.setCursor(15, 1);
  lcd.write(byte(5));
  lcd.setCursor(12, 1);
  lcd.write(byte(7));
  delay(300);
  lcd.setCursor(15, 1);
  lcd.print(" ");
  lcd.setCursor(12, 1);
  lcd.print(" ");
}

void ZeroPercentage()
{
  lcd.setCursor(3, 0);
  lcd.write(byte(4));
  lcd.setCursor(4, 0);
  lcd.write(byte(4));
  lcd.setCursor(5, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  
}
void ZeroPercentage_2()
{  
  lcd.setCursor(3, 1);
  lcd.write(byte(4));
  lcd.setCursor(4, 1);
  lcd.write(byte(4));
  lcd.setCursor(5, 1);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(4));
  
}
void TwentyFivePercentage()
{
  lcd.setCursor(3, 0);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(4));
  lcd.setCursor(5, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  
}
void TwentyFivePercentage_2()
{
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 1);
  lcd.write(byte(4));
  lcd.setCursor(5, 1);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(4));
  
}
void FiftyPercentage()
{
  lcd.setCursor(3, 0);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
}
void FiftyPercentage_2()
{
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 1);
  lcd.write(byte(1));
  lcd.setCursor(5, 1);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(4));
}
void SeventyFivePercentage()
{
  lcd.setCursor(3, 0);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 0);
  lcd.write(byte(2));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  
}
void SeventyFivePercentage_2()
{
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 1);
  lcd.write(byte(1));
  lcd.setCursor(5, 1);
  lcd.write(byte(2));
  lcd.setCursor(6, 1);
  lcd.write(byte(4));
  
}
void HundredPercentage()
{
  lcd.setCursor(3, 0);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 0);
  lcd.write(byte(2));
  lcd.setCursor(6, 0);
  lcd.write(byte(3));
  
}
void HundredPercentage_2()
{
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 1);
  lcd.write(byte(1));
  lcd.setCursor(5, 1);
  lcd.write(byte(2));
  lcd.setCursor(6, 1);
  lcd.write(byte(3));
  
}
