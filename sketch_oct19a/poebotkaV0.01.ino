#include <SoftwareSerial.h>
#include <TinyGPS.h>
SoftwareSerial SIM800(8, 9);
SoftwareSerial GPS(3, 4);
TinyGPS gpsLib;

bool modemReady = 0, // Готов ли модем
     enableEcho = 1; 

String _response = "",
       OPERATOR = "",
       IMEI; // Переменная для хранения ответа модуля

unsigned int taktPerSec = 60000;

int batteryPower = 0,
    network = 0;

// Эта хуйня делает 60000 цыклов в секунду
long ticCounter = 0,
     updateBatteryCounter = 0,
     checkGPSCounter = 0;

// GPS хуйня
    float flat = 0.0, flon = 0.0;
    unsigned long age = 0;
    int sat = 0, hdoop = 0;
    
void updateCounters()
{
   ticCounter++;
   updateBatteryCounter++;
   checkGPSCounter++;
   
    // раз в 60 секунд обновляем данные про батарею
    if (updateBatteryCounter >= 2600000) {
       updateBatteryCounter = 0;
       updateBatteryPower();
       updateNetwork();
       //sendATCommand("AT+CUSD=1,\"*101#\"", true);
    }

    // раз в 30 секунд проверяем данные GPS
    if (checkGPSCounter >= 1800000) {
       checkGPSCounter = 0;
       gpsListener();
       String response = "";
       response = sendATCommand("AT+SAPBR=1,1", true, true);
       if (response == "OK") {
           sendATCommand("AT+HTTPINIT", true, false);
           sendATCommand("AT+HTTPPARA=CID,1", true, false);
            //https!!
           //sendATCommand("AT+HTTPSSL=1", true, false);
       }

String url = "AT+HTTPPARA=URL,http://";
    url += "1854993.yz403522.web.hosting-test.net";
    url += "/sim.php?";
    url += "g=" + String(flat, DEC) + "x" + String(flon, DEC);
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
  sendATCommand("AT+SAPBR=3,1,Contype,GPRS", true, false);
  //if (OPERATOR == "UMC")
  sendATCommand("AT+CSTT=\"internet\",\"\",\"\"", true, false);
}

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(5000); // Ждем запуска симки
  Serial.begin(9600);
  SIM800.begin(9600);
  GPS.begin(9600);
  SIM800.listen();
  sendATCommand("AT", true, false);
  sendATCommand("ATE0", true, false);
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
//    updateNetwork();
//    Serial.println(network);
//    delay(1000);
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
  
  // Секунду слушаем GPS
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (GPS.available())
    {
      character = GPS.read();
      
      if (gpsLib.encode(character)) // Did a new valid sentence come in?
         newData = true;
      //Serial.write(character);
      
      //_response.concat(character);
    }
  }
  if (newData)
  {
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
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");  
  //Serial.println(_response);
  //_response = "";
  SIM800.listen();
}

// Слушает чо говорит симка и чето потом делает...
void simRxListener(String response)
{
  response.trim();

  Serial.println(ticCounter);
  Serial.print("Modem response => [");
  Serial.print(response);
  Serial.println("]");

  if (response == "SMS Ready") {
        updateOperator();
        initInternet();
        modemReady = 1;
  }
  if (response == "RING")
  {
    sendATCommand("ATH", true, false);
  }

  if (response.lastIndexOf("CUSD")) {
     // Обрабатуем ответ с балансом...
  }
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

void resetModem() {
  // reset modem
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(115);
  //digitalWrite(reset_sim800_pin, HIGH);
  pinMode(2, INPUT);
  digitalWrite(2, LOW);
}
