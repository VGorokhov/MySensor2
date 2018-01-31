/*
 *  Based on Author: Patrick 'Anticimex' Fallberg Interrupt driven binary switch example with dual interrupts
 *  Code by 'ServiceXP'
 *  Modified by 'PeteWill to add CO detection
 *  For more info on how to use this code check out this video: https://youtu.be/mNsar_e8IsI
 *  
 */
// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>
#include <SPI.h>

//#define SKETCH_NAME "Smoke Alarm Sensor"
//#define SKETCH_MAJOR_VER "1"
//#define SKETCH_MINOR_VER "1"
#define NODE_ID 11 //or AUTO to let controller assign

#define SMOKE_CHILD_ID 0
#define CO_CHILD_ID 1

#define SIREN_SENSE_PIN 3   // Arduino Digital I/O pin for optocoupler for siren
#define DWELL_TIME 125  // this allows for radio to come back to power after a transmission, ideally 0 

unsigned int SLEEP_TIME         = 32400; // Sleep time between reads (in seconds) 32400 = 9hrs
byte CYCLE_COUNTER                  = 2;  // This is the number of times we want the Counter to reach before triggering a CO signal to controller (Kidde should be 2).
byte CYCLE_INTERVAL        = 2; // How long do we want to watch for smoke or CO (in seconds). Kidde should be 2

byte oldValue;
byte coValue;
byte smokeValue;

//MySensor sensor_node;
MyMessage msgSmoke(SMOKE_CHILD_ID, V_TRIPPED);
MyMessage msgCO(CO_CHILD_ID, V_TRIPPED);

void setup()
{
 // begin(NULL, NODE_ID);
  // Setup the Siren Pin HIGH
  pinMode(SIREN_SENSE_PIN, INPUT_PULLUP);
  // Send the sketch version information to the gateway and Controller
  //sensor_node.sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER"."SKETCH_MINOR_VER);
  //sensor_node.present(SMOKE_CHILD_ID, S_SMOKE);
  //sensor_node.present(CO_CHILD_ID, S_SMOKE);
}

// Loop will iterate on changes on the BUTTON_PINs
void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Smoke Alarm Sensor", "1.0");

    // Register all sensors to gateway (they will be created as child devices)
    present(SMOKE_CHILD_ID, S_SMOKE);
    present(CO_CHILD_ID, S_SMOKE);
}
void loop()
{
  // Check to see if we have a alarm. I always want to check even if we are coming out of sleep for heartbeat.
  AlarmStatus();
  // Sleep until we get a audio power hit on the optocoupler or 9hrs
  sleep(SIREN_SENSE_PIN - 2, CHANGE, SLEEP_TIME * 1000UL);
}

void AlarmStatus()
{

  // We will check the status now, this could be called by an interrupt or heartbeat
  byte value;
  byte wireValue;
  byte previousWireValue;
  byte siren_low_count   = 0;
  byte siren_high_count   = 0;
  unsigned long startedAt = millis();

  Serial.println("Status Check");
  //Read the Pin
  value = digitalRead(SIREN_SENSE_PIN);
  // If Pin returns a 0 (LOW), then we have a Alarm Condition
  if (value == 0)
  {
    //We are going to check signal wire for CYCLE_INTERVAL time
    //This will be used to determine if there is smoke or carbon monoxide
    while (millis() - startedAt < CYCLE_INTERVAL * 1000)
    {
      wireValue = digitalRead(SIREN_SENSE_PIN);
      if (wireValue != previousWireValue)
      {
        if (wireValue == 0)
        {
          siren_low_count++;
          Serial.print("siren_low_count: ");
          Serial.println(siren_low_count);
        }
        else
        {
          siren_high_count++;
          Serial.print("siren_high_count: ");
          Serial.println(siren_high_count);
        }
        previousWireValue = wireValue;
      }
    }
    // Eval siren hit count against our limit. If we are => then CYCLE_COUNTER then there is Carbon Monoxide
    if (siren_high_count >= CYCLE_COUNTER)
    {
      Serial.println("CO Detected");
      //Check to make sure we haven't already sent an update to controller
      if (coValue == 0 )
      {
        //update gateway CO is detected.
        send(msgCO.set("1"));
        Serial.println("CO Detected sent to gateway");
        coValue = 1;
      }
    }
    else
    {
      Serial.println("Smoke Detected");
      //Check to make sure we haven't already sent an update to controller
      if (smokeValue == 0 )
      {
        //update gateway smoke is detected.
        send(msgSmoke.set("1"));
        Serial.println("Smoke Detected sent to gateway");
        smokeValue = 1;
      }
    }
    oldValue = value;
    AlarmStatus(); //run AlarmStatus() until there is no longer an alarm
  }
  //Pin returned a 1 (High) so there is no alarm.
  else
  {
    //If value has changed send update to gateway.
    if (oldValue != value)
    {
      //Send all clear msg to controller
        send(msgSmoke.set("0"));
        wait(DWELL_TIME); //allow the radio to regain power before transmitting again
        send(msgCO.set("0"));
      oldValue = value;
      smokeValue = 0;
      coValue = 0;
      Serial.println("No Alarm");
    }
  }
}

