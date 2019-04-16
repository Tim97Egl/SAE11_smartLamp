#include <farb_sensor.h>
#include <gas_sensor.h>
#include <IR_sensor.h>
#include <lux_sensor.h>
#include <RGBW.h>
#include <leds.h>
#include <WiFi.h>
#include <WebServer.h>

//GUI
#define WIFI_ENABLED true
const char* ssid = "AndroidAP";  // Enter SSID here
const char* password = "obun5560";  //Enter Password here
WebServer server(80);

#define SENSOR_ENABLED true
//Cap-Sensors
#define CAP_PLUS_PIN 27
#define CAP_MINUS_PIN 4
#define CAP_AUTO_PIN 15
#define RISE_SPEED 0.4             //in 1/s
#define CAP_SENSOR_THRESHOLD 40
#define AUTO_PUFFER 200 
bool plusPressed = false;
long plusCounter = 0;
bool minusPressed = false;
long minusCounter = 0;
bool autoPressed = false;
bool autoLastFrame = false;
long autoCounter = 0;

//RGBW-Strip - PIN 14
#define ALARM_INTERVAL 1000
String alarmColor = "red";
bool gasAlarmActive = false;
String alarmColorTimer = "tuerkis";


//Helligkeit
//#define LUX_MAX 2000            //Später wieder raus
#define AVERAGE_LUX 700
#define HYSTERESIS_LUX 300
#define LUX_PER_PERCENT 545             //Aus Messwerten        
#define LUX_PER_PERCENT_ON_SENSOR 21    //Aus Messwerten
#define LED_AUTO_PIN 2
float manualBrightness = 0.5;
float autoBrightness = 0.5;
float getAutoBrightnessLastFrame = 0.5;
bool autoBrightnessEnabled = autoLastFrame;

//Temperatur
float manualTemp = 0.5;
float autoTemp = 0.5;
bool autoTempEnabled = true;

//Sensor-Werte
float colorTemp = 0;
float lightIntens = 0;
bool seesPerson = true;
bool airQuality = false;                        //Wenn Luftqualität schlecht: airQuality = true

//Gas-Sensor
#define ALARM_DELAY_GAS 120000                   //Zeit zwischen zwei Warnungen
#define ALARM_DURATION_GAS 5000                 //Länge der Warnung 
boolean gasSensorAlarmEnabled = true;
boolean gasSensorTriggered = false;              //Alarm-Modus
long timeSinceLastAlarmGas = ALARM_DELAY_GAS+1;  //Zeit seit des letzten Alarm
long alarmDurationGas = 0;                       //Dauer des aktuellen Alarms


//IR-Sensor
#define SWITCH_OFF_TIME 1000                  //In millis; 1 min = 60000
#define IR_RESET_PUFFER 1000                    //In millis; Zeit bis sich Lampe wieder anschaltet
boolean autoSwitchOffEnabled = true;            //Funktion aktiviert?
boolean personAway = false;                     //Ist die Person abwesend?
long personAwaySince = 0;
long resetCounter = 0;



//Timer
#define TIME_BETWEEN_TEMP_MEASUREMENTS 10000    //In millis
long frameTime = 0;
long lastFrame = 0;
long currentFrame = 0;
long timeSinceLastTempMeasurement = TIME_BETWEEN_TEMP_MEASUREMENTS+1;

void setup() {
  Serial.begin(115200);
  Serial.println("Serial initialized");
  delay(100);
  
  if(SENSOR_ENABLED) {
  Wire.begin();
  Serial.println("Wire begin");
  
  //Sensoren
  Serial.println("Lux sensor setup:");
  setup_lux_sensor();
  Serial.println("IR sensor setup:");
  ir_setup();
  Serial.println("Gas sensor setup:");
  setup_gas_sensor();
  Serial.println("Color sensor setup:");
  farb_sensor_setup();
  
  //LED
  Serial.println("LED setup:");
  led_setup();
  handleLED();

  //RGBW + Auto-LED
  Serial.println("RGB Strip setup:");  
  RGBWsetup();
  pinMode(LED_AUTO_PIN, OUTPUT);
  if (autoBrightnessEnabled){
    digitalWrite(LED_AUTO_PIN, HIGH);  
  }else{
    digitalWrite(LED_AUTO_PIN, LOW);
  }
  }
  
  //WiFi Server
  if (WIFI_ENABLED){
    setupWifi();
  }  
}

