#include "AS726X.h"
AS726X sensor;//Creates the sensor object

void farb_sensor_setup() {
  // put your setup code here, to run once:
  sensor.begin();

}
/**
 * gibt die Farbtemperatur als Wert zwischen 0 und 1 zurück. 0 entspricht kaltem Licht und 1 warmem Licht. 
 */

float getTemp(){
  float verhaeltnis;
  //long timeStart = millis();
  sensor.takeMeasurements();
  //Serial.print("Time to measure: "); Serial.println(millis() - timeStart);
  verhaeltnis = (sensor.getCalibratedRed() + sensor.getCalibratedOrange())/(sensor.getCalibratedBlue() + sensor.getCalibratedViolet()+ sensor.getCalibratedOrange() + sensor.getCalibratedRed());
  float gemappt = (float)map(verhaeltnis * 1000, 200, 700, 0, 1000)/ (float) 1000; 

  if (gemappt < 0){gemappt = 0;}
  else if (gemappt >1){gemappt = 1;}
  return gemappt; 
 }
