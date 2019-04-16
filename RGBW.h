#include <NeoPixelBus.h>

const uint16_t PixelCount = 8; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 14;  // make sure to set this to the correct pin, ignored for Esp8266

#define colorSaturation 100

NeoPixelBus<NeoRgbwFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);


RgbColor red(0, colorSaturation, 0);
RgbColor green(colorSaturation, 0, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor tuerkis(colorSaturation, 0, colorSaturation);
RgbColor lila(0, colorSaturation/2, colorSaturation);
RgbColor rosa(colorSaturation*20/255, colorSaturation, colorSaturation*147/255);
RgbColor orange(colorSaturation*0.65, colorSaturation, 0);
RgbColor black(0);



void RGBWsetup()
{
    while (!Serial); // wait for serial attach

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

    Serial.println();
}

void RGBWhochfahren(){

  for(int i = 0; i <= 7; i++){
    strip.SetPixelColor(i, blue);
    strip.Show();
    delay(100);
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(100);
  }
  for(int i = 6; i >= 0; i--){
    strip.SetPixelColor(i, blue);
    strip.Show();
    delay(100);
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(100);
  }
  for(int i = 0; i <= 7; i++){
    strip.SetPixelColor(i, orange);
    strip.Show();
    delay(100);
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(100);
  }
  for(int i = 6; i >= 0; i--){
    strip.SetPixelColor(i, orange);
    strip.Show();
    delay(100);
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(100);
  }
  for(int i = 0; i <= 7; i++){
    strip.SetPixelColor(i, green);
    strip.Show();
    delay(300);
  }
  delay(150);
  for(int i = 0; i <= 7; i++){
    strip.SetPixelColor(i, black);
    strip.Show();
    delay(100);
  }

}

void RGBWpassiv(double userSaturation, String color){  //int userSaturation zwischen 0 und 1, RgbColor farbe

  userSaturation = userSaturation*255;
  for(int i = 0; i <= 7; i++){
    if(color.equals("red")){
      RgbColor red_user(0, userSaturation, 0);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, red_user);
        strip.Show();
      }
    }
    else if(color.equals("green")){
      RgbColor green_user(userSaturation, 0, 0);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, green_user);
        strip.Show();
      }
    }
    else if(color.equals("blue")){
      RgbColor blue_user(0, 0, userSaturation);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, blue_user);
        strip.Show();
      }
    }
    else if(color.equals("white")){
      RgbColor white_user(userSaturation);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, white_user);
        strip.Show();
      }
    }
    else if(color.equals("tuerkis")){
      RgbColor tuerkis_user(userSaturation, 0, userSaturation);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, tuerkis_user);
        strip.Show();
      }
    }
    else if(color.equals("lila")){
      RgbColor lila_user(0, userSaturation/2, userSaturation);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, lila_user);
        strip.Show();
      }
    }
    else if(color.equals("rosa")){
      RgbColor rosa_user(userSaturation*20/255, userSaturation, userSaturation*147/255);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, rosa_user);
        strip.Show();
      }
    }else if(color.equals("orange")){
      RgbColor orange_user(userSaturation*0.65, userSaturation, 0);
      for(int i=0; i<= 7; i++){
        strip.SetPixelColor(i, orange_user);
        strip.Show();
      }
    }
    
  }

}

void RGBWaus(){
  for(int i=0; i<= 7; i++){
    strip.SetPixelColor(i, black);
  }
  strip.Show();
}
 
void RGBWtemp(float temp, float brightness){
 RgbColor tempColor(0.1*colorSaturation, colorSaturation*temp, 0.7*colorSaturation*(1-temp));
 for(int i=0; i<= 7; i++){
	strip.SetPixelColor(i, tempColor);
	strip.Show();
  }
}	 