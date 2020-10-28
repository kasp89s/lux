#include <SoftwareSerial.h>
#include "U8glib.h"

U8GLIB_SSD1306_128X32 u8g (U8G_I2C_OPT_NONE);

int number = 0;
void setup() {
}

void loop() {
  number++;

  drawOn();

  delay(1000);
}

void drawOn()
{
  u8g.firstPage();
  do {
    
  u8g.setFont(u8g_font_unifontr);
  u8g.setPrintPos(0, 10);
  u8g.print("Now is: ");
  u8g.print(number);

  u8g.setPrintPos(10, 10);
  u8g.print("q");
  } while (u8g.nextPage());
}

void drawLoading()
{
  u8g.firstPage();
  do {
  u8g.setFont(u8g_font_unifontr);
  u8g.setPrintPos(20, 20);
  u8g.print("LOADING...");
  } while (u8g.nextPage());
}
