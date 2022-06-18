#include <LiquidCrystal.h>
#define BUZZER 8 //DEFINING PIN 8 AS BUZZER

const int RS=A5, E=A4, D4=A3, D5=A2, D6=A1, D7=A0; //DEFINING PINS FOR 16*2 LCD DISPLAY
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

//Custom Characters
byte Level0[8] = 
{0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b11111,
 0b11111};
byte Level1[8] = 
{0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b11111,
 0b11111,
 0b11111,
 0b11111};
byte Level2[8] = 
{0b00000,
 0b00000,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111};
byte Level3[8] = 
{0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111};
byte NoLevel[8] = 
{0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,};
byte speaker[8] = 
{0b00000,
 0b00010,
 0b00110,
 0b11110,
 0b11110,
 0b11110,
 0b00110,
 0b00010,};
byte soundwave[8] = 
{0b00100,
 0b10010,
 0b01010,
 0b01001,
 0b01001,
 0b01010,
 0b10010,
 0b00100};
byte plug[8] = 
{ 0b01010,
  0b01010,
  0b11111,
  0b10001,
  0b10001,
  0b01110,
  0b00100,
  0b00100}; 

//declaring variables 
int TriggerPin_1 = 12;
int TriggerPin_2 = 9;
int EchoPin_1    = 11;
int EchoPin_2    = 8;

float MaxWaterLevel_1, MaxWaterLevel_2;
float ActualReading_1, ActualReading_2;
float TankHeight_1, TankHeight_2;
float EmptySpace_1, EmptySpace_2;
int   Percentage_1 = 0;
int   Percentage_2 = 0;
float Temp_1, Temp_2;

long Distance_1;
long duration_1;
long Distance_2;
long duration_2;

float InvertPercentage;
float MinWithSec;
float DecimalSec;
int   Minutes;
int   Seconds;

//declaring functions
void ultra();
void callingbuzzer();
void nonblinkingsymbols();
void blinkingsymbols();
void ZeroPercentage();
void TwentyFivePercentage();
void FiftyPercentage();
void SeventyFivePercentage();
void HundredPercentage();
void ZeroPercentage_2();
void TwentyFivePercentage_2();
void FiftyPercentage_2();
void SeventyFivePercentage_2();
void HundredPercentage_2();


void setup()
{
  Serial.begin(9600);
  
  lcd.createChar(0, Level0);
  lcd.createChar(1, Level1);
  lcd.createChar(2, Level2);
  lcd.createChar(3, Level3);
  lcd.createChar(4, NoLevel);
  lcd.createChar(5, soundwave);
  lcd.createChar(6, speaker);
  lcd.createChar(7, plug);

  lcd.begin(16, 2);
  
//welcome note
  lcd.setCursor(0, 0);
  lcd.print("WELCOME  RANJITH");
  lcd.setCursor(1, 1);
  lcd.print(">>SAVE WATER<<");  
  delay(1000);
  lcd.setCursor(0,0);
  lcd.clear();
  
//initializing permanent characters
  lcd.setCursor(14, 1);
  lcd.write(byte(6));
  delay(300);
  lcd.setCursor(15, 1);
  lcd.write(byte(5));
  delay(300);
  lcd.setCursor(12, 1);
  lcd.write(byte(7));
  delay(300);
  
  lcd.setCursor(0,0);
  lcd.print("H1:");
  lcd.setCursor(0,1);
  lcd.print("H2:");
  
  pinMode(TriggerPin_1, OUTPUT);
  pinMode(EchoPin_1, INPUT);
  pinMode(TriggerPin_2, OUTPUT);
  pinMode(EchoPin_2, INPUT);
  
  TankHeight_1    = 76.2; //approx for 500 liter (30)
  MaxWaterLevel_1 = 0.85*TankHeight_1;
  EmptySpace_1    = TankHeight_1 - MaxWaterLevel_1;

  TankHeight_2    = 140;//122.42; //approx for 1000 liter(48.2)
  MaxWaterLevel_2 = 0.85*TankHeight_2;
  EmptySpace_2    = TankHeight_2 - MaxWaterLevel_2;
  
  Serial.print("Height of the Tanks : ");
  Serial.print(TankHeight_1);
  Serial.print(" and ");
  Serial.println(TankHeight_2);
  
  Serial.print("Decided Maximum Water Levels : ");
  Serial.print(MaxWaterLevel_1);
  Serial.print(" and ");
  Serial.println(MaxWaterLevel_2);
}

void loop()
{
  delay(2000);
  
  ultra();
  
  Temp_1 = Distance_1 - EmptySpace_1;
  ActualReading_1 = MaxWaterLevel_1 - Temp_1;
  Percentage_1 = (ActualReading_1 / MaxWaterLevel_1 *100);

  Temp_2 = Distance_2 - EmptySpace_2;
  ActualReading_2 = MaxWaterLevel_2 - Temp_2;
  Percentage_2 = (ActualReading_2 / MaxWaterLevel_2 *100);

  InvertPercentage = 100 - Percentage_1;
  MinWithSec = InvertPercentage / 8;//speed = 8
  Minutes = MinWithSec;
  DecimalSec = MinWithSec - Minutes;
  Seconds = DecimalSec * 60;
   
  lcd.setCursor(11,0);
  lcd.print(Minutes);
  lcd.print(":");
  lcd.print(Seconds);
  lcd.print(" ");
  
  Serial.print("Percentage of Water in Tank :");
  Serial.print(Percentage_1);
  Serial.print(" and ");
  Serial.println(Percentage_2);
  
      lcd.setCursor(7,0); 
      lcd.print(Percentage_1);
      lcd.print("% ");
      
      lcd.setCursor(7,1); 
      lcd.print(Percentage_2);
      lcd.print("% ");       
       
    if(Percentage_1<=20)
      {
        ZeroPercentage();
        blinkingsymbols();
        if(Percentage_1 < 10)
        {
          callingbuzzer();
        }
      }
    else if(Percentage_1>20 && Percentage_1<=25)
      {
        TwentyFivePercentage();
        nonblinkingsymbols();
      }
    else if(Percentage_1>25 && Percentage_1<=50)
      {
        FiftyPercentage();
        nonblinkingsymbols();
      }     
    else if(Percentage_1>50 && Percentage_1<=75)
      {
         SeventyFivePercentage();  
         nonblinkingsymbols();
      }
    else if(Percentage_1>75 && Percentage_1<=100)
      {
          HundredPercentage();
          blinkingsymbols();
          if(Percentage_1 < 95)
        {
          callingbuzzer();
        }
      }
/////////////////////////////////////////////////////////////////////////////

    if(Percentage_2<=20)
      {
        ZeroPercentage_2();
        /*blinkingsymbols();
         if(Percentage_2 <= 15)
        {
          callingbuzzer();
        }*/
      }
    else if(Percentage_2>20 && Percentage_2<=25)
      {
        TwentyFivePercentage_2();
        //nonblinkingsymbols();
      }
    else if(Percentage_2>25 && Percentage_2<=50)
      {
        FiftyPercentage_2();
        //nonblinkingsymbols();
      }     
    else if(Percentage_2>50 && Percentage_2<=75)
      {
         SeventyFivePercentage_2();
         //nonblinkingsymbols();  
      }
    else if(Percentage_2>75 && Percentage_2<=100)
      {
          HundredPercentage_2();
          /*blinkingsymbols();
         if(Percentage_2 > 85)
        {
          callingbuzzer();
        }*/
      }
    else if(Percentage_2=117)
      {
       lcd.setCursor(3,1); 
       lcd.print(">ERROR< ");        
      }
}
