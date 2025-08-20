#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <time.h>

#define EEPROM_SIZE 32
#define EEPROM_VALID_FLAG 0    // 1 byte
#define EEPROM_T1_FULL 1       // 4 bytes
#define EEPROM_T1_EMPTY 5      // 4 bytes
#define EEPROM_T2_FULL 9       // 4 bytes
#define EEPROM_T2_EMPTY 13     // 4 bytes

#define BUZZER D8
#define trigP D6
#define echoP D7
#define trigP2 D3
#define echoP2 D5

#define SECRET_SSID "**********"		// replace it with your WiFi network name
#define SECRET_PASS "**********"	// replace it with your WiFi password
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#define SECRET_CH_ID 111111			// replace it with your channel number
#define SECRET_WRITE_APIKEY "**********"   // replace it with your channel write API Key
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Set the LCD I2C address (default is 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // (I2C address, Columns, Rows)
bool isBacklightOn = true;  // Add this near top

int prevPercent = -1;
unsigned long prevTime = 0;

//TANK SPECIFICATIONS AND VARIABLES
#define TANK1_TOTAL_LITERS 500.0

float tank1FullHeight = 0;
float tank1EmptyHeight = 0;
float tank2FullHeight = 0;
float tank2EmptyHeight = 0;

bool tankHeightsSet = false;

//ThingSpeak DELAY
const unsigned long period = 40000;
String myStatus = "";

int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned long startMillis;
unsigned long currentMillis;

//WIFI VARIABLES
WiFiClient  client;

unsigned long lastWiFiAttempt = 0;
const unsigned long wifiRetryInterval = 30000; // Retry every 30 seconds

int lastGoodPercent[2] = { -1, -1 }; // For tank1 and tank2

// Initialize values
int tank1 = 0;
int tank2 = 0;
int prevTank1 = -1;
int prevTank2 = -1;

//FOR BUZZER
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
bool buzzerTriggeredOnce = false;

//CUSTOM CHARACTER DEFINITION
byte lowsym[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x1E};
byte halfsym[] = {0x00,0x00,0x00,0x00,0x1E,0x1E,0x1E,0x1E};
byte almstsym[] = {0x00,0x00,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
byte fullsym[] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
byte percentsym[] = {0x00,0x19,0x12,0x04,0x09,0x13,0x00,0x00};
uint8_t bell[8]  = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4};

//CLOCK FUNCTION
unsigned long lastClockUpdate = 0;
const unsigned long clockUpdateInterval = 5000; // 5 seconds

//NTP FUNCTION DEFINITION
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // India = GMT+5:30 → 5.5 * 3600 = 19800
const int daylightOffset_sec = 0;
bool ntpSynced = false;
unsigned long lastNtpCheck = 0;
const unsigned long ntpCheckInterval = 60000; // 1 minute

void checkNTP() {
  if (ntpSynced) return; // Skip if already synced

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("✅ NTP time synced.");
    ntpSynced = true;
  } else {
    Serial.println("❌ NTP not yet synced, will retry...");
  }
}

void manageLCDBacklight() {
  if (!ntpSynced) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int currentHour = timeinfo.tm_hour;
  if (currentHour >= 22 || currentHour < 6) {
    lcd.noBacklight();
    isBacklightOn = false;
  } else {
    lcd.backlight();
    isBacklightOn = true;
  }
}

void writeFloatToEEPROM(int addr, float value) {
  byte *data = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(float); i++) {
    EEPROM.write(addr + i, data[i]);
  }
}

float readFloatFromEEPROM(int addr) {
  float value = 0;
  byte *data = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(float); i++) {
    data[i] = EEPROM.read(addr + i);
  }
  return value;
}

