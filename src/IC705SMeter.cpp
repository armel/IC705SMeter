// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <IC705SMeter.h>

void rotate(float x, float y, float angle) {
    angle = angle * PI/180;

    x_new = x * cos(angle) - y * sin(angle);
    y_new = x * sin(angle) + y * cos(angle);
}

void setup()
{
    Serial.begin(115200);

    M5.begin(true, false, false, false);

    M5.Lcd.setBrightness(32);
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(WHITE);
    
    M5.Lcd.drawBitmap(0,0,320, 183, (uint16_t *)SMETER01);

    CAT.begin("IC705SMeter");
}

void loop()
{
    String sMeterString;

    int x = 0;
    int y = 0;

    float sMeterVal0 = 0;
    float sMeterVal1 = 0;  
    float sMeterVal2 = 0;

    float angle = 0;

    uint8_t counter = 0;
    uint8_t read_buffer[12]; //Read buffer
    uint8_t byte1, byte2, byte3;

    char str[9];

    uint8_t request[] = {0xFE, 0xFE, 0xA4, 0xE0, 0x15, 0x02, 0xFD};

    for (uint8_t i = 0; i < sizeof(request); i++)
    {
        CAT.write(request[i]);
    }

    delay(20);

    while (CAT.available())
    {
        byte1 = CAT.read();
        byte2 = CAT.read();

        if (byte1 == 0xFE && byte2 == 0xFE) {
            counter = 0;
            byte3 = CAT.read();
            while(byte3 != 0xFD) {
                read_buffer[counter] = byte3;
                byte3 = CAT.read();
                counter++;
            }
        }
        if(counter == 6) {
            sprintf(str, "%02x%02x", read_buffer[4], read_buffer[5]);
            sMeterVal0 = atoi(str);

            if(sMeterVal0 <= 120) {
                sMeterVal1 = sMeterVal0 / (40/3);
                sMeterVal2 = sMeterVal0 - (sMeterVal1 * (40/3));

                Serial.print(sMeterVal0);
                Serial.print(" ");
                Serial.print("S");
                Serial.print(sMeterVal1);
                Serial.print(", ");
                Serial.print(sMeterVal2);
                Serial.println("");
            }
            else {
                sMeterVal1 = (sMeterVal0 - 120) / 2;
                sMeterVal2 = sMeterVal0 - (sMeterVal1 * 2);

                Serial.print(sMeterVal0);
                Serial.print(" ");
                Serial.print("S9+");
                Serial.print(sMeterVal1);
                Serial.print(", ");
                Serial.print(sMeterVal2);
                Serial.println("");
            }

            M5.Lcd.drawBitmap(0,0,320, 183, (uint16_t *)SMETER01);
            M5.Lcd.fillRect(120,160,80,79, TFT_WHITE);

            if(sMeterVal0 <= 120) 
            {
                angle = 47 - ((47 / 120.0f) * sMeterVal0);
                sMeterString = "S" + String(int(round(sMeterVal1)));
            }
            else 
            {
                angle = -((47 / 120.0f) * (sMeterVal0 - 120));
                sMeterString = "S9+" + String(int(round(sMeterVal1)));
            }

            // Draw line

            x = 0;
            y = 180;

            rotate(x, y, angle);

            x = 160 + int(x_new);
            y = 200 - int(y_new);

            M5.Lcd.drawLine(160, 200, x, y, BLACK);

            // Write SMeter

            M5.Lcd.setTextDatum(CC_DATUM);

            M5.Lcd.setFreeFont(&rounded_led_board10pt7b);
            M5.Lcd.setTextPadding(320);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
            M5.Lcd.drawString(sMeterString, 160, 200);

            M5.Lcd.setFreeFont(0);
            M5.Lcd.setTextPadding(0);
            M5.Lcd.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160, 230);
        }
        delay(100);
    }
}