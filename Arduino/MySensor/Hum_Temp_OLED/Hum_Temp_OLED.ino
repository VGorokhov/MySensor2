/**
 *******************************
 *
 * REVISION HISTORY
 * Version 2.0 - VGorokhov
 * 
 * DESCRIPTION
 * ****************************
 *       OLED         ARDUINO
 *  -----1234-----
 *  |            |  1 ->    3,3 V
 *  |            |  2 ->    GND
 *  |            |  3 -> A5 SCL
 *  |            |  4 -> A4 SDA
 *  --------------
 *  
 *  OLED ASCII Library from https://github.com/greiman/SSD1306Ascii
 *  
 */
 
// Enable debug prints to serial monitor
#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <SPI.h>
#include <MySensors.h> 
#include <HTU21D.h> 
#include <Wire.h>         // I2C

#define TEMP_CHILD_ID 5
#define HUM_CHILD_ID 6
#define BATT_CHILD_ID 10

#define batteryVoltage_PIN  A0    //analog input A0 on ATmega328 is battery voltage ( /2)
#define LTC4067_CHRG_PIN  A1    //analog input A1 on ATmega 328 is /CHRG signal from LTC4067

//oled display 
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

const float VccMin        = 1.0*3.5;  // Minimum expected Vcc level, in Volts. Example for 1 rechargeable lithium-ion.
const float VccMax        = 1.0*4.2;  // Maximum expected Vcc level, in Volts. 

HTU21D SHT21;             // Hum/Temp (SHT12)

float lastTempSHT;          // SHT temp/hum
float lastHumSHT;
float lastBattVoltage;
int lastBattPct = 0;
float VccReference = 3.3;

boolean charge = false;
boolean timeReceived = false;
unsigned long lastUpdate=0, lastRequest=0; 

unsigned long SLEEP_TIME = 60000;   //  60 sec sleep time between reads (seconds * 1000 milliseconds)

// SketchInfo
static const char SketchName[]    ="Temp + hum + display";
static const char SketchVersion[] ="2.0";
int PosX     = 0;
int PosY     = 0;
int ClrScr   = 0;
int FontSize = 1;

// loop
static const uint64_t UPDATE_INTERVAL = 60000;

MyMessage msgTemp(TEMP_CHILD_ID, V_TEMP);      // SHT temp (deg C)
MyMessage msgHum(HUM_CHILD_ID, V_HUM);         // SHT hum (% rh)
MyMessage msgPatteryVoltage(BATT_CHILD_ID, V_VOLTAGE);

void setup()  
{  
   
  analogReference(DEFAULT);             // default external reference = 3.3v for Ceech board
  VccReference = 3.323 ;                // measured Vcc input (on board LDO)

  Wire.begin();                   // init I2C
  SHT21.begin();                    // initialize temp/hum
  
  Serial.println("starting...");
  ShowInfo();
}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SketchName, SketchVersion);
  present(TEMP_CHILD_ID, S_TEMP);     // SHT temp
  present(HUM_CHILD_ID, S_HUM);     // SHT humidity
  present(BATT_CHILD_ID, S_POWER);    // Battery parameters
}

void receive(const MyMessage &mymessage)
{
 if (mymessage.type==V_TEMP) {
   DisplayMessage(mymessage.getString());
  }
 if (mymessage.type==V_HUM) {
   PosX = atoi(mymessage.getString());
  } 
 if (mymessage.type==V_VOLTAGE) {
    PosY = atoi(mymessage.getString());
  } 
 
}

void loop()     
{     
 
  // Wait, but receive messages
  wait(UPDATE_INTERVAL); 
}


void ShowInfo()
{
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Arial14);  
  oled.set1X();
  oled.setCursor(20,1);
  oled.write("Temp ");
  oled.setFont(CalBlk36); 
  oled.write(lastTempSHT);
  oled.write("°");
  oled.setFont(Arial14);
  oled.setCursor(30,2);
  oled.write("Hum ");
  oled.setFont(CalBlk36);
  oled.write(lastHumSHT);
  oled.write("%");
  oled.setFont(Arial14);
  oled.setCursor(30,5);
  oled.write("Batt:");
  oled.setFont(CalBlk36);
  oled.write(lastBattVoltage);
  oled.write("V");
  sleep(3000); 
  oled.clear();
}

void DisplayMessage(const char Message[])
{
  oled.setCursor(PosX,PosY);
  oled.clearToEOL ();
  oled.write(Message);
}

void sendTempHum(void)
{
  // SHT2x sensor
  // Temperature and Humidity
  float humidity = SHT21.readHumidity();
  // send to MySensor network / only if change
  if (humidity != lastHumSHT && humidity != 0.00) {
    lastHumSHT = humidity;
    send(msgHum.set(humidity, 2));  // Send
  }
   
  float temperatureSHT = SHT21.readTemperature();
  // send to MySensor network / only if change
  if (temperatureSHT != lastTempSHT && temperatureSHT != 0.00) {
    lastTempSHT = temperatureSHT;
    send(msgTemp.set(temperatureSHT, 2));  // Send
  }

  Serial.print("SHT21 temp: ");
  Serial.print(temperatureSHT);
  Serial.print(" SHT21 hum: ");
  Serial.print(humidity);
}

void sendVoltage(void)
// battery values
{
  // get Battery Voltage
  float batteryVoltage = ((float)analogRead(batteryVoltage_PIN)* VccReference/1024) * 2;  // actual voltage is double
  Serial.print("Batt: ");
  Serial.print(batteryVoltage);
  Serial.print("V ; ");
  
  // send battery percentage for node
  int battPct = 1 ;
  if (batteryVoltage > VccMin){
    battPct = 100.0*(batteryVoltage - VccMin)/(VccMax - VccMin);
  }

  charge = digitalRead(LTC4067_CHRG_PIN);
  
  send(msgPatteryVoltage.set(batteryVoltage, 3));     // Send (V)
  sendBatteryLevel(battPct);
  
  Serial.print("BattPct: ");
  Serial.print(battPct);
  Serial.println("% ");
}

