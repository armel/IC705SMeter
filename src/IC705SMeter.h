// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Board
#define BOARD BASIC

#define BASIC 1
#define GREY 2
#define CORE2 3

#define TIMEOUT_BIN_LOADER 3 // 3 sec

#if BOARD == BASIC
  #include <M5Stack.h>
#elif BOARD == GREY
  #include <M5Stack.h>
#elif BOARD == CORE2
  #include <M5Core2.h>
#endif

#include "BluetoothSerial.h"
#include <font.h>
#include <image.h>
#include "FS.h"
#include "SPIFFS.h"
#include <M5StackUpdater.h>

#define VERSION "1.1"
#define AUTHOR "F4HWN"
#define NAME "IC705SMeter"

#define IC705_CI_V_ADDRESS 0xA4 // IC705 CI-V default address 0xA4

#define TFT_BACK M5.Lcd.color565(255, 248, 236)
#define TFT_NEDDLE_1 M5.Lcd.color565(241, 120, 100)
#define TFT_NEDDLE_2 M5.Lcd.color565(241, 170, 170)

// Bluetooth connector
BluetoothSerial CAT;

// Reset
boolean reset = true;

// Bin loader
File root;
String binFilename[8];
uint8_t binIndex = 0;

#undef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 40000000