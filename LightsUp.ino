#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_DEBUG // Optional, this enables lots of prints
#define BLYNK_PRINT Serial

extern "C" {
#include "user_interface.h"
}

#define PIN 0

int brightness = 125;
int period_fadeANDchange = 75;
bool Connected2Blynk = false;
bool on_off_change = true;
int effect;
bool change = true;
int currentColor = 0;
int R = 0;
int G = 0;
int B = 0;
int timerID;

WiFiClient client;
BlynkTimer timer_fadeANDchange;
BlynkTimer checkConnec;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

BLYNK_CONNECTED()
{
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V1, 2);
  Blynk.virtualWrite(V5, 125);
  Blynk.virtualWrite(V0, 75);
  Blynk.virtualWrite(V3, 0, 0, 0);
  Blynk.syncAll();
}

BLYNK_WRITE(V5)
{
  brightness = param.asInt();
  strip.setBrightness(brightness);
  strip.show();
  change = true;
}

BLYNK_WRITE(V0)
{
  period_fadeANDchange = param.asInt();
  timer_fadeANDchange.deleteTimer(timerID);
  timerID = timer_fadeANDchange.setInterval(period_fadeANDchange, fadeANDchange);
  //timer_fadeANDchange.enable(timerID);
  change = true;
}

BLYNK_WRITE(V1) { //mode selection
  leds_off();
  currentColor = 0;
  R = 0;
  G = 0;
  B = 0;
  Blynk.virtualWrite(V3, 0, 0, 0);
  switch (param.asInt())
  {
    case 1: // All white
      effect = 0;
      change = true;
      break;

    case 2: // RGB
      effect = 1;
      change = true;
      break;

    case 3: // Slow change
      effect = 2;
      change = true;
      break;

    case 4: // Diff
      effect = 3;
      change = true;
      break;

    case 5: // Off
      effect = 4;
      change = true;
      break;
  }
}

BLYNK_WRITE(V3)
{
  if (effect == 1) {
    R = param[0].asInt();
    G = param[1].asInt();
    B = param[2].asInt();
    change = true;
  }
}

void setup()
{
  randomSeed(analogRead(0));
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("LightsUp")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000);
  }
  Blynk.begin("b3PMh2RMpbuoJGZEp3k_X3fdFmk0kHOp", WiFi.SSID().c_str(), WiFi.psk().c_str());
  int timerID = timer_fadeANDchange.setInterval(period_fadeANDchange, fadeANDchange);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}
void loop()
{
  while (WiFi.status() == WL_CONNECTED)
  {
    Blynk.run();

    if (!change)
    {}
    else
    {
      switch (effect)
      {
        case 0: // all white
          allWhite();
          change = false;
          break;

        case 1: //stable rgb
          stableRGB(R, G, B);
          change = false;
          break;

        case 2: //slow change
          timer_fadeANDchange.run();
          break;

        case 3:
          diff();
          change = false;
          break;
          
        case 4:
          leds_off();
          change = false;
          break;

        default:
          break;
      }
    }
  }
}
void fadeANDchange()
{
  switch (currentColor)
  {
    case 0: //green arttır
      //Serial.println(G);
      G += 3;
      if (G == 252) {
        if (B == 0) {
          currentColor = 2;
        }
        else {
          currentColor = 1;
        }
      }
      break;
    case 1:
      B -= 3;
      if (B == 0) {
        currentColor = 2;
      }
      break;
    case 2: //sarı
      R += 3;
      if (R == 252) {
        currentColor = 3;
      }
      break;
    case 3: //kırmızı
      G -= 3;
      if (G == 0) {
        currentColor = 4;
      }
      break;
    case 4: //magenta
      B += 3;
      if (B == 252) {
        currentColor = 5;
      }
      break;
    case 5:  //mavi
      R -= 3;
      if (R == 0) {
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
  for (int i = 0; i < strip.numPixels(); i = i + 2) {
    strip.setPixelColor(i, R, G, B);
  }
  strip.show();
}
void allWhite()
{
  for (int i = 0; i < strip.numPixels(); i = i + 2) {
    strip.setPixelColor(i, 255, 255, 255);
  }
  strip.show();
}
void diff()
{
  while (1) {
    byte red = 0xff;
    byte green = 0;
    byte blue = 0;
    int BallCount = 3;

    float Gravity = -9.81;
    int StartHeight = 1;

    float Height[BallCount];
    float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
    float ImpactVelocity[BallCount];
    float TimeSinceLastBounce[BallCount];
    int   Position[BallCount];
    long  ClockTimeSinceLastBounce[BallCount];
    float Dampening[BallCount];

    for (int i = 0 ; i < BallCount ; i++) {
      ClockTimeSinceLastBounce[i] = millis();
      Height[i] = StartHeight;
      Position[i] = 0;
      ImpactVelocity[i] = ImpactVelocityStart;
      TimeSinceLastBounce[i] = 0;
      Dampening[i] = 0.90 - float(i) / pow(BallCount, 2);
    }

    while (true) {
      for (int i = 0 ; i < BallCount ; i++) {
        TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
        Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i] / 1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

        if ( Height[i] < 0 ) {
          Height[i] = 0;
          ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
          ClockTimeSinceLastBounce[i] = millis();

          if ( ImpactVelocity[i] < 0.01 ) {
            ImpactVelocity[i] = ImpactVelocityStart;
          }
        }
        Position[i] = round( Height[i] * (strip.numPixels() - 1) / StartHeight);
      }

      for (int i = 0 ; i < BallCount ; i++) {
        strip.setPixelColor(Position[i], red, green, blue);
      }

      strip.show();
      for (int i = 0 ; i < strip.numPixels() ; i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
    }
  }
}

void leds_off()
{
  for (int i = 0; i < strip.numPixels(); i = i + 2) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}
void CheckConnection() {
  Connected2Blynk = Blynk.connected();
  if (!Connected2Blynk) {
    Serial.println("Not connected to Blynk server");
    Blynk.connect(33333);  // timeout set to 10 seconds and then continue without Blynk
  }
  else {
    Serial.println("Connected to Blynk server");
  }
}
