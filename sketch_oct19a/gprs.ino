#include &lt;SoftwareSerial.h&gt;
//#include &lt;Wire.h&gt;
SoftwareSerial mySerial(9, 8); // RX, TX
String apn =&quot;MTS Internet.ru&quot;; //APN
String apn_u = &quot;mts&quot;; //APN-Username
String apn_p = &quot;mts&quot;; //APN-Password
String url = &quot;http://arduino.site/meteo/index.php&quot;; //URL
for HTTP-POST-REQUEST
//Здесь вставьте адрес своего сайта!!!
String data1; //String for the first Paramter (e.g. Sensor1)
String data2; //String for the second Paramter (e.g.
Sensor2)

void setup() {
// Open serial communications and wait for port to open:
Serial.begin(9600);
// set the data rate for the SoftwareSerial port
mySerial.begin(9600);
delay(10000);
}
void loop() { // цикл

data1 = &quot;111&quot;; // данные с датчиков
data2 = &quot;444&quot;;
gsm_sendhttp(); // запуск
delay(60000); //Wait one minute
}
void gsm_sendhttp() {
mySerial.println(&quot;AT&quot;);
runsl();//Print GSM Status an the Serial Output;

delay(4000);
mySerial.println(&quot;AT+SAPBR=3,1,Contype,GPRS&quot;);
runsl();
delay(100);
mySerial.println(&quot;AT+SAPBR=3,1,APN,&quot; + apn);
runsl();
delay(100);
mySerial.println(&quot;AT+SAPBR=3,1,USER,&quot; + apn_u);
//Comment out, if you need username
runsl();
delay(100);
mySerial.println(&quot;AT+SAPBR=3,1,PWD,&quot; + apn_p);
//Comment out, if you need password
runsl();
delay(100);
mySerial.println(&quot;AT+SAPBR =1,1&quot;);
runsl();
delay(100);
mySerial.println(&quot;AT+SAPBR=2,1&quot;);
runsl();
delay(2000);
mySerial.println(&quot;AT+HTTPINIT&quot;);
runsl();
delay(100);
mySerial.println(&quot;AT+HTTPPARA=CID,1&quot;);
runsl();
delay(100);
mySerial.println(&quot;AT+HTTPPARA=URL,&quot; + url);
runsl();
delay(100);
mySerial.println(&quot;AT+HTTPPARA=CONTENT,application/
x-www-form-urlencoded&quot;);
runsl();
delay(100);
mySerial.println(&quot;AT+HTTPDATA=192,10000&quot;);
runsl();
delay(100);

mySerial.println(&quot;params=&quot; + data1 + &quot;~&quot; + data2);
runsl();
delay(10000);
mySerial.println(&quot;AT+HTTPACTION=1&quot;);
runsl();
delay(50000);
mySerial.println(&quot;AT+HTTPREAD&quot;);
runsl();
delay(100);
mySerial.println(&quot;AT+HTTPTERM&quot;);
runsl();
}
//Print GSM Status
void runsl() {
while (mySerial.available()) {
Serial.write(mySerial.read());
}
}
