#include <SoftwareSerial.h>
#include <TinyGPS.h>
SoftwareSerial SIM800(8, 9);
SoftwareSerial GPS(3, 4);
TinyGPS gpsLib;

//Пины управления
byte GPS_PIN = 2;

bool initalized = 0,
     gpsIsFind = 0,
     modemReady = 0, // Готов ли модем
     enableEcho = 1; 

String _response = "",
       OPERATOR = "",
       IMEI; // Переменная для хранения ответа модуля

unsigned int taktPerSec = 60000;

int batteryPower = 0,
    network = 0;

// Эта хуйня делает 60000 цыклов в секунду
long  initalizedCounter = 0,
      checkGPSCounter = 18000000;

// GPS хуйня
    float flat = 0.0, flon = 0.0;
    unsigned long age = 0;
    int sat = 0, hdoop = 0;
// SIM GPS хуйня
    String SIM_GPS;

// Получает данные для сим координат.
void getSimCoordinates()
{
    _response = sendATCommand("AT+CIMI", true, false);
    SIM_GPS = _response.substring(0, 5);
    _response = sendATCommand("AT+CREG?", true, false);
    SIM_GPS += getValue(_response, ',', 2);
    SIM_GPS += getValue(_response, ',', 3);
    SIM_GPS.replace("\"", "");
    Serial.println(SIM_GPS);
}

void updateCounters()
{
   checkGPSCounter++;

    // раз в 5 18000000 минут проверяем данные GPS 
    if (checkGPSCounter >= 3600000) {
       checkGPSCounter = 0;
       updateBatteryPower();
       updateNetwork();
       getSimCoordinates();
       gpsListener();
       String response = "";

       response = sendATCommand("AT+SAPBR=1,1", true, true);
       if (response == "OK") {
           sendATCommand("AT+HTTPINIT", true, false);
           sendATCommand("AT+HTTPPARA=CID,1", true, false);
           //sendATCommand("AT+HTTPSSL=1", true, false);
       }

/**
 *     int MCC;
    String MNC, LAC, CELLID;
 * 
*/
      String url = "AT+HTTPPARA=URL,http://";
          url += "1854993.yz403522.web.hosting-test.net";
          url += "/sim.php?";
          if (gpsIsFind) {
            url += "g=" + String(flat, DEC) + "x" + String(flon, DEC);
            gpsIsFind = false;
          }
          else{
            url += "s=" + SIM_GPS;
          }
          
          url += "&i=" + IMEI;
          url += "&b=" + String(batteryPower, DEC);
          url += "&n=" + String(network, DEC);

       sendATCommand(url, true, false);
       sendATCommand("AT+HTTPACTION=0", true, false);
    }
}

// Инициализация интернета
void initInternet()
{
  //if (OPERATOR == "UMC")
  sendATCommand("AT+SAPBR=3,1,Contype,GPRS", true, false);
  sendATCommand("AT+CSTT=\"internet\",\"\",\"\"", true, false);
}

void setup() {
  delay(5000); // Ждем запуска симки
  pinMode(GPS_PIN, OUTPUT);
  digitalWrite(GPS_PIN, HIGH); // включить GPS
  //analogWrite(A3, 200);
  Serial.begin(9600);
  SIM800.begin(9600);
  GPS.begin(9600);
  SIM800.listen();
  sendATCommand("AT", true, false);
  sendATCommand("ATE0", true, false);
  sendATCommand("AT+CREG=2", true, false);
  IMEI = sendATCommand("AT+GSN", true, false);
  updateBatteryPower();
}

void loop() {
  if (SIM800.available()){
     simRxListener(SIM800.readString());
  }
  if (modemReady) {
    updateCounters();
  } else {
    initalizedCounter++;
    // Ждем 60000 * секунд и говорим что можем готов.
    if (initalizedCounter >= 600000) {
      initalizedCounter = 0;
      if (checkModemReady())
        setModemReady();
    }
  }
}

