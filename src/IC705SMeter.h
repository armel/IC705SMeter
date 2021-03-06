// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <M5Stack.h>
#include "BluetoothSerial.h"
#include <font.h>
#include <image.h>

#define VERSION 0.1
#define AUTHOR  "F4HWN"
#define NAME    "IC705Meter"

float x_new;
float y_new;

BluetoothSerial CAT;