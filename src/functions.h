// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Manage rotation
void rotate(uint16_t *x, uint16_t *y, float angle)
{
  float xNew;
  float yNew;

  // Check that these are not null pointer
  assert(x);
  assert(y);

  angle = angle * PI / 180;

  xNew = *x * cos(angle) - *y * sin(angle);
  yNew = *x * sin(angle) + *y * cos(angle);

  *x = xNew;
  *y = yNew;
}

// Manage map float
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Print needle
void needle(float_t angle, uint16_t a = 0, uint16_t b = 200, uint16_t c = 0, uint16_t d = 100) 
{
  uint16_t x, y;

  x = a;
  y = b;

  rotate(&x, &y, angle);
  
  a = 160 + x;
  b = 220 - y;

  x = c;
  y = d;

  rotate(&x, &y, angle);
  
  c = 160 + x;
  d = 220 - y;

  M5.Lcd.drawLine(a + 2, b, c + 2, d, TFT_NEDDLE_2);
  M5.Lcd.drawLine(a + 1, b, c + 1, d, TFT_NEDDLE_1);
  M5.Lcd.drawLine(a, b, c, d, TFT_RED);
  M5.Lcd.drawLine(a - 1, b, c - 1, d, TFT_NEDDLE_1);
  M5.Lcd.drawLine(a - 2, b, c - 2, d, TFT_NEDDLE_2);
}

// Print value
void value(String valString) 
{
  //M5.Lcd.drawJpg(smeterBottom, sizeof(smeterBottom), 0, 160, 320, 80);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFreeFont(&pf_tempesta_seven_bold8pt7b);
  M5.Lcd.setTextPadding(160);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_BACK);
  M5.Lcd.drawString(valString, 160, 170);
}

// List files on SPIFFS
void getBinaryList(File dir)
{
  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }

    if (strstr(entry.name(), "/.") == NULL && strstr(entry.name(), ".bin") != NULL)
    {
      binFilename[binIndex] = entry.name();
      binIndex++;
    }

    if (entry.isDirectory() && strstr(entry.name(), "/.") == NULL)
    {
      getBinaryList(entry);
    }

    entry.close();
  }
}

// Bin Loader
void binLoader()
{
  boolean click = 0;
  int8_t cursor = 0;
  int8_t change = 255;
  String tmpName;

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");

    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(2);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.drawString("Flash File System", 160, 20);
    M5.Lcd.drawString("needs to be formated.", 160, 50);
    M5.Lcd.drawString("It takes around 4 minutes.", 160, 100);
    M5.Lcd.drawString("Please, wait until ", 160, 150);
    M5.Lcd.drawString("the application starts !", 160, 180);

    Serial.println("SPIFFS Formating...");

    SPIFFS.format(); // Format SPIFFS...

    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(0);

    return;
  }

  root = SPIFFS.open("/");
  getBinaryList(root);

  if (binIndex != 0)
  {
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);

    for (uint8_t i = TIMEOUT_BIN_LOADER * 10; i > 0; i--)
    {
      M5.update();

      if (i % 10 == 0)
      {
        tmpName += ".";
        M5.Lcd.drawString(tmpName, 160, 20);
      }

      if (M5.BtnA.wasPressed() || M5.BtnC.wasPressed())
      {
        return;
      }
      else if (M5.BtnB.wasPressed())
      {
        click = 1;
        break;
      }

      vTaskDelay(100);
    }
  }

  while (click == 1)
  {
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(2);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.drawString("Bin Loader", 160, 20);

    M5.update();

    if (M5.BtnA.wasPressed())
    {
      cursor--;
    }
    else if (M5.BtnC.wasPressed())
    {
      cursor++;
    }
    else if (M5.BtnB.wasPressed())
    {
      updateFromFS(SPIFFS, binFilename[cursor]);
      ESP.restart();
    }

    cursor = (cursor < 0) ? binIndex - 1 : cursor;
    cursor = (cursor > binIndex - 1) ? 0 : cursor;

    if (change != cursor)
    {
      change = cursor;
      M5.Lcd.setTextPadding(320);

      for (uint8_t i = 0; i < binIndex; i++)
      {
        tmpName = binFilename[i].substring(1);

        if (cursor == i)
        {
          tmpName = ">> " + tmpName + " <<";
        }

        M5.Lcd.drawString(tmpName, 160, 60 + i * 20);
      }
    }
    vTaskDelay(100);
  }
}

