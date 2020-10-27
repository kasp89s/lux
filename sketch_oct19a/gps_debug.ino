#include <TinyGPS.h>
#include <SoftwareSerial.h>

SoftwareSerial GPS(8, 9);
TinyGPS gpsLib;

void setup() {
  Serial.begin(9600);
  GPS.begin(9600);
  Serial.println("start!");
  //SIM800.begin(9600);
  //SIM800.println("AT");
  GPS.listen();
}

void loop() {
  runGPS();
}

void runGPS()
{
    bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (GPS.available())
    {
      char c = GPS.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gpsLib.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gpsLib.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gpsLib.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gpsLib.satellites());
    Serial.print(" PREC=");
    Serial.print(gpsLib.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gpsLib.hdop());
  }
  
  gpsLib.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");  
}
