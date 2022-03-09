// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <IC705SMeter.h>
#include "functions.h"
#include "tasks.h"

// Setup
void setup()
{
  uint8_t loop = 0;

  // Debug
  Serial.begin(115200);

  // Init M5
  M5.begin(true, false, false, false);

  // Preferences
  preferences.begin(NAME);
  option = preferences.getUInt("option", 2);

  // Bin Loader
  binLoader();

  // Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED && loop <= 10)
  {
    delay(250);
    loop += 1;
  }

  // Start server (for Web site Screen Capture)
  httpServer.begin();

// Let's go
#if BOARD == CORE2
  M5.Axp.SetLed(0);
#endif

  M5.Lcd.setBrightness(64);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BACK);

  M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 160);
  M5.Lcd.drawJpg(smeterBottom, sizeof(smeterBottom), 0, 160, 320, 80);

  /*
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFreeFont(0);
  M5.Lcd.setTextPadding(0);
  M5.Lcd.setTextColor(TFT_DARKGREY);
  M5.Lcd.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160, 160);
  */

  CAT.register_callback(callbackBT);

  if (!CAT.begin(NAME))
  {
    Serial.println("An error occurred initializing Bluetooth");
  }
  else
  {
    Serial.println("Bluetooth initialized");
  }

  // Multitasking task for retreive button
  xTaskCreatePinnedToCore(
      button,   // Function to implement the task
      "button", // Name of the task
      8192,     // Stack size in words
      NULL,     // Task input parameter
      4,        // Priority of the task
      NULL,     // Task handle
      1);       // Core where the task should run
}

// Main loop
void loop()
{
  static uint8_t alternance = 0;

  if (btConnected == false)
  {
    value("NEED PAIRING");
  }

  if (btConnected == true)
  {
    getMode();
    getFrequency();

    switch (option)
    {
    case 0:
      getPower();
      break;

    case 1:
      getSmeter();
      break;

    case 2:
      getSWR();
      break;
    }
  }

  viewMenu();

  if (alternance == 0)
  {
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFreeFont(0);
    M5.Lcd.setTextPadding(160);
    M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BACK);
    M5.Lcd.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160, 160);
  }
  else if (alternance == 20 && WiFi.status() == WL_CONNECTED)
  {
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFreeFont(0);
    M5.Lcd.setTextPadding(160);
    M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BACK);
    M5.Lcd.drawString(String(WiFi.localIP().toString().c_str()), 160, 160);
  }

  alternance = (alternance++ < 30) ? alternance : 0;

  if (WiFi.status() == WL_CONNECTED)
  {
    getScreenshot();
  }

  vTaskDelay(10);
}