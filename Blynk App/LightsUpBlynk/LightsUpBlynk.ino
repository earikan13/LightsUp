#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp8266.h>

extern "C" {
#include "user_interface.h"
}

#define PIN 0 // Pin for led strip
#define NUM_LEDS 60
int brightness = 125;
int fadeAndChangePeriod = 250;
bool Connected2Blynk = false;
bool on_off_change = true;
int effect = 5;
volatile int currentColor = 0;
volatile int R = 0;
volatile int G = 0;
volatile int B = 0;
int timerID;
bool isFirstConnect = true;
bool change = true;
WiFiClient client;
BlynkTimer fadeAndChangeTimer;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
WiFiManager wifiManager;

BLYNK_CONNECTED() {
  if (isFirstConnect) {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

BLYNK_WRITE(V5)
{
  change = true;
  brightness = param.asInt();
  strip.setBrightness(brightness);
  strip.show();
}

BLYNK_WRITE(V0)
{
  change = true;
  fadeAndChangePeriod = param.asInt();
  fadeAndChangeTimer.deleteTimer(timerID);
  timerID = fadeAndChangeTimer.setInterval(fadeAndChangePeriod, fadeAndChange);
}

BLYNK_WRITE(V1) { //mode selection
  change = true;
  leds_off();
  currentColor = 0;
  R = 0;
  G = 0;
  B = 0;
  Blynk.virtualWrite(V3, R, G, B);
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
  change = true;
  if (effect == 1) {
    R = param[0].asInt();
    G = param[1].asInt();
    B = param[2].asInt();
    stableRGB(R, G, B);
  }
}

void setup()
{
  WiFi.mode(WIFI_STA);
  strip.begin();
  leds_off();
  strip.setBrightness(brightness);
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("LightsUp")) {
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000);
  }
  timerID = fadeAndChangeTimer.setInterval(fadeAndChangePeriod, fadeAndChange);
  Blynk.begin("b3PMh2RMpbuoJGZEp3k_X3fdFmk0kHOp", WiFi.SSID().c_str(), WiFi.psk().c_str());
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V1, 5);
  Blynk.virtualWrite(V5, 125);
  Blynk.virtualWrite(V0, 75);
  Blynk.virtualWrite(V3, 0, 0, 0);
}
void loop()
{
  if (Blynk.connected())
  {
    BLYNK_CONNECTED();
    Blynk.run();
  }

  if (effect == 2)
  {
    fadeAndChangeTimer.run();
  }
  else
  {
    if (!change)
    {
      delay(200);
      light_sleep();
      delay(200);
      if (!wifiManager.autoConnect("LightsUp")) {
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(2000);
        isFirstConnect = true;
      }
      Blynk.connect(3333);
    }
  }
}
void fadeAndChange()
{
  switch (currentColor)
  {
    case 0:
      G += 1;
      if (G == 255)
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
      B -= 1;
      if (B == 0)
      {
        currentColor = 2;
      }
      break;
    case 2:
      R += 1;
      if (R == 255)
      {
        currentColor = 3;
      }
      break;
    case 3:
      G -= 1;
      if (G == 0)
      {
        currentColor = 4;
      }
      break;
    case 4:
      B += 1;
      if (B == 255)
      {
        currentColor = 5;
      }
      break;
    case 5:
      R -= 1;
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
  // NOT IMPLEMENTED
  change = false;
}

void leds_off()
{
  for (int i = 0; i < strip.numPixels(); i = i + 2)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
  change = false;
}
void light_sleep() {
  wifi_station_disconnect();
  wifi_set_opmode_current(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); // set sleep type, the above    posters wifi_set_sleep_type() didnt seem to work for me although it did let me compile and upload with no errors
  wifi_fpm_open(); // Enables force sleep
  wifi_fpm_do_sleep(10000); // Sleep for longest possible time
}
