#include <SoftwareSerial.h>
#include "TinyGPS++.h"
#include "U8glib.h"
#include "rus5x7.h"

U8GLIB_SSD1306_128X32 u8g (U8G_I2C_OPT_NONE);
SoftwareSerial SIM800(8, 9);
SoftwareSerial GPS(3, 4);
TinyGPSPlus gpsPP;
//Пины управления
//byte GPS_PIN = 2;
const String HTTP_URL = "AT+HTTPPARA=URL,http://1854993.yz403522.web.hosting-test.net/sim.php?",
             OK = "OK",
             ERROR = "Error",
             AT = "AT",
             AT_CIMI = "AT+CIMI",
             AT_CREG = "AT+CREG?",
             AT_SAPBR = "AT+SAPBR=1,1",
             AT_HTTPINIT = "AT+HTTPINIT",
             AT_HTTPPARA = "AT+HTTPPARA=CID,1",
             AT_HTTPACTION = "AT+HTTPACTION=0",
             AT_SET_GPRS = "AT+SAPBR=3,1,Contype,GPRS",
             AT_CONFIG_INTERNET = "AT+CSTT=\"internet\",\"\",\"\"",
             AT_ATE = "ATE0",
             AT_CREG_SET = "AT+CREG=2",
             AT_GSMBUSY = "AT+GSMBUSY=1",
             AT_GSN = "AT+GSN";
bool initalized = 0,
     gpsIsFind = 0,
   gprsOk = 0,
     modemReady = 0, // Готов ли модем
     enableEcho = 1; 

String _response = "",
       OPERATOR = "",
     displaySring0 = "",
       displaySring1 = "",
       IMEI; // Переменная для хранения ответа модуля

const unsigned int taktPerSec = 60000;

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
    _response = sendATCommand(AT_CIMI, true, false);
    SIM_GPS = _response.substring(0, 5);
    _response = sendATCommand(AT_CREG, true, false);
    SIM_GPS += getValue(_response, ',', 2);
    SIM_GPS += getValue(_response, ',', 3);
    SIM_GPS.replace("\"", "");
    Serial.println(SIM_GPS);
}

void updateCounters()
{
   delay(30000);
   gpsListener();
   syncWithServer();
}

void syncWithServer()
{       
    updateBatteryPower();
    updateNetwork();
    getSimCoordinates();
     if (gprsOk == 0) {
      _response = sendATCommand(AT_SAPBR, true, true);
      if (_response == OK) {
          gprsOk = 1;
          sendATCommand(AT_HTTPINIT, true, false);
          sendATCommand(AT_HTTPPARA, true, false);
         //sendATCommand("AT+HTTPSSL=1", true, false);
      }
     }
        String url = HTTP_URL;
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

       _response = sendATCommand(url, true, true);
     if (_response != OK) {
       displaySring0 = ERROR;
       displaySring1 = 'Send HTTP_URL';
     }
       _response = sendATCommand(AT_HTTPACTION, true, true);
      if (_response != OK) {
       displaySring0 = ERROR;
       displaySring1 = 'Send AT_HTTPACTION';
     }
}

// Инициализация интернета
void initInternet()
{
  //if (OPERATOR == "UMC")
  sendATCommand(AT_SET_GPRS, true, false);
  sendATCommand(AT_CONFIG_INTERNET, true, false);
}

void setup() {
  drawLoading();
  Serial.begin(9600);
  SIM800.begin(9600);
  GPS.begin(9600);
  SIM800.listen();
  sendATCommand(AT, true, false);
  sendATCommand(AT_ATE, true, false);
  sendATCommand(AT_CREG_SET, true, false);
  sendATCommand(AT_GSMBUSY, true, false);
  IMEI = sendATCommand(AT_GSN, true, false);
  updateBatteryPower();
}

void loop() {
  if (modemReady) {
  drawMainScreen();
    updateCounters();
  } else {
  drawLoading();
    delay(10000);
    updateNetwork();
    if (checkModemReady())
        setModemReady();
  }
}

// Работа с GPS
void gpsListener()
{
  GPS.listen();
  char character;

  unsigned long whaitingFor = 0,
                lastMillis = 0,
                startGPS = millis();

    //_response = "";
    // Секунду слушаем GPS
    while (!gpsIsFind) {
        if (lastMillis < millis() + 1000) {
            lastMillis = millis();
            whaitingFor = (millis() - startGPS) / 1000;
        }
        if (whaitingFor > 0 && whaitingFor % 60 == 0) {
            SIM800.listen();
            syncWithServer();
            GPS.listen();
        }
        while (GPS.available() > 0)
        {
         character = GPS.read();
         Serial.write(character);
         gpsPP.encode(character);
            if (gpsPP.altitude.isUpdated()) {
              flat = gpsPP.location.lat();
              flon = gpsPP.location.lng();
              
              gpsIsFind = true;
           }
        }
        //_response.concat(character);
    }
  //digitalWrite(GPS_PIN, LOW); // выключить GPS
  SIM800.listen();
}

//Проверка готовности модема
bool checkModemReady()
{
  return network > 0 && network != 30 && network != 31 ? true : false;
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

// Экран загрузки
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

// Экран главный
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
