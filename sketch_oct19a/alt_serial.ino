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

String _response = "";                      // Переменная для хранения ответа модуля
void setup() {
  Serial.begin(9600);
  SIM800.begin(9600);
  endATCommand("AT", true);                // Автонастройка скорости
do {
    _response = sendATCommand("AT+CLIP=1", true);  // Включаем АОН
    _response.trim();                       // Убираем пробельные символы в начале и конце
  } while (_response != "OK");              // Не пускать дальше, пока модем не вернет ОК

  Serial.println("CLI enabled");            // Информируем, что АОН включен
  // Проверка пройдена, модем сообщил о готовности, можно запускать основной цикл...
  // ...
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

String sendATCommand(String cmd, bool waiting) {
  String _resp = "";                            // Переменная для хранения результата
  Serial.println(cmd);                          // Дублируем команду в монитор порта
  SIM800.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _resp = waitResponse();                     // ... ждем, когда будет передан ответ
    // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
    if (_resp.startsWith(cmd)) {  // Убираем из ответа дублирующуюся команду
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }
    Serial.println(_resp);                      // Дублируем ответ в монитор порта
  }
  return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                            // Переменная для хранения результата
  long _timeout = millis() + 10000;             // Переменная для отслеживания таймаута (10 секунд)
  while (!SIM800.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (SIM800.available()) {                     // Если есть, что считывать...
    _resp = SIM800.readString();                // ... считываем и запоминаем
  }
  else {                                        // Если пришел таймаут, то...
    Serial.println("Timeout...");               // ... оповещаем об этом и...
  }
  return _resp;                                 // ... возвращаем результат. Пусто, если проблема
}
