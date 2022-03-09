// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Bluetooth callback
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    Serial.println("Client Connected");
    btConnected = true;
  }

  if (event == ESP_SPP_CLOSE_EVT)
  {
    Serial.println("Client disconnected");
    btConnected = false;
  }
}

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
void needle(float_t angle, uint16_t a = 0, uint16_t b = 200, uint16_t c = 0, uint16_t d = 110)
{
  static float angleOld;
  uint16_t x, y;

  if (angle != angleOld)
  {
    angleOld = angle;

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

    M5.Lcd.drawJpg(smeterTop, sizeof(smeterTop), 0, 0, 320, 150);

    // M5.Lcd.drawFastHLine(0, 150, 320, TFT_BLACK);

    M5.Lcd.drawLine(a + 2, b, c + 2, d, TFT_NEDDLE_2);
    M5.Lcd.drawLine(a + 1, b, c + 1, d, TFT_NEDDLE_1);
    M5.Lcd.drawLine(a, b, c, d, TFT_RED);
    M5.Lcd.drawLine(a - 1, b, c - 1, d, TFT_NEDDLE_1);
    M5.Lcd.drawLine(a - 2, b, c - 2, d, TFT_NEDDLE_2);
  }
}

// Print value
void value(String valString, uint8_t x = 160, uint8_t y = 180)
{
  static String valStringOld;

  if (valString != valStringOld)
  {
    valStringOld = valString;

    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFreeFont(&stencilie16pt7b);
    // M5.Lcd.setFreeFont(&YELLOWCRE8pt7b);
    M5.Lcd.setTextPadding(190);
    M5.Lcd.setTextColor(TFT_BLACK, TFT_BACK);
    valString.replace(".", ",");
    M5.Lcd.drawString(valString, x, y);
  }
}

// Print sub value
void subValue(String valString, uint8_t x = 160, uint8_t y = 205)
{
  static String valStringOld;

  if (valString != valStringOld)
  {
    valStringOld = valString;

    M5.Lcd.setTextDatum(CC_DATUM);
    // M5.Lcd.setFreeFont(&stencilie16pt7b);
    M5.Lcd.setFreeFont(&YELLOWCRE8pt7b);
    M5.Lcd.setTextPadding(140);
    M5.Lcd.setTextColor(TFT_BLACK, TFT_BACK);
    // valString.replace(".", ",");
    M5.Lcd.drawString(valString, x, y);
  }
}

// Print option
void viewMenu()
{
  uint16_t i = 65;
  uint8_t j;

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFreeFont(&YELLOWCRE8pt7b);
  M5.Lcd.setTextPadding(0);

  for (j = 0; j <= 2; j++)
  {
    if (option == j)
    {
      M5.Lcd.setTextColor(TFT_BLACK);
    }
    else
    {
      M5.Lcd.setTextColor(TFT_DARKGREY);
    }

    M5.Lcd.drawString(menu[j], i, 230);
    i += 95;
  }
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

// Send CI-V Command
void sendCommand(char *request, size_t n, char *buffer, uint8_t limit)
{
  uint8_t byte1, byte2, byte3;
  uint8_t counter = 0;

  while (counter != limit)
  {
    for (uint8_t i = 0; i < n; i++)
    {
      CAT.write(request[i]);
    }

    vTaskDelay(100);

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
          if (counter > limit)
          {
            Serial.print(" Overflow");
            break;
          }
        }
      }
    }
    vTaskDelay(10);
  }
  Serial.println(" Ok");
}

// Get Smeter
void getSmeter()
{
  String valString;

  char str[2];
  char buffer[6];
  char request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x15, 0x02, 0xFD};

  uint8_t val0 = 0;
  float_t val1 = 0;
  float_t val2 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  Serial.print("Get S");
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
    /*
    Serial.print(val0);
    Serial.print(" ");
    Serial.print(val1);
    Serial.print(" ");
    Serial.print(val2);
    Serial.print(" ");
    Serial.println(angle);
    */

    // Draw line
    needle(angle);

    // Write Value
    value(valString);
  }
}

