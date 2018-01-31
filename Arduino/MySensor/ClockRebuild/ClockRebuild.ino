//## INCLUDES ##
// MySensors
#define MY_DEBUG 
#define MY_RADIO_NRF24
#define MY_NODE_ID 9
//#define MY_RF24_CHANNEL  125
//#define MY_RF24_PA_LEVEL RF24_PA_MAX
//#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h> 
#define cID_TEMP 0
#define cID_ALARM_TIME 1
#define cID_DIM_LED_RING 2
#define cID_ALARM_ACTIVE 3
// RTC
#include "DS3232RTC.h"
#include <TimeLib.h>
#include "TimeAlarms.h"
// Keypad
#include "AnalogMatrixKeypad.h"
// General
#include <Wire.h>
#include <SPI.h>
// 4-digit display 
#include "TM1637.h"
// Speaker
#include "pitches.h"

//## VARIABLES ##
// MySensors
#define MySensors_SketchName      "WakeUp Clock"
#define MySensors_SketchVersion   "0.0.3"
bool MySensors_TimeReceived = false;
unsigned long MySensors_LastUpdate=0, MySensors_pMillis=0;
MyMessage MySensors_MSG_Temp(cID_TEMP, V_TEMP);
bool MySensors_RequestACK = true;
MyMessage MySensors_MSG_Alarm1_Time(cID_ALARM_TIME, V_TEXT);
MyMessage MySensors_DIM_LED_RING(cID_DIM_LED_RING, V_DIMMER);
MyMessage MySensors_SWITCH_LED_RING(cID_DIM_LED_RING, V_LIGHT);
MyMessage MySensors_MSG_Alarm_Active(cID_ALARM_ACTIVE, V_LIGHT);
static int16_t LED_CurrentLevel = 0;  // Current dim level...
// 4-digit display
unsigned long Screen_pMillis = 0;
long Screen_sCheck = 1000;
const int Screen_PIN_clk = 2;
const int Screen_PIN_dta = 3;
TM1637 ledScreen(Screen_PIN_clk, Screen_PIN_dta);
int8_t Screen_Display[] = {0x00,0x00,0x00,0x00};//blank out the screen
unsigned char Screen_ClockPoint = 1;//on/off for time double dots
bool Screen_HourFormat24 = false;
int Screen_Brightness = 0;
// Alarm
unsigned long Alarm_pMillis = 0;
long Alarm_sCheck = 1000;
unsigned long Alarm_Request_pMillis = 0;
long Alarm_Request_sCheck = 60000;// 5 miuntes = 300,000
AlarmId Alarm1;
int Alarm1_Time[] = {6,45,0};
int Alarm1_Time_Local[] = {0,0,0};
int Alarm1_Time_Remote[] = {0,0,0};
bool Alarm1_Set_Locally = false;
bool Alarm1_Set_Remotely = false;
bool Alarm1_Enabled = false;
bool Alarm1_ALARMING = false;
bool Alarm1_Local_Change = false;
unsigned long Alarm1_Local_Change_pMillis = 0;
long Alarm1_Local_Change_sCheck = 3000;
// RTC
unsigned long RTC_Temp_pMillis = 0;
long RTC_Temp_sCheck = 300000;// 5 miuntes = 300,000
unsigned int RTC_Temp = -1;
// Keypad
#define Keys_PIN A0
AnalogMatrixKeypad Keys_Keypad1x5(Keys_PIN);
unsigned long Keys_pMillis = 0;
long Keys_sCheck = 100;
bool Keys_ShowAlarmOnScreen = false;
bool Keys_Debug = true;
// LED
int LED_PIN = 6;
//int LED[] = {0,0};
// LED WakeUp Light
unsigned long LED_Sunrise_pMillis = 0;
long LED_Sunrise_sCheck = 150;
int LED_Brightness = 0;
int LED_Steps = 1;
bool LED_Sunrise = false;
unsigned long LED_Flash_pMillis = 0;
long LED_Flash_sCheck = 200;
bool LED_Flash = false;
bool LED_Flash_State = false;
// LED Night Light
bool LED_Night_Light_On = false;
int LED_Night_LightBrightness = 10;
// Speaker
int Speaker_PIN = 5;
unsigned long Speaker_pMillis = 0;
long Speaker_sCheck = 0;
int Speaker_Melody[] = {NOTE_E4,NOTE_E4,REST,NOTE_E4,REST,NOTE_C4,NOTE_E4,REST,NOTE_G4,REST,REST,NOTE_G3,REST,      NOTE_C4,REST,REST,NOTE_G3,REST,NOTE_E3,REST,REST,NOTE_A3,REST,NOTE_B3,REST,NOTE_AS3,NOTE_A3,REST,     NOTE_G3,NOTE_E4,NOTE_G4,NOTE_A4,REST,NOTE_F4,NOTE_G4,REST,NOTE_E4,REST,NOTE_C4,NOTE_D4,NOTE_B3,REST,      NOTE_C4,REST,REST,NOTE_G3,REST,NOTE_E3,REST,REST,NOTE_A3,REST,NOTE_B3,REST,NOTE_AS3,NOTE_A3,REST,     NOTE_G3,NOTE_E4,NOTE_G4,NOTE_A4,REST,NOTE_F4,NOTE_G4,REST,NOTE_E4,REST,NOTE_C4,NOTE_D4,NOTE_B3,REST};
int Speaker_Notes[]={4,4,4,4,4,4,4,4,4,2,4,2,2,     4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,      4,2,4,4,4,4,4,4,4,4,4,4,4,2,      4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,      4,2,4,4,4,4,4,4,4,4,4,4,4,2};
const int Speaker_Note_NumberOfNotes = sizeof(Speaker_Notes)/sizeof(Speaker_Notes[0]);
int Speaker_Current_Note = 0;
int Speaker_Current_Note_Duration = 0;
int Speaker_Current_Pause = 0;
bool Speaker_Playing = false;

