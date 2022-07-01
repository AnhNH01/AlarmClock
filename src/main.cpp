#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <SPI.h>

#define TIME 0 // show time
#define DATE_SET 1
#define ALARM_SET_1 2 // alarm 1
#define ALARM_SET_2 3 // alarm 2
#define ALARM_SET_3 4 // alarm 3
#define TIME_SET 5 // set time


struct Alarm {
  int flag; // used to differentiate alarms
  int On;  // 0 -> Off , flag --> On
  int hour;
  int minute;
  int second = 0;
};


RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

Alarm alarm1, alarm2, alarm3;

const int btn1Pin = 2;
const int btn2Pin = 3;
const int btn3Pin = 4;
const int btn4Pin = 5;
const int buzzerPin = 10;

// state of the programe 
int state; 

// contains all the ON alarm
int AlarmOn  = 0; 

int setTimeHour;
int setTimeMinute;
int setDay;
int setMonth;
int setYear;


void showTime();
void setAlarm(Alarm &alarm);
void setTime();
void setDate();
void printTwoDigits(int t);
void Buzz(DateTime &now, int AlarmOn);

int check(DateTime &now,const Alarm &alarm);



void setup () 
{ 
  // Init the LCd
  lcd.init(); 

  // Turn on backlight
  lcd.backlight();
  
  // No buzz
  noTone(buzzerPin);
  
  //Show time
  state = TIME;

  // Set default state of the alarms
  alarm1.flag = 1;
  alarm1.On = 0;
  alarm1.hour = 12;
  alarm2.minute = 0;

  alarm2.flag = 3;
  alarm2.On = 0;
  alarm2.hour = 13;
  alarm2.minute = 0;

  alarm3.flag = 5;
  alarm3.On = 0;
  alarm3.hour = 14;
  alarm3.minute = 0;
  
  // Set Pin Mode
  pinMode(btn1Pin, INPUT);
  pinMode(btn2Pin, INPUT);
  pinMode(btn3Pin, INPUT);
  pinMode(btn4Pin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  if (! rtc.begin()) 
  {
    lcd.print("Couldn't find RTC");
    while (1);
  }
  if (!rtc.isrunning()) 
  {
    lcd.print("RTC is NOT running!");
  }
  
}


int btn1Pressed, btn2Pressed, btn3Pressed, btn4Pressed;
void loop () 
{

  if(state == TIME){
    showTime();
    
    // Change state to set date
    btn1Pressed = digitalRead(btn1Pin);
    if(btn1Pressed == HIGH) 
    {
      delay(200);
      if(state == TIME) 
      {
        lcd.clear();
        state = DATE_SET;
        setDate();
      }
    }

    // Turn Off Buzzer, turn of the matching alarm
    btn2Pressed = digitalRead(btn2Pin);
    if (btn2Pressed){
      noTone(buzzerPin);
      
      DateTime now = rtc.now();
      if(check(now, alarm1))
        alarm1.On = 0;
      
      if(check(now, alarm2))
        alarm2.On = 0;
      
      if(check(now, alarm3))
        alarm3.On = 0;
    }
  }

  if(state == ALARM_SET_1)
    setAlarm(alarm1);
  
  else if(state == ALARM_SET_2)
    setAlarm(alarm2);
  
  else if(state == ALARM_SET_3)
    setAlarm(alarm3);
  else if (state == TIME_SET)
    setTime();
}


void printTwoDigits(int t)
{
  if(t < 10) 
  {
    lcd.print("0");
    lcd.print(t);
  }
  else 
    lcd.print(t);
}


void showTime() {
  DateTime now = rtc.now();
  
  setDay = now.day();
  setMonth = now.month();
  setYear = now.year();

  // Print Time
  lcd.setCursor(0, 0);
  lcd.print("TIME: ");
  printTwoDigits(now.hour());
  lcd.print(':');
  printTwoDigits(now.minute());
  lcd.print(':');
  printTwoDigits(now.second());

  // Print date
  lcd.setCursor(0,1);
  lcd.print("DATE: " + (String)now.day() + '/' + (String)now.month() + '/' + (String)now.year());

  AlarmOn = alarm1.On + alarm2.On + alarm3.On;
  Buzz(now, AlarmOn);
  delay(100);
}


void setDate() 
{
  lcd.setCursor(0,0);
  lcd.print("Set Date:");

  while (state == DATE_SET)
  {
    lcd.setCursor(1, 1);
    lcd.print((String)setDay + '/' + (String)setMonth + '/' + (String)setYear +"  ");

    // Increment day
    btn2Pressed = digitalRead(btn2Pin);
    if(btn2Pressed)
    {
      delay(200);
      setDay = (setDay + 1) % 32;
    }

    // Increment month
    btn3Pressed = digitalRead(btn3Pin);
    if(btn3Pressed)
    {
      delay(200);
      setMonth = (setMonth + 1) % 13;
    }

    // Increment year, max is 2032
    btn4Pressed = digitalRead(btn4Pin);
    if(btn4Pressed)
    {
      delay(200);
      setYear += 1;
      if(setYear > 2032)
        setYear = 2022;
    }

    // Set date, change state to ALARM_SET_1
    btn1Pressed = digitalRead(btn1Pin);
    if(btn1Pressed)
    {
      lcd.clear();
      delay(200);
      rtc.adjust(DateTime(setYear, setMonth, setDay, rtc.now().hour(), rtc.now().minute(), rtc.now().second()));
      state = ALARM_SET_1;
    }
  }
  
}



void setAlarm(Alarm &alarm) {
  int id;
  if(alarm.flag == 1)
    id = 1;
  else if (alarm.flag == 3)
    id = 2;
  else 
    id = 3;
  
  while(1) {
    lcd.setCursor(0, 0);
    if(alarm.On)
      lcd.print("Alarm " + (String)id + ": ON  ");
    else
      lcd.print("Alarm " + (String)id + ": OFF");
    
    lcd.setCursor(1,3);
    printTwoDigits(alarm.hour);
    lcd.print(":");
    printTwoDigits(alarm.minute);
    lcd.print(":");
    printTwoDigits(alarm.second);
    
    // Change state depend on which alarm
    btn1Pressed = digitalRead(btn1Pin);
    if(btn1Pressed)
    {
      delay(200);
      lcd.clear();
      if(id == 1)
        state = ALARM_SET_2;
      else if (id == 2)
        state = ALARM_SET_3;
      else
        state = TIME_SET;
      break;
    }

    // Increment Hour
    btn2Pressed = digitalRead(btn2Pin);
    if(btn2Pressed)
    {
      delay(200);
      alarm.hour = (alarm.hour + 1) % 24;
    }

    // Increment Minute
    btn3Pressed = digitalRead(btn3Pin);
    if(btn3Pressed)
    {
      delay(200);
      alarm.minute = (alarm.minute + 1) % 60;
    }
    
    // Turn alarm on or off
    btn4Pressed = digitalRead(btn4Pin);
    if(btn4Pressed)
    {
      delay(200);
      if(alarm.On)
        alarm.On = 0;
      else 
        alarm.On = alarm.flag;
    }
  }
}



void setTime()
{
  setTimeHour = rtc.now().hour();
  setTimeMinute = rtc.now().minute();
  lcd.setCursor(0,0);
  lcd.print("Set Time");

  while (state == TIME_SET)
  {
    lcd.setCursor(1, 3);
    printTwoDigits(setTimeHour);
    lcd.print(":");
    printTwoDigits(setTimeMinute);
    lcd.print(":00");
    

    // Change state to TIME, dont Update time
    btn1Pressed = digitalRead(btn1Pin);
    if(btn1Pressed)
    {
      delay(200);
      lcd.clear();
      state = TIME;
    }   

    // Increment Hour
    btn2Pressed = digitalRead(btn2Pin);
    if(btn2Pressed)
    {
      delay(200);
      setTimeHour = (setTimeHour + 1) % 24;
    }

    // Increment Minute
    btn3Pressed = digitalRead(btn3Pin);
    if(btn3Pressed)
    {
      delay(200);
      setTimeMinute = (setTimeMinute + 1) % 60;
    }

    // Update time, change state to TIME
    btn4Pressed = digitalRead(btn4Pin);
    if(btn4Pressed)
    {
      delay(200);
      rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), setTimeHour, setTimeMinute, 0));
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Time saved");
      delay(1000);
      lcd.clear();
      state = TIME;
    }
    
  }
}