void loop() {
  //Timer
  currentFrame = millis();
  frameTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  
  //Cap-Sensors
  handleManualSensors(frameTime);
  handleAutoSensor(frameTime);
  
  if(SENSOR_ENABLED){
  //Sensor-Daten
  readSensors(frameTime);
  
  //Gas-Sensor
  handleAirQuality(frameTime);

  //IR-Sensor
  if (autoSwitchOffEnabled == false){
    personAway = false;
  }else{
    handleAutoSwitchOff(frameTime);
  }

  //RGBW-Strip
  if (personAway == true && autoSwitchOffEnabled == true){
    RGBWaus();
  }else if (gasAlarmActive){
    alarm(alarmColor);
  }else{
    passiveRGB();
  }
  
  //Auto-LED
  if (autoBrightnessEnabled && (personAway == false || autoSwitchOffEnabled == false)){
    digitalWrite(LED_AUTO_PIN, HIGH);  
  }else{
    digitalWrite(LED_AUTO_PIN, LOW);
  }

    //LED
  if (personAway == true && autoSwitchOffEnabled == true){
    setLEDs(0, 0);
  }else{
    handleLED();
  }
  }
  
  //GUI
  if (WIFI_ENABLED){
    server.handleClient();
  }


  
  //Test
  Serial.println("Helligkeitsstufen:");
  Serial.print("Auto: ");
  Serial.print(autoBrightness);
  Serial.print("       Manual: ");
  Serial.print(manualBrightness); 
  Serial.print("      AutoBrightness: ");
  Serial.println(autoBrightnessEnabled);
  Serial.print("Auto: ");
  Serial.print(autoTemp);
  Serial.print("       Manual: ");
  Serial.print(manualTemp); 
  Serial.print("      AutoTemp: ");
  Serial.println(autoTempEnabled);
  Serial.println("\n");
  
  Serial.println("Messwerte:");
  Serial.print("Temp: "); Serial.println(colorTemp);
  Serial.print("Intensity: "); Serial.println(lightIntens);
  Serial.print("Air Quality: "); Serial.println(airQuality);
  Serial.print("Sees person: "); Serial.println(seesPerson);
  
  Serial.print("Frame: "); Serial.print(frameTime); Serial.println(" ms");
  //delay(10);
}

