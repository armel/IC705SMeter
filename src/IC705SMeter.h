// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "settings.h"

#define BASIC 1
#define GREY 2
#define CORE2 3

#define TIMEOUT_BIN_LOADER 3 // 3 sec

#if BOARD == BASIC
  #include <M5Stack.h>
  #include "WebIndexBasicAndGrey.h"
#elif BOARD == GREY
  #include <M5Stack.h>
  #include "WebIndexBasicAndGrey.h"
#elif BOARD == CORE2
  #include <M5Core2.h>
  #include "WebIndexCore2.h"
#endif

#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include "BluetoothSerial.h"
#include <font.h>
#include <image.h>
#include "FS.h"
#include "SPIFFS.h"
#include <M5StackUpdater.h>

#define VERSION "1.8"
#define AUTHOR "F4HWN"
#define NAME "IC705SMeter"

// Needle
#define TFT_BACK M5.Lcd.color565(255, 248, 236)
#define TFT_MODE M5.Lcd.color565(84, 103, 143)
#define TFT_NEDDLE_1 M5.Lcd.color565(241, 120, 100)
#define TFT_NEDDLE_2 M5.Lcd.color565(241, 170, 170)

// Bluetooth connector
BluetoothSerial CAT;

// Preferences
Preferences preferences;

// Global Variables
WiFiServer httpServer(80);
WiFiClient httpClient;
uint8_t htmlGetRequest;
uint8_t htmlGetRefresh = 3;
uint8_t option = 2;
boolean reset = true;
boolean btConnected = false;
char dataMode = 0;

const char *menu[] = {"PWR", "S", "SWR"};

// Web site Screen Capture stuff
#define GET_unknown 0
#define GET_index_page  1
#define GET_screenshot  2

// Flags for button presses via Web site Screen Capture
bool buttonLeftPressed = false;
bool buttonCenterPressed = false;
bool buttonRightPressed = false;

// Bin loader
File root;
String binFilename[8];
uint8_t binIndex = 0;

// Optimize SPI Speed
#undef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 40000000