// Get SWR
void getSWR()
{
  String valString;

  char str[2];
  char buffer[6];
  char request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x15, 0x12, 0xFD};

  uint8_t val0 = 0;
  float_t val1 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  Serial.print("Get SWR");
  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  val0 = atoi(str);

  if (val0 != val3 || reset == true)
  {
    val3 = val0;
    reset = false;

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

// Get Power
void getPower()
{
  String valString;

  char str[2];
  char buffer[6];
  char request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x15, 0x11, 0xFD};

  uint8_t val0 = 0;
  float_t val1 = 0;
  float_t val2 = 0;
  static uint8_t val3 = 0;

  float_t angle = 0;

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  Serial.print("Get PWR");
  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  val0 = atoi(str);

  if (val0 != val3 || reset == true)
  {
    val3 = val0;
    reset = false;

    if (val0 <= 27)
    {
      angle = mapFloat(val0, 0, 27, 42.0f, 30.50f);
      val1 = mapFloat(val0, 0, 27, 0, 0.5);
    }
    else if (val0 <= 49)
    {
      angle = mapFloat(val0, 28, 49, 30.50f, 23.50f);
      val1 = mapFloat(val0, 28, 49, 0.5, 1.0);
    }
    else if (val0 <= 78)
    {
      angle = mapFloat(val0, 50, 78, 23.50f, 14.50f);
      val1 = mapFloat(val0, 50, 78, 1.0, 2.0);
    }
    else if (val0 <= 104)
    {
      angle = mapFloat(val0, 79, 104, 14.50f, 6.30f);
      val1 = mapFloat(val0, 79, 104, 2.0, 3.0);
    }
    else if (val0 <= 143)
    {
      angle = mapFloat(val0, 105, 143, 6.30f, -6.50f);
      val1 = mapFloat(val0, 105, 143, 3.0, 5.0);
    }
    else if (val0 <= 175)
    {
      angle = mapFloat(val0, 144, 175, -6.50f, -17.50f);
      val1 = mapFloat(val0, 144, 175, 5.0, 7.0);
    }
    else
    {
      angle = mapFloat(val0, 176, 226, -17.50f, -30.50f);
      val1 = mapFloat(val0, 176, 226, 7.0, 10.0);
    }

    val2 = round(val1 * 10);
    valString = "PWR " + String((val2 / 10)) + " W";

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

// Get Data Mode
void getDataMode()
{
  char buffer[6];
  char request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x1A, 0x06, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  Serial.print("Get Data");

  /*
  for(int i = 0; i < 6; i++) {
    Serial.println(int(buffer[i]));
  }
  */

  dataMode = buffer[4];
}

// Get Frequency
void getFrequency()
{
  String valString;

  char buffer[8];
  char request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x03, 0xFD};

  String val0;
  String val1;
  String val2;

  uint32_t frequency; // Current frequency in Hz
  const uint32_t decMulti[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

  uint8_t lenght = 0;

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 8);

  Serial.print("Get frequency");
  frequency = 0;
  for (uint8_t i = 2; i < 7; i++)
  {
    frequency += (buffer[9 - i] >> 4) * decMulti[(i - 2) * 2];
    frequency += (buffer[9 - i] & 0x0F) * decMulti[(i - 2) * 2 + 1];
  }

  valString = String(frequency);
  lenght = valString.length();
  val0 = valString.substring(lenght - 3, lenght);
  val1 = valString.substring(lenght - 6, lenght - 3);
  val2 = valString.substring(0, lenght - 6);

  subValue(val2 + "." + val1 + "." + val0);
}

// Get Mode
void getMode()
{
  String valString;
  static String modeOld;
  static String filterOld;

  char buffer[5];
  char request[] = {0xFE, 0xFE, IC705_CI_V_ADDRESS, 0xE0, 0x04, 0xFD};

  const char *mode[] = {"LSB", "USB", "AM", "CW", "RTTY", "FM", "WFM", "CW-R", "RTTY-R", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "DV"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  Serial.print("Get Mode");

  M5.Lcd.setFreeFont(0);
  M5.Lcd.setTextPadding(24);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextDatum(CC_DATUM);

  valString = "FIL" + String(uint8_t(buffer[4]));
  if (valString != filterOld)
  {
    filterOld = valString;
    M5.Lcd.fillRoundRect(44, 199, 44, 13, 2, TFT_MODE);
    M5.Lcd.drawString(valString, 66, 206);
  }

  valString = String(mode[buffer[3]]);

  getDataMode(); // Data ON or OFF ?

  if (dataMode == 1)
  {
    valString += "-D";
  }
  if (valString != modeOld)
  {
    modeOld = valString;
    M5.Lcd.fillRoundRect(232, 199, 44, 13, 2, TFT_MODE);
    M5.Lcd.drawString(valString, 254, 206);
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

    if (btnA == 1)
    {
      val0 -= 1;
      btnA = 0;
    }
    if (btnC == 1)
    {
      val0 += 1;
      btnC = 0;
    }

    vTaskDelay(100);

    if (val0 != val3 || reset == true)
    {
      val3 = val0;
      reset = false;

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
      valString = "PWR " + String((val2 / 10)) + " W";

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
}

bool M5Screen24bmp()
{
  uint16_t image_height = M5.Lcd.height();
  uint16_t image_width = M5.Lcd.width();
  const uint16_t pad = (4 - (3 * image_width) % 4) % 4;
  uint32_t filesize = 54 + (3 * image_width + pad) * image_height;
  unsigned char swap;
  unsigned char line_data[image_width * 3 + pad];
  unsigned char header[54] = {
      'B', 'M',    // BMP signature (Windows 3.1x, 95, NT, …)
      0, 0, 0, 0,  // Image file size in bytes
      0, 0, 0, 0,  // Reserved
      54, 0, 0, 0, // Start of pixel array
      40, 0, 0, 0, // Info header size
      0, 0, 0, 0,  // Image width
      0, 0, 0, 0,  // Image height
      1, 0,        // Number of color planes
      24, 0,       // Bits per pixel
      0, 0, 0, 0,  // Compression
      0, 0, 0, 0,  // Image size (can be 0 for uncompressed images)
      0, 0, 0, 0,  // Horizontal resolution (dpm)
      0, 0, 0, 0,  // Vertical resolution (dpm)
      0, 0, 0, 0,  // Colors in color table (0 = none)
      0, 0, 0, 0}; // Important color count (0 = all colors are important)

  // Fill filesize, width and heigth in the header array
  for (uint8_t i = 0; i < 4; i++)
  {
    header[2 + i] = (char)((filesize >> (8 * i)) & 255);
    header[18 + i] = (char)((image_width >> (8 * i)) & 255);
    header[22 + i] = (char)((image_height >> (8 * i)) & 255);
  }
  // Write the header to the file
  httpClient.write(header, 54);

  // To keep the required memory low, the image is captured line by line
  // initialize padded pixel with 0
  for (uint16_t i = (image_width - 1) * 3; i < (image_width * 3 + pad); i++)
  {
    line_data[i] = 0;
  }
  // The coordinate origin of a BMP image is at the bottom left.
  // Therefore, the image must be read from bottom to top.
  for (uint16_t y = image_height; y > 0; y--)
  {
    // Get one line of the screen content
    M5.Lcd.readRectRGB(0, y - 1, image_width, 1, line_data);
    // BMP color order is: Blue, Green, Red
    // Return values from readRectRGB is: Red, Green, Blue
    // Therefore: R und B need to be swapped
    for (uint16_t x = 0; x < image_width; x++)
    {
      swap = line_data[x * 3];
      line_data[x * 3] = line_data[x * 3 + 2];
      line_data[x * 3 + 2] = swap;
    }
    // Write the line to the file
    httpClient.write(line_data, (image_width * 3) + pad);
  }
  return true;
}

// Get screenshot
void getScreenshot()
{
  unsigned long timeout_millis = millis() + 3000;
  String currentLine = "";

  httpClient = httpServer.available();

  if (WiFi.status() == WL_CONNECTED)
  {
    // httpClient.setNoDelay(1);
    if (httpClient)
    {
      // Force a disconnect after 3 seconds
      // Serial.println("New Client.");
      // Loop while the client's connected
      while (httpClient.connected())
      {
        // If the client is still connected after 3 seconds,
        // Something is wrong. So kill the connection
        if (millis() > timeout_millis)
        {
          // Serial.println("Force Client stop!");
          httpClient.stop();
        }
        // If there's bytes to read from the client,
        if (httpClient.available())
        {
          char c = httpClient.read();
          Serial.write(c);
          // If the byte is a newline character
          if (c == '\n')
          {
            // Two newline characters in a row (empty line) are indicating
            // The end of the client HTTP request, so send a response:
            if (currentLine.length() == 0)
            {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line,
              // followed by the content:
              switch (htmlGetRequest)
              {
              case GET_index_page:
              {
                httpClient.println("HTTP/1.1 200 OK");
                httpClient.println("Content-type:text/html");
                httpClient.println();
                httpClient.write_P(index_html, sizeof(index_html));
                break;
              }
              case GET_screenshot:
              {
                httpClient.println("HTTP/1.1 200 OK");
                httpClient.println("Content-type:image/bmp");
                httpClient.println();
                M5Screen24bmp();
                break;
              }
              default:
                httpClient.println("HTTP/1.1 404 Not Found");
                httpClient.println("Content-type:text/html");
                httpClient.println();
                httpClient.print("404 Page not found.<br>");
                break;
              }
              // The HTTP response ends with another blank line:
              // httpClient.println();
              // Break out of the while loop:
              break;
            }
            else
            { // if a newline is found
              // Analyze the currentLine:
              // detect the specific GET requests:
              if (currentLine.startsWith("GET /"))
              {
                htmlGetRequest = GET_unknown;
                // If no specific target is requested
                if (currentLine.startsWith("GET / "))
                {
                  htmlGetRefresh = 3;
                  htmlGetRequest = GET_index_page;
                }
                // If the screenshot image is requested
                if (currentLine.startsWith("GET /screenshot.bmp"))
                {
                  htmlGetRefresh = 3;
                  htmlGetRequest = GET_screenshot;
                }
                // If the button left was pressed on the HTML page
                if (currentLine.startsWith("GET /buttonLeft"))
                {
                  buttonLeftPressed = true;
                  htmlGetRefresh = 1;
                  htmlGetRequest = GET_index_page;
                }
                // If the button center was pressed on the HTML page
                if (currentLine.startsWith("GET /buttonCenter"))
                {
                  buttonCenterPressed = true;
                  htmlGetRefresh = 1;
                  htmlGetRequest = GET_index_page;
                }
                // If the button right was pressed on the HTML page
                if (currentLine.startsWith("GET /buttonRight"))
                {
                  buttonRightPressed = true;
                  htmlGetRefresh = 1;
                  htmlGetRequest = GET_index_page;
                }
              }
              currentLine = "";
            }
          }
          else if (c != '\r')
          {
            // Add anything else than a carriage return
            // character to the currentLine
            currentLine += c;
          }
        }
      }
      // Close the connection
      httpClient.stop();
      // Serial.println("Client Disconnected.");
    }
  }
}