// returns 1 if match time, 0 if not
int check(DateTime &now,const Alarm &alarm)
{
  if(now.hour() == alarm.hour && now.minute() == alarm.minute)
    return 1;
  else
    return 0;
}


void Buzz(DateTime &now, int AlarmOn)
{
  switch (AlarmOn)
  {
    case 1:
      if(check(now, alarm1))
        tone(buzzerPin, 250);
      else 
        noTone(buzzerPin);
      break;
    
    case 3: 
      if(check(now, alarm2))
        tone(buzzerPin, 250);
      else
        noTone(buzzerPin);
      break;
    
    case 5: 
      if(check(now, alarm3))
        tone(buzzerPin, 250);
      else 
        noTone(buzzerPin);
      break;
    
    case 4:
      if(check(now, alarm1) || check(now, alarm2))
        tone(buzzerPin, 250);
      else 
        noTone(buzzerPin);
      break; 

    case 6:
      if(check(now,alarm1) || check(now, alarm3))
        tone(buzzerPin, 250);
      else
        noTone(buzzerPin);
      break;
    
    case 8:
      if(check(now, alarm2) || check(now, alarm3))
        tone(buzzerPin, 250);
      else 
        noTone(buzzerPin);
      break;
    
    case 9:
      if(check(now, alarm1) || check(now, alarm2) || check(now, alarm3))
        tone(buzzerPin, 250);
      else 
        noTone(buzzerPin);
      break;

    default:
      noTone(buzzerPin);
      break;
  }

}


/*
State:
TIME -> set DATE -> Alarm 1 -> Alarm 2 --> Alarm 3 --> SET TIME  
0           1          2           3          4            5
^                                                         | 
|_________________________________________________________|

In state 0:
  Button 1: chage state
  Button 2: set hour
  Button 3: set minute
  Button 4: CHange to setTime state

In state 1: 
  Button 1: change state
  Button 2: set day
  Button 3: set month
  Button 4: set year

In state 2, 3, 4:
  Button 1: changeState
  Button 2: setHour
  Button 3: setMinute
  Button 4: turn alarm on/off

In state 5
  Button 1: changeState to TIME, dont update time
  Button 2: setHour
  Button 3: setMinute
  Button 4: changState to TIME, update time
*/
