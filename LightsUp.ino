#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp8266.h>

extern "C" {
#include "user_interface.h"
}

#define PIN 0 // Pin for led strip

int brightness = 125;
int fadeAndChangePeriod = 75;
bool Connected2Blynk = false;
bool on_off_change = true;
int effect;
int currentColor = 0;
int R = 0;
int G = 0;
int B = 0;
int timerID;

WiFiClient client;
BlynkTimer fadeAndChangeTimer;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

BLYNK_CONNECTED()
{
  Blynk.syncAll();
}

BLYNK_WRITE(V5)
{
  brightness = param.asInt();
  strip.setBrightness(brightness);
  strip.show();
}

BLYNK_WRITE(V0)
{
  fadeAndChangePeriod = param.asInt();
  fadeAndChangeTimer.deleteTimer(timerID);
  timerID = fadeAndChangeTimer.setInterval(fadeAndChangePeriod, fadeAndChange);
}

BLYNK_WRITE(V1) { //mode selection
  leds_off();
  currentColor = 0;
  R = 0;
  G = 0;
  B = 0;
  fadeAndChangeTimer.deleteTimer(timerID);
  Blynk.virtualWrite(V3, 0, 0, 0);
  switch (param.asInt())
  {
    case 1: // All white
      effect = 0;
      allWhite();
      break;

    case 2: // RGB
      effect = 1;
      break;

    case 3: // Slow change
      effect = 2;
      timerID = fadeAndChangeTimer.setInterval(fadeAndChangePeriod, fadeAndChange);
      fadeAndChangeTimer.run();
      break;

    case 4: // NOT IMPLEMENTED
      effect = 3;
      break;

    case 5: // Off
      effect = 4;
      leds_off();
      break;
  }
}

BLYNK_WRITE(V3)
{
  if (effect == 1) {
    R = param[0].asInt();
    G = param[1].asInt();
    B = param[2].asInt();
    stableRGB(R, G, B);
  }
}

void setup()
{
  randomSeed(analogRead(0));
  WiFi.mode(WIFI_STA);
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("LightsUp")) {
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000);
  }
  Blynk.begin("b3PMh2RMpbuoJGZEp3k_X3fdFmk0kHOp", WiFi.SSID().c_str(), WiFi.psk().c_str());
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V1, 2);
  Blynk.virtualWrite(V5, 125);
  Blynk.virtualWrite(V0, 75);
  Blynk.virtualWrite(V3, 0, 0, 0);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}
void loop()
{
  Blynk.run();
}
void fadeAndChange()
{
  switch (currentColor)
  {
    case 0:
      G += 3;
      if (G == 252)
      {
        if (B == 0)
        {
          currentColor = 2;
        }
        else {
          currentColor = 1;
        }
      }
      break;
    case 1:
      B -= 3;
      if (B == 0)
      {
        currentColor = 2;
      }
      break;
    case 2:
      R += 3;
      if (R == 252)
      {
        currentColor = 3;
      }
      break;
    case 3:
      G -= 3;
      if (G == 0)
      {
        currentColor = 4;
      }
      break;
    case 4:
      B += 3;
      if (B == 252)
      {
        currentColor = 5;
      }
      break;
    case 5:
      R -= 3;
      if (R == 0)
      {
        currentColor = 0;
      }
      break;
  }
  for (int i = 0; i < strip.numPixels(); i = i + 2)
  {
    strip.setPixelColor(i, R, G, B);
  }
  strip.show();
}

void stableRGB(int R, int G, int B)
{
  for (int i = 0; i < strip.numPixels(); i = i + 2)
  {
    strip.setPixelColor(i, R, G, B);
  }
  strip.show();
}
void allWhite()
{
  for (int i = 0; i < strip.numPixels(); i = i + 2)
  {
    strip.setPixelColor(i, 255, 255, 255);
  }
  strip.show();
}
void diff()
{

}

void leds_off()
{
  for (int i = 0; i < strip.numPixels(); i = i + 2)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}
//void CheckConnection()
//{
//  Connected2Blynk = Blynk.connected();
//  if (!Connected2Blynk)
//  {
//    Serial.println("Not connected to Blynk server");
//    Blynk.connect(33333);  // timeout set to 10 seconds and then continue without Blynk
//  }
//  else
//  {
//    Serial.println("Connected to Blynk server");
//  }
//}
