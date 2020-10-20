#include <AltSoftSerial.h>
AltSoftSerial SIM800;

void setup() {
  Serial.begin(9600);
  Serial.println("1111!");
  SIM800.begin(9600);
  SIM800.println("AT");
}

void loop() {
  // Определяем заряд батареи.
  if (SIM800.available()){
     Serial.write(SIM800.read());
  }
  if (Serial.available()) {
      SIM800.write(Serial.read());
  }
}
