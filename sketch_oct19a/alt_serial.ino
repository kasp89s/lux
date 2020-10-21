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

#include <SoftwareSerial.h>

//#include <AltSoftSerial.h>
//AltSoftSerial SIM800;
SoftwareSerial SIM800(8, 9);
void setup() {
  Serial.begin(9600);
  Serial.println("1111!");
  SIM800.begin(9600);
  SIM800.println("AT");
}

void loop() {
  // Определяем заряд батареи.
  if (SIM800.available()){
     int simResponse = SIM800.read();

     Serial.write(simResponse);
//     Serial.print(simResponse);
  }
  if (Serial.available()) {
      SIM800.write(Serial.read());
  }
  delay(1000);
}
