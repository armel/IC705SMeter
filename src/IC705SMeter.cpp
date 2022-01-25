// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <IC705SMeter.h>

// Manage rotation
void rotate(float x, float y, float angle) {
    angle = angle * PI/180;

    xNew = x * cos(angle) - y * sin(angle);
    yNew = x * sin(angle) + y * cos(angle);
}

// List files on SPIFFS
void getBinaryList(File dir)
{
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }

    if(strstr(entry.name(), "/.") == NULL && strstr(entry.name(), ".bin") != NULL) {
      binFilename[binIndex] = entry.name();
      binIndex++;
    }

    if (entry.isDirectory() && strstr(entry.name(), "/.") == NULL) {
      getBinaryList(entry);
    }

    entry.close();
  }
}

// Bin Loader
void binLoader() {
  boolean click = 0;
  int8_t cursor = 0;
  int8_t change = 255;
  String tmpName;

  if(!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  root = SPIFFS.open("/");
  getBinaryList(root);

  if(binIndex != 0) {
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);

    for (uint8_t i = TIMEOUT_BIN_LOADER * 10; i > 0; i--) {
      M5.update();

      if( i % 10 == 0) {
        tmpName += ".";
        M5.Lcd.drawString(tmpName, 160, 20);
      }

      if(M5.BtnA.wasPressed() || M5.BtnC.wasPressed()) {
        return;
      }
      else if(M5.BtnB.wasPressed()) {
        click = 1;
        break;
      }

      vTaskDelay(100);
    }
  }

  while(click == 1) {
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(2);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.drawString("Bin Loader", 160, 20);

    M5.update();

    if(M5.BtnA.wasPressed()) {
      cursor--;
    }
    else if(M5.BtnC.wasPressed()) {
      cursor++;
    }
    else if(M5.BtnB.wasPressed()) {
      updateFromFS(SPIFFS, binFilename[cursor]);
      ESP.restart(); 
    }

    cursor = (cursor < 0) ? binIndex - 1 : cursor;
    cursor = (cursor > binIndex - 1) ? 0 : cursor;

    if(change != cursor) {
      change = cursor;
      M5.Lcd.setTextPadding(320);

      for(uint8_t i = 0; i < binIndex; i++) {
        tmpName = binFilename[i].substring(1);

        if (cursor == i) {
          tmpName = ">> " + tmpName + " <<";
        }
        
        M5.Lcd.drawString(tmpName, 160, 60 + i * 20);
      }
    }
    vTaskDelay(100);
  }
}

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

    M5.Lcd.setBrightness(32);
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(WHITE);
    
    M5.Lcd.drawBitmap(0,0,320, 183, (uint16_t *)SMETER01);

    CAT.begin("IC705SMeter");
}

// Main loop
void loop()
{
    String sMeterString;

    uint8_t sMeterVal0 = 0;
    float_t sMeterVal1 = 0;  
    float_t sMeterVal2 = 0;
    static uint8_t sMeterVal3 = -1;

    float_t angle = 0;

    uint16_t x = 0;
    uint16_t y = 0;

    uint16_t counter = 0;
    uint8_t buffer[1024]; 
    uint8_t byte1, byte2, byte3;

    uint8_t request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x15, 0x02, 0xFD};

    char str[12];
   
    for (uint8_t i = 0; i < sizeof(request); i++)
    {
        CAT.write(request[i]);
    }

    delay(25);

    while (CAT.available())
    {
        byte1 = CAT.read();
        byte2 = CAT.read();

        if (byte1 == 0xFE && byte2 == 0xFE) {
            counter = 0;
            byte3 = CAT.read();
            while(byte3 != 0xFD) {
                buffer[counter] = byte3;
                byte3 = CAT.read();
                counter++;
            }
        }
        if(counter == 6) {
            sprintf(str, "%02x%02x", buffer[4], buffer[5]);
            sMeterVal0 = atoi(str);

            if(sMeterVal0 <= 120) {     // 120 = S9 = 9 * (40/3)
                sMeterVal1 = sMeterVal0 / (40/3.0f);
                sMeterVal2 = sMeterVal0 - (sMeterVal1 * (40/3));
            }
            else {                      // 240 = S9 + 60 
                sMeterVal1 = (sMeterVal0 - 120) / 2.0f;
                sMeterVal2 = sMeterVal0 - (sMeterVal1 * 2);
            }

            if(abs(sMeterVal0 - sMeterVal3) > 2) {
                sMeterVal3 = sMeterVal0;

                M5.Lcd.drawBitmap(0,0,320, 183, (uint16_t *)SMETER01);
                M5.Lcd.fillRect(120,160,80,59, TFT_WHITE);

                if(sMeterVal0 <= 120) 
                {
                    angle = map(sMeterVal0, 0, 120, 52, -5); // SMeter image start at S1 so S0 is out of image on the left... (angle 52)
                    sMeterString = "S" + String(int(round(sMeterVal1)));
                }
                else 
                {
                    angle = - map(sMeterVal0, 121, 240, 5, 47);
                    sMeterString = "S9+" + String(int(round(sMeterVal1))) + "dB";
                }

                // Debug trace
                Serial.print(sMeterVal0);
                Serial.print(" ");
                Serial.print(sMeterVal1);
                Serial.print(" ");
                Serial.print(sMeterVal2);
                Serial.print(" ");
                Serial.println(angle);

                // Draw line
                x = 0;
                y = 180;

                rotate(x, y, angle);

                x = 160 + int(xNew);
                y = 200 - int(yNew);

                M5.Lcd.drawLine(162, 200, x, y, TFT_DARKGREY);
                M5.Lcd.drawLine(161, 200, x, y, TFT_BLACK);
                M5.Lcd.drawLine(160, 200, x, y, TFT_BLACK);
                M5.Lcd.drawLine(159, 200, x, y, TFT_BLACK);
                M5.Lcd.drawLine(158, 200, x, y, TFT_DARKGREY);

                // Write SMeter
                M5.Lcd.setTextDatum(CC_DATUM);

                M5.Lcd.setFreeFont(FSSB18);
                M5.Lcd.setTextPadding(320);
                M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
                M5.Lcd.drawString(sMeterString, 160, 200);

                M5.Lcd.setFreeFont(0);
                M5.Lcd.setTextPadding(0);
                M5.Lcd.setTextColor(TFT_DARKGREY, TFT_WHITE);
                M5.Lcd.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160, 225);
            }
        }
        delay(25);
    }
}