void setupWifi(){
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/intens_auto_on", handle_intens_auto_on);
  server.on("/intens_auto_off", handle_intens_auto_off);
  server.on("/temp_auto_on", handle_temp_auto_on);
  server.on("/temp_auto_off", handle_temp_auto_off);
  server.on("/setting", handle_setting);
  server.on("/reading", handle_reading);
  server.on("/studying", handle_studying);
  server.on("/relax", handle_relax);
  server.on("/night", handle_night);
  server.on("/setting/ir", handle_ir);
  server.on("/setting/gas", handle_gas);
  server.on("/intens_plus", handle_intens_plus);
  server.on("/intens_minus", handle_intens_minus);
  server.on("/temp_plus", handle_temp_plus);
  server.on("/temp_minus", handle_temp_minus);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void handleAutoSwitchOff(long frameTime){
  if (autoSwitchOffEnabled == false) return;
  
  if (seesPerson){
    //Person anwesend
    resetCounter = resetCounter + frameTime;
  }else{
    //Niemand da
    personAwaySince = personAwaySince + frameTime;
    resetCounter = 0;
  }

  if (resetCounter > IR_RESET_PUFFER){
    personAway = false;
    personAwaySince = 0;
  }
  if (personAwaySince > SWITCH_OFF_TIME){
    personAway = true;
  }
}

void handleLED(){
  float brightness = 0.5;                                             //0.5 = Dummy Werte
  float temp = 0.5;
  
  if (autoBrightnessEnabled == false) brightness = manualBrightness;  //Helligkeit Manuell
  else brightness = getAutoBrightness();                              //Helligkeit automatisch

  if (autoTempEnabled == false) temp = manualTemp;                    //Farbtemperatur manuell
  else temp = getAutoTemp();                                          //Farbtemperatur automatisch

  if (brightness > 1) brightness = 1;
  if (brightness < 0) brightness = 0;
  if (temp > 1) temp = 1;
  if (temp < 0) temp = 0;
  
  setLEDs(brightness, temp);  
}

float getAutoBrightness(){
  //Für konstante Helligkeit sorgen
  float desiredLux = AVERAGE_LUX + 2*(autoBrightness-0.5)*HYSTERESIS_LUX;
  float sensorLux = lightIntens*LUX_MAX - getAutoBrightnessLastFrame*LUX_PER_PERCENT_ON_SENSOR; //Feedback von Lambe an Sensor berücksichtigen
  float luxNeededFromLamp = desiredLux - sensorLux;  
  getAutoBrightnessLastFrame = (luxNeededFromLamp) / ((float)LUX_PER_PERCENT);
  return getAutoBrightnessLastFrame;
}

float getAutoTemp(){
  //Der Umgebung anpassen
  return colorTemp;
}

void passiveRGB(){
  if (autoTempEnabled && autoBrightnessEnabled){
    RGBWtemp(autoTemp, getAutoBrightness());
  }else if(autoTempEnabled && !autoBrightnessEnabled){
    RGBWtemp(autoTemp, manualBrightness);
  }else if (!autoTempEnabled && autoBrightnessEnabled){
    RGBWtemp(manualTemp, getAutoBrightness());
  }else{
    RGBWtemp(manualTemp, manualBrightness);
  }
}

void readSensors(long frameTime){
  //Gassensor
  //Serial.println("Reading Gas sensor");
  airQuality = gas_sensor_getAirQuality();

  //IR-Sensor
  //Serial.println("Reading IR sensor");
  seesPerson = isPerson();

  //Lichtintensität
  //Serial.println("Reading lux sensor");
  lightIntens = lux_sensor_getIntensity();

  //Wellenlänge
  timeSinceLastTempMeasurement = timeSinceLastTempMeasurement + frameTime;
  if (timeSinceLastTempMeasurement > TIME_BETWEEN_TEMP_MEASUREMENTS){
    Serial.println("Reading color spectrum");
    colorTemp = getTemp();
    timeSinceLastTempMeasurement = timeSinceLastTempMeasurement - TIME_BETWEEN_TEMP_MEASUREMENTS;
  }
}

void alarm(String color){ 
  long alarmTime = millis() % (2*ALARM_INTERVAL);
  if (alarmTime > ALARM_INTERVAL){
    RGBWaus();
  } else {
    RGBWpassiv(1, color);
  }
}

//GUI
String SendHTML_Main(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>smart lamp</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; background-color: #FAFAFA;}\n";
  ptr +="body{margin-top: 50px; margin-left: auto; margin-right: auto} h1 {color: Black;margin: 50px auto 30px;}";

  ptr+= ".buttons{display: inline-block;border: none;width: 70px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttons-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttons-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttons-off {background-color: #585858; color: white;}";
  ptr +=".buttons-off:active {background-color: #6E6E6E; color: white;}";
  ptr +=".buttonm{display: inline-block;border: none;width: 120px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttonm-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttonm-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttonm-off {background-color: #585858; color: white;}";
  ptr +=".buttonm-off:active {background-color: #6E6E6E; color: white;}";
  ptr +=".buttonl{display: inline-block;border: none;width: 140px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttonl-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttonl-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttonl-off {background-color: #585858; color: white;}";
  ptr +=".buttonl-off:active {background-color: #6E6E6E; color: white;}";
  ptr +=".buttonxl{display: inline-block;border: none;width: 180px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttonxl-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttonxl-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttonxl-off {background-color: #585858; color: white;}";
  ptr +=".buttonxl-off:active {background-color: #6E6E6E; color: white;}";
  
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  
  ptr += "<div>";
  {ptr +="<a class=\"buttonm buttonl-off\" href=\"/studying\">Arbeit</a>\n";}
  {ptr +="<a class=\"buttonm buttonl-off\" href=\"/reading\">Lesen</a>\n";}
  ptr += "<\div>";

  ptr += "<div>";
  {ptr +="<a class=\"buttonm buttonl-off\" href=\"/relax\">Entspannen</a>\n";}
  {ptr +="<a class=\"buttonm buttonl-off\" href=\"/night\">Nacht</a>\n";}
  ptr += "<\div>";
  
  //settings_button / eine breite Zeile
  ptr += "<div>";
  ptr +="<a class=\"buttonxl buttonxl-off\" href=\"/setting\">Einstellungen</a>\n";
  ptr += "<\div>";
  ptr += "<div>";
  ptr +="<a class=\"buttonm buttonm-off\" href=\"/intens_minus\">dunkler</a>\n";
  ptr +="<a class=\"buttonm buttonm-off\" href=\"/intens_plus\">heller</a>\n";

    if(autoBrightnessEnabled)
  {ptr +="<a class=\"buttons buttons-on\" href=\"/intens_auto_off\">ON</a>\n";}
  else
  {ptr +="<a class=\"buttons buttons-off\" href=\"/intens_auto_on\">OFF</a>\n";}
  ptr += "<\div>";
  ptr += "<div>";
  ptr +="<a class=\"buttonm buttonm-off\" href=\"/temp_minus\">kaelter</a>\n";
  ptr +="<a class=\"buttonm buttonm-off\" href=\"/temp_plus\">waermer</a>\n";

  if(autoTempEnabled)
  {ptr +="<a class=\"buttons buttons-on\" href=\"/temp_auto_off\">ON</a>\n";}
  else
  {ptr +="<a class=\"buttons buttons-off\" href=\"/temp_auto_on\">OFF</a>\n";}
  ptr += "<\div>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr; 
}

String SendHTML_Setting(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>smart lamp</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; background-color: #FAFAFA;}\n";
  ptr +="body{margin-top: 50px; margin-left: auto; margin-right: auto} h1 {color: Black;margin: 50px auto 30px;}";

  ptr+= ".buttons{display: inline-block;border: none;width: 70px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttons-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttons-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttons-off {background-color: #585858; color: white;}";
  ptr +=".buttons-off:active {background-color: #6E6E6E; color: white;}";
  ptr +=".buttonm{display: inline-block;border: none;width: 120px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttonm-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttonm-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttonm-off {background-color: #585858; color: white;}";
  ptr +=".buttonm-off:active {background-color: #6E6E6E; color: white;}";
  ptr +=".buttonl{display: inline-block;border: none;width: 140px; height: 30px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttonl-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttonl-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttonl-off {background-color: #585858; color: white;}";
  ptr +=".buttonl-off:active {background-color: #6E6E6E; color: white;}";
  ptr +=".buttonxl{display: inline-block;border: none;width: 260px; height: 50px;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}";
  ptr +=".buttonxl-on {background-color: #F2F2F2;color: black;}";
  ptr +=".buttonxl-on:active {background-color: #D8D8D8; color: black;}";
  ptr +=".buttonxl-off {background-color: #585858; color: white;}";
  ptr +=".buttonxl-off:active {background-color: #6E6E6E; color: white;}";
  
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";

    if(gasSensorAlarmEnabled)
  {ptr +="<a class=\"buttonxl buttonxl-on\" href=\"/setting/gas\">Luftqulitaetswarnung an</a>\n";}
  else
  {ptr +="<a class=\"buttonxl buttonxl-off\" href=\"/setting/gas\">Luftqualtiaetsarnung aus</a>\n";}
  ptr += "<\div>";
  ptr += "<div>";


  if(autoSwitchOffEnabled)
  {ptr +="<a class=\"buttonxl buttonxl-on\" href=\"/setting/ir\">Abwesenheitserkennung an</a>\n";}
  else
  {ptr +="<a class=\"buttonxl buttonxl-off\" href=\"/setting/ir\">Abwesenheitserkennung aus</a>\n";}
  ptr += "<\div>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr; 
}
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_intens_auto_on() {
  autoBrightnessEnabled = true;
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_intens_auto_off() {
  autoBrightnessEnabled = false;
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_temp_auto_on() {
  autoTempEnabled = true;
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_temp_auto_off() {
  autoTempEnabled = false;
  server.send(200, "text/html", SendHTML_Main()); 
}
void handle_setting() {
  server.send(200, "text/html", SendHTML_Setting()); 
}
void handle_ir() {
  autoSwitchOffEnabled =! autoSwitchOffEnabled;
  server.send(200, "text/html", SendHTML_Setting()); 
}
void handle_gas() {
  gasSensorAlarmEnabled = !gasSensorAlarmEnabled;
  server.send(200, "text/html", SendHTML_Setting()); 
}

void handle_reading() {
  manualBrightness = 0.4;
  manualTemp = 0.8;
  autoBrightnessEnabled = false;
  autoTempEnabled = false;
  Serial.print("reading");
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_studying() {
  autoBrightness = 0.7;
  manualTemp = 0.1;
  autoBrightnessEnabled = true;
  autoTempEnabled = false;
  Serial.print("studying");
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_relax() {
  manualBrightness = 0.2;
  manualTemp = 0.8;
  autoBrightnessEnabled = false;
  autoTempEnabled = false;
  Serial.print("relax");
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_night() {
  manualBrightness = 0.1;
  manualTemp = 1;
  autoBrightnessEnabled = false;
  autoTempEnabled = false;
  Serial.print("night");
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_intens_plus() {
  //Manual oder Auto?
  float brightness;
  if (autoBrightnessEnabled) brightness = autoBrightness;
  else brightness = manualBrightness;
  
  brightness = brightness + 0.1;
  if (brightness > 1) brightness = 1;

  if (autoBrightnessEnabled) autoBrightness = brightness;
  else manualBrightness = brightness;
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_intens_minus() {
  //Manual oder Auto?
  float brightness;
  if (autoBrightnessEnabled) brightness = autoBrightness;
  else brightness = manualBrightness;
  
  brightness = brightness - 0.1;
  if (brightness < 0) brightness = 0;

  if (autoBrightnessEnabled) autoBrightness = brightness;
  else manualBrightness = brightness;
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_temp_plus() {
  //Manual oder Auto?
  float temp;
  if (autoTempEnabled) temp = autoTemp;
  else temp = manualTemp;
  
  temp = temp + 0.1;
  if (temp > 1) temp = 1;

  if (autoTempEnabled) autoTemp = temp;
  else manualTemp = temp;
  server.send(200, "text/html", SendHTML_Main()); 
}

void handle_temp_minus() {
  //Manual oder Auto?
  float temp;
  if (autoTempEnabled) temp = autoTemp;
  else temp = manualTemp;
  
  temp = temp - 0.1;
  if (temp < 0) temp = 0;

  if (autoTempEnabled) autoTemp = temp;
  else manualTemp = temp;
  server.send(200, "text/html", SendHTML_Main());  
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


//Input der Plus/Minus Buttons
void handleManualSensors(long frameTime){
  //Manual oder Auto?
  float brightness;
  if (autoBrightnessEnabled) brightness = autoBrightness;
  else brightness = manualBrightness;

  //Buttons gedrückt?
  plusPressed = (touchRead(CAP_PLUS_PIN) < CAP_SENSOR_THRESHOLD);
  minusPressed = (touchRead(CAP_MINUS_PIN) < CAP_SENSOR_THRESHOLD);
  
  if (plusPressed){
    plusCounter = plusCounter + frameTime;
  }else{
    plusCounter = 0;
  }
  if (minusPressed){
    minusCounter = minusCounter + frameTime;
  }else{
    minusCounter = 0;
  }

  //Puffer überschritten?
  if (plusCounter >= AUTO_PUFFER){
    brightness = brightness + RISE_SPEED * (frameTime) / (float)1000;
    personAway = false;
    personAwaySince = 0;
  }
   if (minusCounter >= AUTO_PUFFER){
    brightness = brightness - RISE_SPEED * (frameTime) / (float)1000;
    personAway = false;
    personAwaySince = 0;
  }
  
  if (brightness > 1) brightness = 1;
  if (brightness < 0) brightness = 0;  

  //Manual oder Auto?
  if (autoBrightnessEnabled) autoBrightness = brightness;
  else manualBrightness = brightness;
}

//Input des Auto-Buttons
void handleAutoSensor(long frameTime){
  autoPressed = (touchRead(CAP_AUTO_PIN) < CAP_SENSOR_THRESHOLD);
  
  if (autoPressed && !autoLastFrame){
    autoCounter = autoCounter + frameTime;
  }else{
    autoCounter = 0;
  }
  if (autoPressed == false) autoLastFrame = false;
  
  if (autoCounter >= AUTO_PUFFER){
    autoBrightnessEnabled = !autoBrightnessEnabled;
    autoCounter = 0;
    autoLastFrame = true;
    personAway = false;
    personAwaySince = 0;  
  }
}

void handleAirQuality(long frameTime){
  if(!gasSensorAlarmEnabled) return;              //Wenn Gas-Alarm deaktiviert
  if(!airQuality && !gasSensorTriggered) return;  //Luftqulität ist gut und kein Alarm getriggered
  if(!airQuality && gasSensorTriggered){          //Luftqulität ist gut aber Alarm war getriggered
    gasSensorTriggered = false;
    gasAlarmActive =false;
    timeSinceLastAlarmGas = ALARM_DELAY_GAS+1;
    alarmDurationGas = 0;
    return;
  }
  if(!gasSensorTriggered){                         //Luft schlecht und es wurde noch kein Alarm ausgelöst
    gasSensorTriggered = true;
    gasAlarmActive = true;
    return;
  }
  
  if((alarmDurationGas < ALARM_DURATION_GAS) && (timeSinceLastAlarmGas > ALARM_DELAY_GAS)){
    alarmDurationGas = alarmDurationGas + frameTime;
    return;
  }  if((alarmDurationGas > ALARM_DURATION_GAS ) && (timeSinceLastAlarmGas > ALARM_DELAY_GAS)){
    gasAlarmActive = false;
    alarmDurationGas = 0;
    timeSinceLastAlarmGas = frameTime;
    return;
  }
  if(timeSinceLastAlarmGas < ALARM_DELAY_GAS){
    timeSinceLastAlarmGas = timeSinceLastAlarmGas + frameTime;
    return;
  }
  if(timeSinceLastAlarmGas > ALARM_DELAY_GAS){
    gasAlarmActive = true;
    return;
  }
}
