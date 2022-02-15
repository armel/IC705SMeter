// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <IC705SMeter.h>
#include "functions.h"

// Setup
void setup()
{
  // Debug
  Serial.begin(115200);

  // Init M5
  M5.begin(true, false, false, false);

  // Bin Loader
  binLoader();

  // Let's go
  #if BOARD == CORE2
  M5.Axp.SetLed(0);
  #endif

  M5.Lcd.setBrightness(64);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BACK);

  M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 160);
  M5.Lcd.drawJpg(smeterBottom, sizeof(smeterBottom), 0, 160, 320, 80);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFreeFont(0);
  M5.Lcd.setTextPadding(0);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160, 190);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFreeFont(&robosapien14pt7b);
  M5.Lcd.setTextPadding(0);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.drawString("PWD", 70, 220);
  M5.Lcd.drawString("S", 160, 220);
  M5.Lcd.drawString("SWR", 250, 220);

  CAT.begin("IC705SMeter");
}

// Main loop
void loop()
{
  uint8_t btnA;
  uint8_t btnB; 
  uint8_t btnC;
  static uint8_t mode = 2;

  M5.update();

  btnA = M5.BtnA.read();
  btnB = M5.BtnB.read();
  btnC = M5.BtnC.read();

  if(btnA == 1) {
    mode = 1;
    reset = true;
  }
  else if(btnB == 1) {
    mode = 2;
    reset = true;
  }
  else if(btnC == 1) {
    mode = 3;
    reset = true;
  }

  switch (mode)
  {
  case 1:
    getPower();
    break;
  
  case 2:
    getSmeter();
    break;
  
  case 3:
    getSWR();
    break;
  }

  delay(50);
}