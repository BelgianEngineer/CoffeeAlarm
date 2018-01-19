#include "application.h"                                                        // STATUS VARIABLE
#include "LiquidCrystal.h"

LiquidCrystal lcd(D0, D1, D2, D3, D4, D5);

const int relayPin = D7;                                      // TO BE PUT ON D6

int brewTime = 30;          //Brew time (s)
int extraTime = 10;         //Extra heating time (min)

int alarmHour1;  //Start time
int alarmMinute1;
int alarmHour2;  //Alarm time
int alarmMinute2;
bool alarmFlag = false;
String alarmStatus = "off";

//String message = "No message";
String yourMessage = "No message";


//#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
//unsigned long lastSync = millis();

//void loop() {
//  if (millis() - lastSync > ONE_DAY_MILLIS) {
//    // Request time synchronization from the Particle Cloud
//    Particle.syncTime();
//    lastSync = millis();
//  }
//}


void setup()
{

  Serial.begin(9600);


  Particle.syncTime();
  Time.zone(+1);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  lcd.begin(16,2);

  Particle.function("setAlarm",setAlarm);
  Particle.function("alarmOnOff",alarmOnOff);
  Particle.function("brew", brew);
  Particle.function("setMessage", setMessage);
  Particle.variable("alarmStatus", &alarmStatus, STRING);
  Particle.variable("alarmHour", &alarmHour2, INT);
  Particle.variable("alarmMinute", &alarmMinute2, INT);
  Particle.variable("yourMessage", &yourMessage, STRING);
}

void loop()
{
  // NORMAL TIME
  updateLcd(alarmMessage());

  Serial.println("Local: " + String(Time.hour(),DEC) + String(Time.minute(),DEC));
  Serial.println("Alarm: " + String(alarmHour1,DEC) + String(alarmMinute1,DEC));

  // BREWING TIME
  if (Time.hour() == alarmHour1 && Time.minute() == alarmMinute1 && alarmFlag == true)
  {
      brew(yourMessage);
  }

  delay(1000);
}



//-------------------------FUNCTIONS------------------------------

int setAlarm(String alarm) // alarm is HHMM
{
  alarmHour2 = (alarm.substring(0,2)).toInt();
  alarmMinute2 = (alarm.substring(2,4)).toInt();

  int timeValue2 = alarmHour2*60 + alarmMinute2;
  int timeValue1 = timeValue2 - (brewTime/60);
  if (timeValue1 > 0)
  {
    alarmMinute1 = timeValue1 % 60;
//    Serial.println("Minute1: "String(alarmMinute1,DEC));
    alarmHour1 = (timeValue1 - alarmMinute1)/60;
//    Serial.println("Minute1: "String(alarmMinute1,DEC));
  }
  else
  {
    alarmMinute1 = 60 - (timeValue1 % 60);
    alarmHour1 = 24 + (timeValue1 - alarmMinute1)/60;
  }

  alarmOnOff("on");
}

int alarmOnOff(String status) // alarm on or off
{
  if (status == "on")
  {
    alarmFlag = true;
    alarmStatus = "on";
//    message = alarmMessage();
    updateLcd("Alarm on");
  }

  if (status == "off")
  {
    alarmFlag = false;
    alarmStatus = "off";
//    message = alarmMessage();
    updateLcd("Alarm off");
  }
delay(1000);
}

int setMessage(String message)
{
  yourMessage = message;
  updateLcd("Updating msg...");
  delay(1000);
}

int brew(String brewMessage)
{
  digitalWrite(relayPin, HIGH); //Launch coffee machine

  long t = 0;
  int tToggle = 2;                                                              // message toggle time (s)
  while (t <= brewTime)
  {
    updateLcd("Brewing...");
    delay(tToggle*1000);
    updateLcd(alarmMessage());
    delay(tToggle*1000);
    t = t + 2*tToggle;
  }

  t = 0;                                                          // Time message is displayed and continues heating (min)
  while (t <= extraTime*60) // No second brew in same minute
  {
    updateLcd(brewMessage);
    delay(15000);
    t = t + 15;
  }

  digitalWrite(relayPin, LOW); //Stop coffee machine
}

void updateLcd(String line2)
{
  lcd.clear();
  lcd.print(Time.timeStr(Time.now()));
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

String alarmMessage()
{
  String zeroPad1 = "";
  if (alarmHour2 < 10)
  {
    zeroPad1 = "0";
  }
  String zeroPad2 = "";
  if (alarmMinute2 < 10)
  {
    zeroPad2 = "0";
  }
  return "Alarm: " + zeroPad1 + String(alarmHour2) + ":" + zeroPad2 + String(alarmMinute2)+ " " + alarmStatus;
}
