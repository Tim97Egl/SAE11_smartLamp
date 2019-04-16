#include <Wire.h>
#include "Adafruit_TSL2561_U.h"

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 39);

//LALALLALALALLA
float luxes = 1;
#define LUX_MAX 2000

//lux_sensor_getIntensity();

void setup_lux_sensor() {
  Serial.begin(9600);
  if(!tsl.begin()) {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("No TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */

  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
}

float lux_sensor_getIntensity() {
  sensors_event_t event;
  tsl.getEvent(&event);
  if(event.light) {
    luxes = event.light;
  }
  else {
    luxes = 0;
  }
  
  if(luxes <= 0) {
    return 0;
  }
  
  if(luxes >= LUX_MAX) {
    return 1;
  }
  else{
    return luxes/LUX_MAX;
  }
}