// Get Smeter
void getSmeter()
{
  String valString;

  uint8_t buffer[1024];
  uint8_t byte1, byte2, byte3;
  uint8_t request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x15, 0x02, 0xFD};

  uint8_t val0 = 0;
  float_t val1 = 0;
  float_t val2 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  uint16_t counter = 0;
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

    if (byte1 == 0xFE && byte2 == 0xFE)
    {
      counter = 0;
      byte3 = CAT.read();
      while (byte3 != 0xFD)
      {
        buffer[counter] = byte3;
        byte3 = CAT.read();
        counter++;
      }
    }

    if (counter == 6)
    {
      sprintf(str, "%02x%02x", buffer[4], buffer[5]);
      val0 = atoi(str);

      if (val0 <= 120)
      { // 120 = S9 = 9 * (40/3)
        val1 = val0 / (40 / 3.0f);
        val2 = val0 - (val1 * (40 / 3));
      }
      else
      { // 240 = S9 + 60
        val1 = (val0 - 120) / 2.0f;
        val2 = val0 - (val1 * 2);
      }

      if (abs(val0 - val3) > 1 || reset == true)
      {
        val3 = val0;
        reset = false;

        M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 160);

        if (val0 <= 120)
        {
          angle = mapFloat(val0, 0, 120, 49.0f, -6.50f); // SMeter image start at S1 so S0 is out of image on the left...
          valString = "S " + String(int(round(val1)));
        }
        else
        {
          angle = mapFloat(val0, 121, 241, -6.50f, -43.0f);
          valString = "S 9 + " + String(int(round(val1))) + " dB";
        }

        // Debug trace
        Serial.print(val0);
        Serial.print(" ");
        Serial.print(val1);
        Serial.print(" ");
        Serial.print(val2);
        Serial.print(" ");
        Serial.println(angle);

        // Draw line
        needle(angle);

        // Write Value
        value(valString);
      }
    }
    delay(25);
  }
}

// Get SWR
void getSWR()
{
  String valString;

  uint8_t val0 = 0;
  float_t val1 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  uint16_t counter = 0;
  uint8_t buffer[1024];
  uint8_t byte1, byte2, byte3;

  uint8_t request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x15, 0x12, 0xFD};

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

    if (byte1 == 0xFE && byte2 == 0xFE)
    {
      counter = 0;
      byte3 = CAT.read();
      while (byte3 != 0xFD)
      {
        buffer[counter] = byte3;
        byte3 = CAT.read();
        counter++;
      }
    }

    if (counter == 6)
    {
      sprintf(str, "%02x%02x", buffer[4], buffer[5]);
      val0 = atoi(str);

      if (val0 != val3 || reset == true)
      {
        val3 = val0;
        reset = false;

        M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 160);

        if (val0 <= 48)
        {
          angle = mapFloat(val0, 0, 48, 42.0f, 32.50f);
          val1 = mapFloat(val0, 0, 48, 1.0, 1.5);
        }
        else if (val0 <= 80)
        {
          angle = mapFloat(val0, 49, 80, 32.50f, 24.0f);
          val1 = mapFloat(val0, 49, 80, 1.5, 2.0);
        }
        else if (val0 <= 120)
        {
          angle = mapFloat(val0, 81, 120, 24.0f, 10.0f);
          val1 = mapFloat(val0, 81, 120, 2.0, 3.0);
        }
        else if (val0 <= 155)
        {
          angle = mapFloat(val0, 121, 155, 10.0f, 0.0f);
          val1 = mapFloat(val0, 121, 155, 3.0, 4.0);
        }
        else if (val0 <= 175)
        {
          angle = mapFloat(val0, 156, 175, 0.0f, -7.0f);
          val1 = mapFloat(val0, 156, 175, 4.0, 5.0);
        }
        else if (val0 <= 225)
        {
          angle = mapFloat(val0, 176, 225, -7.0f, -19.0f);
          val1 = mapFloat(val0, 176, 225, 5.0, 10.0);
        }
        else
        {
          angle = mapFloat(val0, 226, 255, -19.0f, -30.50f);
          val1 = mapFloat(val0, 226, 255, 10.0, 50.0);
        }

        valString = "SWR " + String(val1);

        // Debug trace
        Serial.print(val0);
        Serial.print(" ");
        Serial.print(val1);
        Serial.print(" ");
        Serial.println(angle);

        // Draw line
        needle(angle);

        // Write Value
        value(valString);
      }
    }
    delay(25);
  }
}

