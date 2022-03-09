// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Get Button
void button(void *pvParameters)
{
  uint8_t btnA;
  uint8_t btnB;
  uint8_t btnC;

  for (;;)
  {

    M5.update();

    btnA = M5.BtnA.read();
    btnB = M5.BtnB.read();
    btnC = M5.BtnC.read();

    if (btnA == 1 || buttonLeftPressed == 1)
    {
      option = 0;
      reset = true;
      buttonLeftPressed = 0;
      preferences.putUInt("option", option);
    }
    else if (btnB == 1 || buttonCenterPressed == 1)
    {
      option = 1;
      reset = true;
      buttonCenterPressed = 0;
      preferences.putUInt("option", option);
    }
    else if (btnC == 1 || buttonRightPressed == 1)
    {
      option = 2;
      reset = true;
      buttonRightPressed = 0;
      preferences.putUInt("option", option);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}