void resetTankHeights() {
  Serial.println("🔁 Re-enter tank height values:");

  Serial.print("Enter Tank 1 Full Height (cm): ");
  while (Serial.available() == 0) {}
  tank1FullHeight = Serial.parseFloat();
  Serial.read();

  Serial.print("Enter Tank 1 Empty Height (cm): ");
  while (Serial.available() == 0) {}
  tank1EmptyHeight = Serial.parseFloat();
  Serial.read();

  Serial.print("Enter Tank 2 Full Height (cm): ");
  while (Serial.available() == 0) {}
  tank2FullHeight = Serial.parseFloat();
  Serial.read();

  Serial.print("Enter Tank 2 Empty Height (cm): ");
  while (Serial.available() == 0) {}
  tank2EmptyHeight = Serial.parseFloat();
  Serial.read();

  writeFloatToEEPROM(EEPROM_T1_FULL, tank1FullHeight);
  writeFloatToEEPROM(EEPROM_T1_EMPTY, tank1EmptyHeight);
  writeFloatToEEPROM(EEPROM_T2_FULL, tank2FullHeight);
  writeFloatToEEPROM(EEPROM_T2_EMPTY, tank2EmptyHeight);
  EEPROM.write(EEPROM_VALID_FLAG, 123);
  EEPROM.commit();

  Serial.println(" Tank heights updated and saved.");
  tankHeightsSet = true;
}


void DisplayWaterLevel(int sensorID, int percentage) {
    DisplayPercentage(sensorID, percentage);
    DisplayWaterBlocks(sensorID, percentage);
}

void DisplayPercentage(int sensorID, int percentage) {
    int row = (sensorID == 0) ? 0 : 1;

    lcd.setCursor(0, row);
    lcd.print(sensorID == 0 ? "H1:" : "H2:");

    lcd.setCursor(7, row);
    lcd.print("    ");  // Clear up to 3 digits + % (4 characters)

    lcd.setCursor(7, row);
    if (percentage < 0) {
      lcd.print("--");
      lcd.write(4);  // Print % symbol even with error
      } else {
        lcd.print(percentage);
        lcd.write(4);  // Always print after the number
        }
}

void DisplayWaterBlocks(int pos, int percentage) {
    // Determine the number of blocks (0 to 4) based on percentage
int blocks = (percentage >= 85) ? 4 :
             (percentage >= 75) ? 3 :
             (percentage >= 50) ? 2 :
             (percentage >= 25) ? 1 : 0;

    // Clear previous level indicators
    for (int i = 3; i <= 6; i++) {
        lcd.setCursor(i, pos);
        lcd.print(" ");
    }

    // Display new blocks
    for (int i = 0; i < blocks; i++) {
        lcd.setCursor(3 + i, pos);
        lcd.write(byte(i));
    }
}

void callingBuzzerAndBell() {
    static unsigned long lastToggleTime = 0;
    static bool buzzerState = false;

    if (!buzzerActive) {
        buzzerActive = true;
        buzzerStartTime = millis();
        lastToggleTime = millis();

        // Display bell icon once at start
        lcd.setCursor(11, 0);
        lcd.write(5);  // bell symbol
    }

    unsigned long elapsedTime = millis() - buzzerStartTime;
    Serial.print("elapsedTime :");
    Serial.println(elapsedTime);
    if (elapsedTime < 60000) {  // Beep for 1 minutes
      if (millis() - lastToggleTime >= 1000) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER, buzzerState ? HIGH : LOW);
      Serial.println(buzzerState ? "Buzzer ON" : "Buzzer OFF");
      lastToggleTime = millis();
  }

    } else {
        digitalWrite(BUZZER, LOW);
        buzzerActive = false;

        // Clear bell icon
        lcd.setCursor(11, 0);
        lcd.print(" ");
    }
}

