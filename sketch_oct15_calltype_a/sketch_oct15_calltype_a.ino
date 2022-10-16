#include <U8glib.h>
#include <SoftwareSerial.h>


SoftwareSerial SIM800(8, 9);
U8GLIB_SSD1306_128X64 u8g (U8G_I2C_OPT_NONE);

const char   OK[] = "OK",
             ERROR[] = "Error",
             SET_NUMBER[] = "Set phone number:",
             RUN_ON[] = "Run ON target:",
             AT[] = "AT",
             AT_ATE[] = "ATE0",
             AT_COPS[] = "AT+COPS?",
             AT_CSQ[] = "AT+CSQ",
             AT_CLCC[] = "AT+CLCC",
             AT_DOWN[] = "ATH";
     
bool initalized = 0,
     switchButton = 0,
     switchButtonFlag = 0,
     applyButton = 0,
     applyButtonFlag = 0,
     modemReady = 0, // Готов ли модем
     call = 0,
     enableEcho = 1; 

String _response = "",
        targetPhone= "380",
        OPERATOR = ""; // Переменная для хранения ответа модуля

int callType = 0,
    network = 0,
    batteryPower = 4150,
    currentNum = 0;

void setPhoneNumber(){
    while(!initalized) {
       switchButton = !digitalRead(2);
       applyButton = !digitalRead(3);

      // Switch press
      if (switchButton == 1 && switchButtonFlag == 0) {
          switchButtonFlag = 1;

          if (currentNum < 9) {
             currentNum++;
          } else {
            currentNum = 0;
          }

          printDisplay();
      }
      // Switch relase
      if (switchButton == 0 && switchButtonFlag == 1) {
          switchButtonFlag = 0;
      }

      // Switch press
      if (applyButton == 1 && applyButtonFlag == 0) {
          applyButtonFlag = 1;

          targetPhone = targetPhone + currentNum;
          currentNum = 0;
          
          printDisplay();
          if (targetPhone.length() == 12) {
             initalized = 1;
          }
      }
      // Switch relase
      if (applyButton == 0 && applyButtonFlag == 1) {
          applyButtonFlag = 0;
      }
      
       delay(50);
    }
}

void setCallType() {
    initalized = 0;

    while(!initalized) {
       switchButton = !digitalRead(2);
       applyButton = !digitalRead(3);

             // Switch press
      if (switchButton == 1 && switchButtonFlag == 0) {
          switchButtonFlag = 1;

          if (callType < 2) {
             callType++;
          } else {
            callType = 0;
          }

          printTypeDisplay();
      }
      // Switch relase
      if (switchButton == 0 && switchButtonFlag == 1) {
          switchButtonFlag = 0;
      }

      // Switch press
      if (applyButton == 1 && applyButtonFlag == 0) {
          applyButtonFlag = 1;

          initalized = 1;
      }
      // Switch relase
      if (applyButton == 0 && applyButtonFlag == 1) {
          applyButtonFlag = 0;
      }
      
       delay(50);
    }
}

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  printDisplay();
  setPhoneNumber();
  printTypeDisplay();
  setCallType();

  printDisplay();

  SIM800.begin(9600);
  SIM800.listen();
  sendATCommand(AT, true, false);
  sendATCommand(AT_ATE, true, false);
  updateBatteryPower();
}

void loop() {
  if (modemReady) {
    updateBatteryPower();
    sendATCommand("ATD+" + targetPhone + ";", true, false);
    call = 1;

    _response = "CALL";
    printDisplay();
    while (call) {
       if (SIM800.available()) {                     // Если есть, что считывать...
            _response = SIM800.readString();                // ... считываем и запоминаем
            Serial.println("Event");
            Serial.println(_response);
            call = 0;
       }
       
       int status = getValue(sendATCommand(AT_CLCC, true, false), ',', 2).toInt();

       if (callType == 0) {
          if (status == 0 || status == 3) {
            sendATCommand(AT_DOWN, true, false);
            call = 0;
          }
       }

       if (callType == 1) {
          if (status == 0) {
              sendATCommand(AT_DOWN, true, false);
              call = 0;
          }
       }

       if (callType == 2) {
          // @todo запустить звук
       }

       printDisplay();
       delay(500);
    };

  } else {
    delay(10000);
    updateNetwork();
    if (checkModemReady())
        setModemReady();
  }
}

void printTypeDisplay()
{
  u8g.firstPage();
  do {
        String SET = "SET: ";

        u8g.setFont(u8g_font_5x7);
        u8g.setPrintPos(20, 10);
        u8g.print("Set call type");

        u8g.setPrintPos(10, 25);
        u8g.print("0:quick; 1:off; 2:play");

        u8g.setPrintPos(10, 40);
        u8g.print(SET + callType);
  } while (u8g.nextPage());
}

void printDisplay()
{
  u8g.firstPage();
  do {
//  u8g.setFont(u8g_font_u8glib_4);
  u8g.setFont(u8g_font_5x7);
     if (!initalized) {
        u8g.setPrintPos(20, 10);
        u8g.print(SET_NUMBER);

        u8g.setPrintPos(10, 25);
        u8g.print(targetPhone + currentNum);
    } else {
        // БАТАРЕЯ
        if (batteryPower < 0)
            batteryPower = 4150;
        
        u8g.drawFrame(110, 0, 15, 7);
        u8g.drawBox(125,2,2,3);
        int dec = map(batteryPower, 3300, 4150, 1, 13);
  
        u8g.drawBox(111,1,dec,5);
      
        u8g.setPrintPos(20, 10);
        u8g.print(RUN_ON);

        u8g.setPrintPos(10, 25);
        u8g.print(targetPhone);

        u8g.setPrintPos(10, 40);
        u8g.print(_response);
    }
  } while (u8g.nextPage());
}

//Проверка готовности модема
bool checkModemReady()
{
  return network > 0 ? true : false;
}

// Установка готовности модема.
void setModemReady()
{
  updateNetwork();
  updateOperator();
  modemReady = 1;
}

String sendATCommand(String cmd, bool waiting, bool boolResult) {
  if (enableEcho)
    Serial.println(cmd);                          // Дублируем команду в монитор порта

  SIM800.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _response = waitResponse();                     // ... ждем, когда будет передан ответ
     if (enableEcho)
        Serial.println(_response);                      // Дублируем ответ в монитор порта

     if (!boolResult)
        _response.replace(OK, "");
  }
  _response.trim();

  printDisplay();
  return _response;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _response = "";                            // Переменная для хранения результата
  long _timeout = millis() + 60000;             // Переменная для отслеживания таймаута (10 секунд)
  while (!SIM800.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (SIM800.available()) {                     // Если есть, что считывать...
    _response = SIM800.readString();                // ... считываем и запоминаем
  }
  
  return _response;                                 // ... возвращаем результат. Пусто, если проблема
}

// Обновляет значения уровня сети.
void updateOperator()
{
    OPERATOR =  getValue(sendATCommand(AT_COPS, true, false), ',', 2);
}

// Обновляет значения уровня сети.
void updateNetwork()
{
    network = getValue(sendATCommand(AT_CSQ, true, false), ': ', 1).toInt();
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
