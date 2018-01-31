// Enable debug prints
// #define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>
#include <DHT.h>

unsigned long SLEEP_TIME = 120000; // Sleep time between reports (in milliseconds)

#define CHILD_ID_LIGHT 0
#define LIGHT_SENSOR_ANALOG_PIN 0

#define CHILD_ID_PIR 1   // Id of the sensor child
#define PIR_DIGITAL_INPUT_SENSOR_PIN 3    // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)

static const uint8_t FORCE_UPDATE_N_READS = 10;

#define CHILD_ID_HUM 3
#define CHILD_ID_TEMP 4
#define DHT_DATA_PIN 4 // humidi

#define SENSOR_TEMP_OFFSET 0

static const uint64_t UPDATE_INTERVAL = 60000;

float lastTemp;
float lastHum;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
bool metric = true;

#define CHILD_ID_DOOR 5
#define DOOR_PIN 5

Bounce debouncer = Bounce(); 
int oldValue=-1;

MyMessage msg(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
int lastLightLevel;


MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
DHT dht;

// Initialize motion message
MyMessage msgPir(CHILD_ID_PIR, V_TRIPPED);

// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msgDoor(CHILD_ID_DOOR,V_TRIPPED);

void presentation()  
{ 

// Send the sketch version information to the gateway and Controller
  sendSketchInfo("Bathroom Sensor", "1.0");


  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_PIR, S_MOTION);
  
  // Register all sensors to gateway (they will be created as child devices)
  present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_DOOR, S_DOOR);
  
  metric = getControllerConfig().isMetric;
  
}


void setup()
{
	pinMode(PIR_DIGITAL_INPUT_SENSOR_PIN, INPUT);      // sets the motion sensor digital pin as input

 dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
  if (UPDATE_INTERVAL <= dht.getMinimumSamplingPeriod()) {
    Serial.println("Warning: UPDATE_INTERVAL is smaller than supported by the sensor!");
  }
  // Sleep for the time of the minimum sampling period to give the sensor time to power up
  // (otherwise, timeout errors might occure for the first reading)
  sleep(dht.getMinimumSamplingPeriod());

  // Setup the button
  pinMode(DOOR_PIN,INPUT);
  // Activate internal pull-up
  digitalWrite(DOOR_PIN,HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(DOOR_PIN);
  debouncer.interval(5);

}



void loop()
{

  int16_t lightLevel = (1023-analogRead(LIGHT_SENSOR_ANALOG_PIN))/10.23;
  Serial.println(lightLevel);
  if (lightLevel != lastLightLevel) {
    send(msg.set(lightLevel));
    lastLightLevel = lightLevel;
  }
  
	// Read digital motion value
	bool tripped = digitalRead(PIR_DIGITAL_INPUT_SENSOR_PIN) == HIGH;

	Serial.println(tripped);
	send(msg.set(tripped?"1":"0"));  // Send tripped value to gw

  // Force reading sensor, so it works also after sleep()
  dht.readSensor(true);
  
  // Get temperature from DHT library
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    // Reset no updates counter
    nNoUpdatesTemp = 0;
    temperature += SENSOR_TEMP_OFFSET;
    send(msgTemp.set(temperature, 1));

    #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }

  // Get humidity from DHT library
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } 
  else if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS)
  {
    // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
    lastHum = humidity;
    // Reset no updates counter
    nNoUpdatesHum = 0;
    send(msgHum.set(humidity, 1));
    
    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesHum++;
  }

   debouncer.update();
  // Get the update value
  int value = debouncer.read();

  if (value != oldValue) {
     // Send in the new value
     send(msg.set(value==HIGH ? 1 : 0));
     oldValue = value;
  }

	// Sleep until interrupt comes in on motion sensor. Send update every two minute.
	sleep(digitalPinToInterrupt(PIR_DIGITAL_INPUT_SENSOR_PIN), CHANGE, SLEEP_TIME);
}