int getFilteredDistance(int sensorID, int Trig, int Echo) {
  int readings[5];
  for (int i = 0; i < 5; i++) {
    readings[i] = ultra(sensorID, Trig, Echo);
    delay(50); // Short delay to avoid immediate back-to-back readings
  }

  // Sort and pick median
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (readings[j] < readings[i]) {
        int temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  return readings[2];  // Median value
}

int ultra(int sensorID, int Trig, int Echo) {
    digitalWrite(Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig, LOW);

    long duration = pulseIn(Echo, HIGH, 30000); // Timeout in 30ms
    if (duration == 0) {
        Serial.print("Ultrasonic read failed: No echo from sensor ");
        Serial.println(sensorID == 0 ? "Tank 1" : "Tank 2");
        return -1;
    }

    int distance = duration * 0.034 / 2;
    Serial.print("Distance from ");
    Serial.print(sensorID == 0 ? "Tank 1" : "Tank 2");
    Serial.print(": ");
    Serial.println(distance);
    return distance;
}
////////////////////////////////////////////////////////////////////////////////////////
int waterLevelStatus(int sensorID, int trigPin, int echoPin, float fullHeight, float emptyHeight) {
    float usableHeight = emptyHeight - fullHeight;
    //float distance = ultra(sensorID, trigPin, echoPin);
    float distance = getFilteredDistance(sensorID, trigPin, echoPin);

    if (distance < 0) {
        DisplayWaterLevel(sensorID, -1);
        return -1;
    }

    float currentLevel = constrain(emptyHeight - distance, 0, usableHeight);
    int percentage = (currentLevel / usableHeight) * 100;

    // ✅ Sudden drop filtering logic
    if (lastGoodPercent[sensorID] != -1 && abs(percentage - lastGoodPercent[sensorID]) > 30) {
        Serial.print("⚠️ Ignoring sudden spike/drop in Tank ");
        Serial.print(sensorID + 1);
        Serial.print(": "); Serial.print(lastGoodPercent[sensorID]);
        Serial.print(" → "); Serial.println(percentage);

        DisplayWaterLevel(sensorID, lastGoodPercent[sensorID]);
        return lastGoodPercent[sensorID];
    } else {
        lastGoodPercent[sensorID] = percentage;
    }

    DisplayWaterLevel(sensorID, percentage);
    Serial.print("Tank ");
    Serial.print(sensorID + 1);
    Serial.print(" Level: ");
    Serial.print(percentage);
    Serial.println("%");

    return percentage;
}
////////////////////////////////////////////////////////////////////////////////////////
void displayClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  char timeStr[6]; // HH:MM
  sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  lcd.setCursor(11, 1); // Row 1, Column 11
  lcd.print(timeStr);
}

////////////////////////////////////////////////////////////////////////////////////////
void checkWiFiStatus() {
    if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(10, 1);
        lcd.write(162);
        return;
    }

    if (millis() - lastWiFiAttempt < wifiRetryInterval) {
        lcd.setCursor(10, 1);
        lcd.write(163);  // Wi-Fi not connected icon
        return;
    }

    lastWiFiAttempt = millis();
    Serial.println("🔄 Reconnecting WiFi...");
    WiFi.begin(ssid, pass);
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 5000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Reconnected");
    } else {
        Serial.println("\n❌ WiFi Reconnect Failed");
    }

    lcd.setCursor(10, 1);
    lcd.write(WiFi.status() == WL_CONNECTED ? 162 : 163);
}
////////////////////////////////////////////////////////////////////////////////////////
void dynamicTimerem(int sensorID, int currentPercent) {
  static unsigned long lastCheckTime = 0;
  static int lastPercent = -1;
  static String lastEta = "    ";
  static unsigned long lastToggleTime = 0;
  static bool showLiters = false;

  if (sensorID != 0 || currentPercent < 0 || currentPercent >= 100) {
    lcd.setCursor(12, 0);
    lcd.print("    ");
    lastCheckTime = millis();
    lastPercent = currentPercent;
    lastEta = "    ";
    return;
  }

  unsigned long now = millis();

  // Update ETA every 10 seconds
  if ((now - lastCheckTime) >= 10000) {
    int deltaPercent = (lastPercent >= 0) ? (currentPercent - lastPercent) : 0;
    float deltaTimeMin = (lastPercent >= 0) ? ((now - lastCheckTime) / 60000.0) : 0;

    if (deltaPercent > 0 && deltaTimeMin > 0) {
      float fillSpeed = deltaPercent / deltaTimeMin;
      float remPercent = 100 - currentPercent;

      if (abs(deltaPercent) >= 2 && fillSpeed > 0.1) {
        float estMin = remPercent / fillSpeed;
        if (estMin >= 1.0) {
          lastEta = "E" + String((int)estMin) + "m";
        } else {
          int secs = (int)(estMin * 60);
          lastEta = "E" + String(secs) + "s";
        }
      }
    }

    // Fallback ETA
    if (currentPercent < 100 && lastEta == "    ") {
      float remPercent = 100 - currentPercent;
      float estMin = remPercent * 0.1;  // fallback: 10% per min
      lastEta = "E" + String((int)estMin) + "m";
    }

    lastCheckTime = now;
    lastPercent = currentPercent;
  }

  // Toggle between Liters and ETA every 3 seconds
  if (millis() - lastToggleTime >= 3000) {
    showLiters = !showLiters;
    lastToggleTime = millis();
  }

  lcd.setCursor(12, 0);
  lcd.print("    "); // Clear old

  if (showLiters) {
    // 🔁 Accurate liter calculation based on distance
    float radius = 43.33;           // cm
    float maxDistance = 85.0;       // cm (empty)
    float minDistance = 20.0;       // cm (full)
    float usableHeight = maxDistance - minDistance;

    float distance = getFilteredDistance(0, trigP, echoP);
    float waterHeight = constrain(maxDistance - distance, 0, usableHeight);

    float liters = 3.1416 * radius * radius * waterHeight * 0.001;
    int rounded = ((int)liters / 10) * 10;

    lcd.setCursor(12, 0);
    lcd.print(String(rounded) + "L");
  } else {
    lcd.setCursor(12, 0);
    lcd.print(lastEta);
  }
}