// Get Power
void getPower()
{
  String valString;

  uint8_t val0 = 0;
  float_t val1 = 0;
  float_t val2 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  uint16_t counter = 0;
  uint8_t buffer[1024];
  uint8_t byte1, byte2, byte3;

  uint8_t request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x14, 0x0A, 0xFD};

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

    if (byte1 == 0xFE && byte2 == 0xFE)
    {
      counter = 0;
      byte3 = CAT.read();
      while (byte3 != 0xFD)
      {
        buffer[counter] = byte3;
        byte3 = CAT.read();
        counter++;
      }
    }

    if (counter == 6)
    {
      sprintf(str, "%02x%02x", buffer[4], buffer[5]);
      val0 = atoi(str);

      if (val0 != val3 || reset == true)
      {
        val3 = val0;
        reset = false;

        M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 160);

        if (val0 <= 13)
        {
          angle = mapFloat(val0, 0, 13, 42.0f, 30.50f);
          val1 = mapFloat(val0, 0, 13, 0, 0.5);
        }
        else if (val0 <= 26)
        {
          angle = mapFloat(val0, 14, 26, 30.50f, 23.50f);
          val1 = mapFloat(val0, 14, 26, 0.5, 1.0);
        }
        else if (val0 <= 51)
        {
          angle = mapFloat(val0, 27, 51, 23.50f, 14.50f);
          val1 = mapFloat(val0, 27, 51, 1.0, 2.0);
        }
        else if (val0 <= 127)
        {
          angle = mapFloat(val0, 52, 127, 14.50f, -6.50f);
          val1 = mapFloat(val0, 52, 127, 2.0, 5.0);
        }
        else if (val0 <= 179)
        {
          angle = mapFloat(val0, 128, 179, -6.50f, -17.50f);
          val1 = mapFloat(val0, 128, 179, 5.0, 7.0);
        }
        else
        {
          angle = mapFloat(val0, 180, 255, -17.50f, -30.50f);
          val1 = mapFloat(val0, 180, 255, 7.0, 10.0);
        }

        val2 = round(val1 * 10);
        valString = "PWR " + String((val2/10)) + " W";

        // Debug trace
        /*
        Serial.print(val0);
        Serial.print(" ");
        Serial.print(val1);
        Serial.print(" ");
        Serial.println(angle);
        */

        // Draw line
        needle(angle);

        // Write Value
        value(valString);
      }
    }
    delay(25);
  }
}

void getDebug()
{
  String valString;

  uint8_t val0 = 60;
  float_t val1 = 0;
  float_t val2 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  uint8_t btnA;
  uint8_t btnC; 

  while (true)
  {
    M5.update();
    btnA = M5.BtnA.read();
    btnC = M5.BtnC.read();

    if(btnA == 1) {
      val0 -= 1;
      btnA = 0;
    }
    if(btnC == 1) {
      val0 += 1;
      btnC = 0;
    }

    delay(100);

    if (val0 <= 120)
    { // 120 = S9 = 9 * (40/3)
      val1 = val0 / (40 / 3.0f);
      val2 = val0 - (val1 * (40 / 3));
    }
    else
    { // 240 = S9 + 60
      val1 = (val0 - 120) / 2.0f;
      val2 = val0 - (val1 * 2);
    }

    if (abs(val0 - val3) > 2 || reset == true)
    {
      val3 = val0;
      reset = false;

      M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 160);

      if (val0 <= 120)
      {
        angle = mapFloat(val0, 0, 120, 49.0f, -6.50f); // SMeter image start at S1 so S0 is out of image on the left...
        valString = "SWR" + String(int(round(val1)));
      }
      else
      {
        angle = mapFloat(val0, 121, 241, -6.50f, -43.0f);
        valString = "SWR" + String(int(round(val1)));
      }

      // Debug trace
      Serial.print(val0);
      Serial.print(" ");
      Serial.print(val1);
      Serial.print(" ");
      Serial.print(val2);
      Serial.print(" ");
      Serial.println(angle);

      // Draw line
      needle(angle);

      // Write Value
      value(valString);
    }
  }
}