#include <SoftwareSerial.h>
#include "U8glib.h"

U8GLIB_SSD1306_128X32 u8g (U8G_I2C_OPT_NONE);
SoftwareSerial SIM800(8, 9);

int batteryVoltagePin = 0,
    chargeBatteryPin = 1,
    lastBatteryVoltage = 0,
    // Напряжение питания ардуино.
    ACIN_Voltage = 4680;

void setup() {
  Serial.begin(9600);
  pinMode(batteryVoltagePin, INPUT);
  Serial.println("Start");
  SIM800.begin(9600);
  SIM800.println("AT");
}

void loop() {
  // Определяем заряд батареи.
  if (SIM800.available())
    Serial.write(SIM800.read());
  if (Serial.available())
    SIM800.write(Serial.read());
    
  int batteryPinValue = analogRead(batteryVoltagePin),
      // Напряжение на батарее.
      batteryVoltageValue = roundVoltage(map(batteryPinValue, 0, 1024, 0, ACIN_Voltage)),
      // Значение в процентах.
      pesentOfBattery = map(batteryVoltageValue, 3500, 4200, 0, 100);

  u8g.firstPage();
  do {
  u8g.setFont(u8g_font_unifontr);
  u8g.setPrintPos(0, 12);
  u8g.print("BATTERY: ");
  u8g.print(pesentOfBattery);
  u8g.print("%");
  } while (u8g.nextPage());
  
    //draw(batteryVoltageValue);
  // Serial.println(batteryVoltageValue);
  //Serial.println(batteryVoltageValue);
  delay(1000);

}

int roundVoltage(int voltage)
{
   // Если напряжение скачит на 50 миливольт от предыдущего то пошло оно нахуй!
   byte coefficient = 50;

   if (lastBatteryVoltage == 0) {
       lastBatteryVoltage = voltage;
       return lastBatteryVoltage;
   }

   unsigned int defuse = abs(lastBatteryVoltage - voltage);
   if (defuse > coefficient && defuse < 4200)
      lastBatteryVoltage = voltage;

   return lastBatteryVoltage;
}
