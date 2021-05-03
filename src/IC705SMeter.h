// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Board
#define BOARD GREY

#define BASIC 1
#define GREY  2
#define CORE2 3

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

#define VERSION "0.6"
#define AUTHOR "F4HWN"
#define NAME "IC705SMeter"

#define IC705_CI_V_ADDRESS 0xA4 // IC705 CI-V default address 0xA4

float xNew;
float yNew;

BluetoothSerial CAT;