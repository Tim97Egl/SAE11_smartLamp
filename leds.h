#define LED_CH0_PIN     12
#define LED_CH1_PIN     13
#define LED_FREQUENCY   1000
#define LED_RESOLUTION  8

const float stepW = 256.0f/340.0f;
const float stepC = 256.0f/400.0f;
int intenseW;
int intenseC;

void led_setup() {
  ledcSetup(0, LED_FREQUENCY, LED_RESOLUTION);
  ledcSetup(1, LED_FREQUENCY, LED_RESOLUTION);

  ledcAttachPin(LED_CH0_PIN, 0);
  ledcAttachPin(LED_CH1_PIN, 1);
}

void set_ch0_dutycycle(int dutycycle) {
  ledcWrite(0, dutycycle);
}

void set_ch1_dutycycle(int dutycycle) {
  ledcWrite(1, dutycycle);
}

void IntensityW(int intense){
  set_ch0_dutycycle(256-stepW*intense);
  //Serial.println(256-stepW*intense);
  
  }

void IntensityC(int intense){
  set_ch1_dutycycle(256-stepC*intense);
  //Serial.println(256-stepC*intense);
  }

int mapIntense(float input){
  int i = input*100;
  return map(i,0 ,100 ,0 ,600);
}


void setLEDs(float intensef, float temp){
  int intense = mapIntense(intensef);
  if((intense*temp) <= 340 && (intense*(1-temp)) <= 340 ){
  intenseW = intense*temp;
  intenseC = intense*(1-temp);
  }
  else{
  if((intense*temp) >= 340){
    intenseW = 340;
    intenseC = intenseW*((1-temp)/temp);
  }else if((intense*(1-temp)) >= 340){
       intenseC = 340;
       intenseW = intenseC*(temp/(1-temp));
    }
  }
  IntensityW(intenseW);
  IntensityC(intenseC);
}