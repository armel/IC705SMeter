// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <M5Stack.h>
#include "BluetoothSerial.h"
#include <font.h>
#include <image.h>

#define VERSION "0.3"
#define AUTHOR "F4HWN"
#define NAME "IC705Meter"

#define IC705_ADDRESS 0xE0 // IC705 default address

float xNew;
float yNew;

BluetoothSerial CAT;