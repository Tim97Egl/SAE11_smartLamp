#include <SparkFun_GridEYE_Arduino_Library.h>
#include <Wire.h>

//Setup
#define UPPER_LIMIT 25
#define LOWER_LIMIT 0
#define HYSTERESIS 1

GridEYE grideye;

void ir_setup(){
	grideye.begin();
	grideye.setInterruptModeAbsolute();
	grideye.setUpperInterruptValue(UPPER_LIMIT);
	grideye.setLowerInterruptValue(LOWER_LIMIT);
	grideye.setInterruptHysteresis(HYSTERESIS);
}


bool isPerson(){
	int z =0;
	for(int i = 0; i < 64; i++){
	  if(grideye.pixelInterruptSet(i) == true) z++;    
	}
	return (z>30);
}