void setup() {
  //## Serial ##
  Serial.begin(115200);
  while (!Serial) ;
  Serial.print("compiled: ");Serial.print(__DATE__);Serial.println(__TIME__);//Base Details

  //## RTC ##
  setSyncProvider(RTC.get); //Sync arduino time with RTC time
  Update_Alarm1_From_Controller();
  //Alarm1 = Alarm.alarmRepeat(Alarm1_Time[0],Alarm1_Time[1],Alarm1_Time[3], MorningAlarm);
  requestTime();  

  //## LED ##
  pinMode(LED_PIN, OUTPUT);
  
  //## Keypad ##
  Keys_Keypad1x5.setDebounceTime(200);
  Keys_Keypad1x5.setThresholdValue(7);// default 4
  Keys_Keypad1x5.setNumberOfKeys(5);

  request( cID_DIM_LED_RING, V_DIMMER );
}
void presentation()  {
  sendSketchInfo(MySensors_SketchName, MySensors_SketchVersion);
  
  present(cID_ALARM_TIME, S_INFO, "Alarm Time", MySensors_RequestACK);
  present(cID_TEMP, S_TEMP, "RTCembeded Temperature", MySensors_RequestACK);
  present(cID_DIM_LED_RING, S_DIMMER , "LED Ring Light", MySensors_RequestACK);
  present(cID_ALARM_ACTIVE, S_LIGHT , "Alarm Active", MySensors_RequestACK);
}
void loop() {
  unsigned long currentMillis = millis();

  if ((!MySensors_TimeReceived && (currentMillis-MySensors_pMillis) > (10UL*1000UL)) || (MySensors_TimeReceived && (currentMillis-MySensors_pMillis) > (60UL*1000UL*60UL))) {// Request time from controller. 
    requestTime();//Serial.println("requesting time");
    MySensors_pMillis = currentMillis;
  }

  Check_Speaker(currentMillis);
  Update_Screen(currentMillis);// Update display every second
  Check_KeyPad(currentMillis);// check for key press
  Check_RTC_Temp(currentMillis);
  Check_Local_Alarm_Change(currentMillis);
  Update_Alarm1_From_Controller(currentMillis);
  if (Alarm1_ALARMING){
    Update_LED_Sunrise(currentMillis);
    Update_LED_Flash(currentMillis);
  }

  if (currentMillis - Alarm_pMillis >= Alarm_sCheck){
    Alarm.manualServicing();
    Alarm_pMillis = currentMillis;
  }
}
// ## MySensors Methods
void receiveTime(unsigned long controllerTime) {
  //Serial.print("Time value received: ");Serial.println(controllerTime);// Ok, set incoming time 
  RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  MySensors_TimeReceived = true;
}
void receive(const MyMessage &message) {

  if (message.type == V_LIGHT || message.type == V_DIMMER) {
    int requestedLevel = atoi( message.data );//  Retrieve the power or dim level from the incoming request message
    requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );// Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
    requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;// Clip incoming level to valid range of 0 to 100
    requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;
    int requestedLevel_mapped;

    if (message.sensor == cID_DIM_LED_RING) {
      requestedLevel_mapped = map(requestedLevel, 0, 100, 0, 255);
    }
    #ifdef MY_DEBUG
    Serial.print( "Changing level to " );
    Serial.print( requestedLevel );
    Serial.print( " (" );
    Serial.print( requestedLevel_mapped );
    Serial.print( ") , from " );
    Serial.println( LED_CurrentLevel );
    #endif

    if (message.sensor == cID_DIM_LED_RING) {
      #ifdef MY_DEBUG
      Serial.println ("LED Ring Light Dimmer data");
      #endif
      LED_CurrentLevel = requestedLevel;        
      //LED[0] = LED_CurrentLevel == 0 ? 0 : 1;
      //LED[1] = requestedLevel_mapped;
      LED_Night_Light_On=true;
      analogWrite(LED_PIN, requestedLevel_mapped);
      send(MySensors_DIM_LED_RING.set(LED_CurrentLevel > 0));// Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
      //send(MySensors_SWITCH_LED_RING.set(LED_CurrentLevel) );// hek comment: Is this really nessesary?
    } else if (message.sensor == cID_ALARM_ACTIVE){
      if ((requestedLevel == 0) && (Alarm1_ALARMING)) {
        Alarm1_ALARMING = false;
        Stop_Alarm();
      }
    }
  }

  if (message.type==V_TEXT) {
    if (message.sensor == cID_ALARM_TIME){
      String msg = message.getString();
      msg.trim();
      msg.toLowerCase();
      if ((msg == "off") && (Alarm1_Time_Remote[0] + Alarm1_Time_Remote[1] + Alarm1_Time_Remote[2] != 0)){
        Alarm1_Enabled = false;
        //Alarm.free(Alarm1);
        Alarm1_Time_Remote[0] = 0;Alarm1_Time_Remote[1] = 0;Alarm1_Time_Remote[2] = 0;
      }else if (msg != "off"){
        if ((Alarm1_Time_Remote[0] + Alarm1_Time_Remote[1] + Alarm1_Time_Remote[2] == 0) && (Alarm1_Time_Local[0] + Alarm1_Time_Local[1] + Alarm1_Time_Local[2] == 0)){
          Alarm1_Set_Locally = false;Alarm1_Set_Remotely = true;
          Alarm1_Time_Local[0] = Alarm1_Time_Remote[0];
          Alarm1_Time_Local[1] = Alarm1_Time_Remote[1];
        }
        int commaIndex = msg.indexOf('.');
        if(commaIndex >0){
          int newHour = msg.substring(0, commaIndex).toInt();
          int newMinute = msg.substring(commaIndex+1).toInt();
          if ((Alarm1_Time_Remote[0] != newHour) || (Alarm1_Time_Remote[1] != newMinute)){
            Alarm1_Time_Remote[0] = newHour;Alarm1_Time_Remote[1] = newMinute;Alarm1_Time_Remote[2] = 0;
            Alarm1_Set_Locally = false;Alarm1_Set_Remotely = true;
            ChangeAlarm1();
          }
        }
      }
    }
    //Serial.print("Message: "); Serial.print(message.sensor); Serial.print(", Message: "); Serial.println(message.getString());
  }
}
// ## MySensors Support Methods
void Update_Alarm1_From_Controller(unsigned long theMills){
  if (theMills - Alarm_Request_pMillis >= Alarm_Request_sCheck){
    Update_Alarm1_From_Controller();
    Alarm_Request_pMillis = theMills;
  }
}
void Update_Alarm1_From_Controller(){
  request(cID_ALARM_TIME, V_TEXT);
}
// ## Wake-Up Light Methods ##
void Update_LED_Sunrise(unsigned long theMills){
  if ((LED_Sunrise == false) && (theMills - LED_Sunrise_pMillis >= LED_Sunrise_sCheck)){
    LED_Brightness = LED_Brightness + LED_Steps;
    analogWrite(LED_PIN, LED_Brightness);
    if (LED_Brightness >= 255){
      LED_Sunrise = true;
      LED_Flash = true;
      Speaker_Reset_Song();
      Speaker_Playing = true;
    }
    LED_Sunrise_pMillis = theMills;
  }
}
void Update_LED_Flash(unsigned long theMills){
  if ((LED_Flash == true) && (theMills - LED_Flash_pMillis >= LED_Flash_sCheck)){
    if (LED_Flash_State){
      analogWrite(LED_PIN, 255);
      LED_Flash_State = false;
    }else{
      analogWrite(LED_PIN, 0);
      LED_Flash_State = true;
    }
    LED_Flash_pMillis = theMills;
  }
}
void Stop_Alarm(){
  LED_Flash_pMillis = 0;
  LED_Sunrise_pMillis = 0;
  LED_Sunrise = false;
  LED_Flash = false;
  LED_Brightness = 0;
  analogWrite(LED_PIN, 0);

  noTone(Speaker_PIN);
  Speaker_Playing = false;
  send(MySensors_MSG_Alarm_Active.set(0));
}
//## Night Light Methods ##
void Night_Light(){
  int brightness = 0;
  if (LED_Night_Light_On){
    brightness = LED_Night_LightBrightness;
  }

  int brightness_mapped = map(brightness, 0, 100, 0, 255);

  analogWrite(LED_PIN, brightness_mapped);
  send(MySensors_DIM_LED_RING.set(brightness));// Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
  //send(MySensors_SWITCH_LED_RING.set(LED_CurrentLevel) );// hek comment: Is this really nessesary?
}
//## KeyPad Methods ##
void Check_KeyPad(unsigned long theMills){
  if (theMills - Keys_pMillis >= Keys_sCheck){
    char Key1x5_custom = Keys_Keypad1x5.readKey();
    if(Key1x5_custom != KEY_NOT_PRESSED){
      if (Keys_Debug){Serial.print("1x5_c Key: ");Serial.println(Key1x5_custom);}
      if (Key1x5_custom == '1'){
        if ( Keys_ShowAlarmOnScreen == true){
          Keys_ShowAlarmOnScreen = false;
          Screen_sCheck = 1000;
        }else{
          Keys_ShowAlarmOnScreen = true;
          Screen_sCheck = 200;
        }
        Alarm1_Local_Change_pMillis = theMills;
      }else if (Key1x5_custom == '2'){
        if (Alarm1_Time_Local[0] == 24)
          Alarm1_Time_Local[0] = 1;
        else
          Alarm1_Time_Local[0] = Alarm1_Time_Local[0] + 1;
      }else if (Key1x5_custom == '3'){
        if (Alarm1_Time_Local[1] == 59){
          Alarm1_Time_Local[1] = 0;
          if (Alarm1_Time_Local[0] == 24)
            Alarm1_Time_Local[0] = 1;
          else
            Alarm1_Time_Local[0] = Alarm1_Time_Local[0] + 1;
        }else{
          Alarm1_Time_Local[1] = Alarm1_Time_Local[1] + 1;
        }
      }else if ((Key1x5_custom == '4') || (Key1x5_custom == 'c')){
        if(Alarm1_Enabled){
          if(Alarm1_ALARMING) {
            Alarm1_ALARMING = false;
            Stop_Alarm();
          }
          Alarm1_Enabled=false;
        }else{
          Alarm1_Enabled=true;
        }
      }else if (Key1x5_custom == '5'){
        if(!LED_Night_Light_On){
          LED_Night_Light_On=true;
          Night_Light();
        }else{
          LED_Night_Light_On=false;
          Night_Light();
        }
      }
      if ((Key1x5_custom == '2') || (Key1x5_custom == '3')){
        Alarm1_Set_Locally = true;Alarm1_Set_Remotely = false;
        Alarm1_Local_Change_pMillis = 0;Alarm1_Local_Change = true;
        Alarm1_Local_Change_pMillis = theMills;
        Alarm1_Time[0] = Alarm1_Time_Local[0];
        Alarm1_Time[1] = Alarm1_Time_Local[1];
      }
    }
    Keys_pMillis = theMills;
  }
}
//## RTC Methods ##
void Check_RTC_Temp(unsigned long theMills){
  if (theMills - RTC_Temp_pMillis >= RTC_Temp_sCheck){
    RTC_Temp = RTC.temperature()/4;
    send(MySensors_MSG_Temp.set(RTC_Temp, 1), MySensors_RequestACK);
    RTC_Temp_pMillis = theMills;
  }
}
//## Alarm Methods ##
void ChangeAlarm1(){
  if (Alarm1_Set_Remotely){
    Alarm1_Time[0] = Alarm1_Time_Remote[0];
    Alarm1_Time[1] = Alarm1_Time_Remote[1];
  } else if (Alarm1_Set_Locally){
    Alarm1_Time[0] = Alarm1_Time_Local[0];
    Alarm1_Time[1] = Alarm1_Time_Local[1];
  }
  Alarm.free(Alarm1);
  Alarm1 = Alarm.alarmRepeat(Alarm1_Time[0],Alarm1_Time[1],00, MorningAlarm);
  Alarm1_Enabled = true;
  //Serial.print("Alarm Updated: ");Serial.print(Alarm1_Time[0]);Serial.print(":");Serial.println(Alarm1_Time[1]);
}
void MorningAlarm(){
  if (Alarm1_Enabled){
    LED_Night_Light_On=false;
    Night_Light();
    Alarm1_ALARMING = true;
    send(MySensors_MSG_Alarm_Active.set(1));
  }//else
    //Serial.println("alarm was supposed to go off but somehow it got turned off but not wiped");
}
void Check_Local_Alarm_Change(unsigned long theMills){
  if (theMills - Alarm1_Local_Change_pMillis >= Alarm1_Local_Change_sCheck){
    if (Alarm1_Local_Change){
      ChangeAlarm1();
      Alarm1_Local_Change = false;
      char buf[20];    
      sprintf(buf, "%d.%d", Alarm1_Time_Local[0], Alarm1_Time_Local[1]);
      send(MySensors_MSG_Alarm1_Time.set(buf), MySensors_RequestACK);
    }
    Keys_ShowAlarmOnScreen = false;
    Screen_sCheck = 1000;
  }
}
//## Screen Methods ##
void Update_Screen(unsigned long theMills){
  if (theMills - Screen_pMillis >= Screen_sCheck){
    tmElements_t tm;
    RTC.read(tm);
    int displayHour, displayMinute;
    if (Keys_ShowAlarmOnScreen){
      displayHour = Alarm1_Time[0];
      displayMinute = Alarm1_Time[1];
    }else{
      displayHour = tm.Hour;
      displayMinute = tm.Minute;
    }
    
    if ((Alarm1_Enabled) && (!Keys_ShowAlarmOnScreen)){
      if(Screen_ClockPoint)ledScreen.point(POINT_ON);
      else ledScreen.point(POINT_OFF);
    } else
      ledScreen.point(POINT_ON);
    if ((displayHour > 12) && (!Screen_HourFormat24) && (!Keys_ShowAlarmOnScreen)){
      if((displayHour-12) < 10)
        Screen_Display[0] = 0x7f;
      else
        Screen_Display[0] = (displayHour-12) / 10;
      Screen_Display[1] = (displayHour-12) % 10;
    } else{
      if(displayHour <10){
        Screen_Display[0] = 0x7f;
      }else
        Screen_Display[0] = displayHour / 10;
      Screen_Display[1] = displayHour % 10;
    }
    Screen_Display[2] = displayMinute / 10;
    Screen_Display[3] = displayMinute % 10;
    
    if (tm.Hour < 7){//dim
      Screen_Brightness = 0;
    }else if (tm.Hour < 11){//medium
      Screen_Brightness = 2;
    }else if (tm.Hour < 18){//bright
      Screen_Brightness = 7;
    }else if (tm.Hour < 11){//medium
      Screen_Brightness = 2;
    }else if (tm.Hour < 25){//dark
      Screen_Brightness = 0;
    }
    ledScreen.set(Screen_Brightness);
    ledScreen.init();
    
    ledScreen.display(Screen_Display);
    
    Screen_pMillis = theMills;
    if(Screen_ClockPoint){Screen_ClockPoint=0;}else{Screen_ClockPoint=1;}
  }
}
//## Speaker Methods ##
void Check_Speaker(unsigned long theMills){
  if (Speaker_Playing){
    if (theMills - Speaker_pMillis >= Speaker_sCheck){
      noTone(Speaker_PIN);
      Speaker_Current_Note_Duration = 600/Speaker_Notes[Speaker_Current_Note];
      Speaker_Current_Pause = Speaker_Current_Note_Duration * 1.30;
      tone(Speaker_PIN, Speaker_Melody[Speaker_Current_Note],Speaker_Current_Note_Duration);
      Speaker_sCheck = Speaker_Current_Note_Duration;
      Speaker_pMillis = theMills;
      Speaker_Current_Note = Speaker_Current_Note + 1;
      if (Speaker_Current_Note >= Speaker_Note_NumberOfNotes)
        Speaker_Current_Note = 0;
    }
  }
}
void Speaker_Reset_Song(){
  Speaker_Current_Note = 0;
  Speaker_Current_Note_Duration = 0;
  Speaker_Current_Pause = 0;
  Speaker_pMillis = millis();
}



/*
TODO LIST ******************************************************************************
-Make audio alarm delay until wake-up light is flashing | DONE
-Fix manual alarm setting to not blank time | DONE
-Fix buttons not always working (frequency or debounce issue?) | DONE
-Full communication with controller, turn alarm on and off | DONE
-Fix light status between button and controller | DONE
-
****************************************************************************************
*/