void WiFiThingspeak(int tank1, int tank2) {
    if (tank1 == prevTank1 && tank2 == prevTank2) return; // No change → exit

    checkWiFiStatus(); // Ensure WiFi is connected before updating

    ThingSpeak.setField(1, tank1);
    ThingSpeak.setField(2, tank2);

    int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (response == 200) {
        Serial.println("ThingSpeak update successful.");
        prevTank1 = tank1;
        prevTank2 = tank2;
    } else {
        Serial.println("ThingSpeak update failed. Error: " + String(response));
    }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(32);  // Initialize EEPROM with size

  // Wait for Serial (for boards like NodeMCU this isn't needed)
  while (!Serial) {
    delay(10);
  }

  // Load or set tank heights
  if (EEPROM.read(EEPROM_VALID_FLAG) == 123) {
    tank1FullHeight = readFloatFromEEPROM(EEPROM_T1_FULL);
    tank1EmptyHeight = readFloatFromEEPROM(EEPROM_T1_EMPTY);
    tank2FullHeight = readFloatFromEEPROM(EEPROM_T2_FULL);
    tank2EmptyHeight = readFloatFromEEPROM(EEPROM_T2_EMPTY);
    tankHeightsSet = true;
    Serial.println("✅ Tank heights loaded from EEPROM:");
    Serial.print("Tank 1 Full: "); Serial.println(tank1FullHeight);
    Serial.print("Tank 1 Empty: "); Serial.println(tank1EmptyHeight);
    Serial.print("Tank 2 Full: "); Serial.println(tank2FullHeight);
    Serial.print("Tank 2 Empty: "); Serial.println(tank2EmptyHeight);
  } else {
    Serial.println("🚨 First-time setup: Please enter tank levels");

    Serial.print("Enter Tank 1 Full Height (cm): ");
    while (Serial.available() == 0) {}
    tank1FullHeight = Serial.parseFloat();
    Serial.read();

    Serial.print("Enter Tank 1 Empty Height (cm): ");
    while (Serial.available() == 0) {}
    tank1EmptyHeight = Serial.parseFloat();
    Serial.read();

    Serial.print("Enter Tank 2 Full Height (cm): ");
    while (Serial.available() == 0) {}
    tank2FullHeight = Serial.parseFloat();
    Serial.read();

    Serial.print("Enter Tank 2 Empty Height (cm): ");
    while (Serial.available() == 0) {}
    tank2EmptyHeight = Serial.parseFloat();
    Serial.read();

    writeFloatToEEPROM(EEPROM_T1_FULL, tank1FullHeight);
    writeFloatToEEPROM(EEPROM_T1_EMPTY, tank1EmptyHeight);
    writeFloatToEEPROM(EEPROM_T2_FULL, tank2FullHeight);
    writeFloatToEEPROM(EEPROM_T2_EMPTY, tank2EmptyHeight);
    EEPROM.write(EEPROM_VALID_FLAG, 123);
    EEPROM.commit();

    Serial.println("✅ Tank heights saved to EEPROM.");
    tankHeightsSet = true;
  }

  // Initialize peripherals
  lcd.init();
  lcd.backlight();  // Default to backlight ON
  lcd.createChar(0, lowsym);
  lcd.createChar(1, halfsym);
  lcd.createChar(2, almstsym);
  lcd.createChar(3, fullsym);
  lcd.createChar(4, percentsym);
  lcd.createChar(5, bell);

  pinMode(trigP, OUTPUT);
  pinMode(echoP, INPUT);
  pinMode(trigP2, OUTPUT);
  pinMode(echoP2, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // After all init and before loop starts
int tempPercent = waterLevelStatus(0, trigP, echoP, tank1FullHeight, tank1EmptyHeight);

// 🧠 Avoid triggering buzzer immediately on reboot if tank is already full
if (tempPercent >= 85) {
  buzzerTriggeredOnce = true;
  Serial.println("🚫 Buzzer already triggered before reboot, skipping on boot.");
} else {
  buzzerTriggeredOnce = false;
}

  lcd.setCursor(0, 0);
  lcd.print("Welcome  Ranjith");
  //delay(3000);
  

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);

unsigned long wifiStartTime = millis();
const unsigned long wifiTimeout = 5000; // Try for max 5 seconds

while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < wifiTimeout) {
  delay(500);
  Serial.print(".");
}

