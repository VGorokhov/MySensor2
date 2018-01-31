/*
  This is a gateway node between mysensors and 433Mhz devices. A 433Mhz receiver is connected to pin 2 (int 0).
  This works with any 433Mhz device (as long as rc-switch understands the protocol).

  If you only want to add your own codes simply paste them into the switches array. If you want to use a device that should not be treated as a door/window/button/binary
  you have to change the present() and send() calls.

  This is based on the MySensors network: https://www.mysensors.org
  This uses the rc-switch library to detect button presses: https://github.com/sui77/rc-switch

  Written by Oliver Hilsky 07.03.2017
*/

#define MY_RADIO_NRF24
#define MY_BAUD_RATE 9600
#define MY_DEBUG    // Enables debug messages in the serial log
//#define MY_NODE_ID 10

#include <EEPROM.h>
#include <SPI.h>
#include <MyConfig.h>
#include <MySensors.h>
#include <Vcc.h>
#include <RCSwitch.h>

#define RECEIVER_PIN 0

RCSwitch wirelessSwitch = RCSwitch();

// add the decimal codes for your switches here
unsigned long switches[] = {12587348, 12587346, 12587345};
int numberOfSwitches = 3;

void presentation()
{
  sendSketchInfo("433 Wireless Gateway", "07032017");

  for (int i = 0; i < numberOfSwitches; i++) {
  	present(i, S_DOOR); // s_door has tripped, s_binary might be a choice too 
  }
}

void setup() {
  wirelessSwitch.enableReceive(RECEIVER_PIN);  // Receiver on inerrupt 0 => that is pin #2
  Serial.println("Started listening on pin 2 for incoming 433Mhz messages");
}

void loop() {
  if (wirelessSwitch.available()) {
  	// for testing
    //output(wirelessSwitch.getReceivedValue(), wirelessSwitch.getReceivedBitlength(), wirelessSwitch.getReceivedDelay(), wirelessSwitch.getReceivedRawdata(),wirelessSwitch.getReceivedProtocol());

    unsigned long received_val = wirelessSwitch.getReceivedValue();

    Serial.print("Pressed button ");
    Serial.print(received_val);

    for (int i = 0; i < numberOfSwitches; i++) {
  	  if (received_val == switches[i]) {
  	  	Serial.print(". Recognized! Its button ");
  	  	Serial.println(i);
  	  	send(MyMessage(i, V_TRIPPED).set(true));
  	  }
  	}

    // software debounce
    wait(500);
    wirelessSwitch.resetAvailable();
  }
}
