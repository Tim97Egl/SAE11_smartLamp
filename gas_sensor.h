#include <Wire.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    sgp.setHumidity(absoluteHumidityScaled);
}


void setup_gas_sensor() {
  if (!sgp.begin()) {
    //Serial.println("SPG30: sensor not found :(");
  }
}

bool gas_sensor_getAirQuality() {
  if (!sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
 //   return;
  }
  float eco2 = sgp.eCO2;
  
  //Error when values of tvoc and eco2 become bigger than normal conditions
  if (eco2 > 1000) {
   //Serial.println("CO2 too much");
   return true;
  }
  
  else {
    return false;
  }
}