lcd.clear(); //Until wifi getting connected lcd will display welcome message

if (WiFi.status() == WL_CONNECTED) {
  Serial.println("\n✅ WiFi Connected");
} else {
  Serial.println("\n⚠️ WiFi connection failed, continuing without it");
}
  // NTP Setup
  configTime(19800, 0, "pool.ntp.org");  // Set NTP but don't wait
  Serial.println("🔄 NTP setup called, not blocking for response.");

  ThingSpeak.begin(client);

  startMillis = millis();  // Initialize timestamp for periodic tasks
}

void loop() {
  manageLCDBacklight();

  // Check if Serial has new input
if (Serial.available()) {
  String input = Serial.readStringUntil('\n');
  input.trim();
  input.toUpperCase();
  if (input == "RESET") {
    resetTankHeights();  // Re-enter and update values
  }
}
    tank1 = waterLevelStatus(0, trigP, echoP, tank1FullHeight, tank1EmptyHeight);
    dynamicTimerem(0, tank1);
    delay(200);  // Wait before triggering the second sensor
    tank2 = waterLevelStatus(1, trigP2, echoP2, tank2FullHeight, tank2EmptyHeight);

    checkWiFiStatus();

    // Reset trigger flag when water drops below 50%
    if (tank1 < 50 && tank1 != -1) {
        buzzerTriggeredOnce = false;
    }

    // Start buzzer only once when tank1 crosses threshold
if (tank1 >= 85 && tank1 != -1 && !buzzerActive && !buzzerTriggeredOnce) {
  if (isBacklightOn) {
    callingBuzzerAndBell();
    buzzerTriggeredOnce = true;
    Serial.println("callingBuzzerAndBell started...");
  } else {
    Serial.println("Buzzer skipped due to night time (backlight off).");
  }
}
    // Keep toggling buzzer while it's active
    if (buzzerActive) {
      callingBuzzerAndBell();
      }
    if (millis() - startMillis >= period) {
      Serial.print(period / 1000);
      Serial.println(" seconds elapsed! Sending to ThingSpeak...");
      WiFiThingspeak(tank1, tank2);
      startMillis = millis();
      }
    if (millis() - lastNtpCheck >= ntpCheckInterval) {
      lastNtpCheck = millis();
      checkNTP();
      }
    if (millis() - lastClockUpdate >= clockUpdateInterval) {
      lastClockUpdate = millis();
      displayClock();
      }

    delay(5000);  // Reduce CPU load
}
