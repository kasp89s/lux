#include <SoftwareSerial.h>
#include "U8glib.h"
#include "rus5x7.h"

U8GLIB_SSD1306_128X32 u8g (U8G_I2C_OPT_NONE);

String OPERATOR = "KIEVSTAR",
       displaySring0 = "",
       displaySring1 = "";
int batteryPower = 3800,
    network = 20;
    
bool gprsOk = 0, gpsIsFind = 0;

void setup() {
}

void loop() {
  //drawMainScreen();
  drawLoading();
}

void drawLoading()
{
  u8g.firstPage();
  do {
  u8g.setFont(rus5x7);
  u8g.setPrintPos(30, 10);
  u8g.print("Загрузка...");

  u8g.setFont(rus5x7);
  u8g.setPrintPos(10, 25);
  u8g.print(displaySring0);
  } while (u8g.nextPage());
}

void drawMainScreen()
{
  u8g.firstPage();
  do {
  // ОПЕРАТОР  
  u8g.setFont(rus5x7);
  u8g.setPrintPos(0, 6);
  u8g.print(OPERATOR);

  // БАТАРЕЯ
  u8g.drawFrame(110, 0, 15, 7);
  u8g.drawBox(125,2,2,3);
  int dec = map(batteryPower, 3300, 4150, 1, 13);
  u8g.drawBox(111,1,dec,5);

  // УРОВЕНЬ СИГНАЛА
  if (network > 1)
    u8g.drawLine(95, 4, 95, 6);
  if (network > 8)
    u8g.drawLine(97, 3, 97, 6);
  if (network > 10)
    u8g.drawLine(99, 2, 99, 6);
  if (network > 13)
    u8g.drawLine(101, 1, 101, 6);
  if (network > 16)
    u8g.drawLine(103, 0, 103, 6);
  
  // GPRS
  if (gprsOk) {
    u8g.drawLine(67, 7, 88, 7);
    u8g.setFont(rus5x7);
    u8g.setPrintPos(68, 6);
    u8g.print("GPRS");
  }
  
  // GPS
  if (gpsIsFind) {
    u8g.drawLine(45, 7, 60, 7);
    u8g.setFont(rus5x7);
    u8g.setPrintPos(46, 6);
    u8g.print("GPS");
  }

  // MESSAGE
  // 1 строка 25 символов
  if (displaySring0 != "") {
    u8g.setFont(rus5x7);
    u8g.setPrintPos(0, 19);
    u8g.print(displaySring0);
  }

  // 2 строка 25 символов
  if (displaySring1 != "") {
    u8g.setFont(rus5x7);
    u8g.setPrintPos(0, 32);
    u8g.print(displaySring1);
  }
  } while (u8g.nextPage());
}