// Работа с GPS
void gpsListener()
{
  GPS.listen();
  char character;
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  unsigned long whaitingFor = 0,
                lastMillis = 0,
                startGPS = millis();

  // Пока данные не получим слушаем GPS!
  while (!newData && whaitingFor < 10) {
    if (lastMillis < millis() + 1000) {
        lastMillis = millis();
        whaitingFor = (millis() - startGPS) / 1000;

        Serial.println(whaitingFor);
    }
    //_response = "";
    // Секунду слушаем GPS
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while (GPS.available())
      {
        character = GPS.read();
        
        if (gpsLib.encode(character)) // Did a new valid sentence come in?
           newData = true;
        //_response.concat(character);
      }
    }
    //Serial.println(_response);
    if (newData)
    {
      gpsIsFind = true;
      gpsLib.f_get_position(&flat, &flon, &age);
  
      flat = flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
      flon = flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;
      sat = gpsLib.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gpsLib.satellites();
      hdoop = gpsLib.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gpsLib.hdop();
      Serial.print("LAT=");
      Serial.print(flat);
      Serial.print(" LON=");
      Serial.print(flon);
      Serial.print(" SAT=");
      Serial.print(sat);
      Serial.print(" PREC=");
      Serial.print(hdoop);
    }
    gpsLib.stats(&chars, &sentences, &failed);
    Serial.print(" CHARS=");
    Serial.print(chars);
    Serial.print(" SENTENCES=");
    Serial.print(sentences);
    Serial.print(" CSUM ERR=");
    Serial.println(failed);
  }

  //digitalWrite(GPS_PIN, LOW); // выключить GPS
  SIM800.listen();
}

// Слушает чо говорит симка и чето потом делает...
void simRxListener(String response)
{
  response.trim();

  Serial.print(response);

  if (response == "RING")
  {
    sendATCommand("ATH", true, false);
  }

  if (response.lastIndexOf("CUSD")) {
     // Обрабатуем ответ с балансом...
  }
}

//Проверка готовности модема
bool checkModemReady()
{
  network = getValue(sendATCommand("AT+CSQ", true, false), ': ', 1).toInt();

  return network > 0 ? true : false;
}

// Установка готовности модема.
void setModemReady()
{
  updateNetwork();
  updateOperator();
  initInternet();
  modemReady = 1;
}

String sendATCommand(String cmd, bool waiting, bool boolResult) {
  String _resp = "";                            // Переменная для хранения результата
  if (enableEcho)
    Serial.println(cmd);                          // Дублируем команду в монитор порта

  SIM800.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _resp = waitResponse();                     // ... ждем, когда будет передан ответ
    // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
    if (_resp.startsWith(cmd)) {  // Убираем из ответа дублирующуюся команду
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }

     if (enableEcho)
        Serial.println(_resp);                      // Дублируем ответ в монитор порта

     if (!boolResult)
        _resp.replace("OK", "");
  }
  _resp.trim();
  return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                            // Переменная для хранения результата
  long _timeout = millis() + 60000;             // Переменная для отслеживания таймаута (10 секунд)
  while (!SIM800.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (SIM800.available()) {                     // Если есть, что считывать...
    _resp = SIM800.readString();                // ... считываем и запоминаем
  }
  else {                                        // Если пришел таймаут, то...
    Serial.println("Timeout...");               // ... оповещаем об этом и...
  }
  return _resp;                                 // ... возвращаем результат. Пусто, если проблема
}

// Обновляет значения уровня сети.
void updateOperator()
{
    OPERATOR =  getValue(sendATCommand("AT+COPS?", true, false), ',', 2);
    OPERATOR.replace("\"", "");
}

// Обновляет значения уровня сети.
void updateNetwork()
{
    network = getValue(sendATCommand("AT+CSQ", true, false), ': ', 1).toInt();
}

// Обновляет значение вольтажа батареи
void updateBatteryPower()
{
    batteryPower = getValue(sendATCommand("AT+CBC", true, false), ',', 2).toInt();
}

// Вытаскивает значение со строки по разделителю.
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
