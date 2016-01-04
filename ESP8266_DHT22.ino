/*                                      +-----+
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |           GND/RST2  [ ] [ ]         |
         |         MOSI2/SCK2  [ ] [ ]  SCL[ ] |   D0
         |            5V/MISO2 [ ] [ ]  SDA[ ] |   D1
         |                             AREF[ ] |
         |                              GND[ ] |
         | [ ]N/C                        13[ ]~|   B7
         | [ ]IOREF                      12[ ]~|   B6
         | [ ]RST                        11[ ]~|   B5
         | [ ]3V3      +----------+      10[ ]~|   B4
         | [ ]5v       | ARDUINO  |       9[ ]~|   H6
         | [ ]GND      |   MEGA   |       8[ ]~|   H5
         | [ ]GND      +----------+            |
         | [ ]Vin                         7[ ]~|   H4
         |                                6[ ]~|   H3
         | [ ]A0                          5[H]~|   E3
         | [ ]A1                          4[ ]~|   G5
         | [ ]A2                     INT5/3[ ]~|   E5
         | [ ]A3                     INT4/2[ ]~|   E4
         | [ ]A4                       TX>1[D]~|   E1
         | [ ]A5                       RX<0[D]~|   E0
         | [ ]A6                               |   
         | [ ]A7                     TX3/14[ ] |   J1
         |                           RX3/15[ ] |   J0
         | [ ]A8                     TX2/16[ ] |   H1         
         | [ ]A9                     RX2/17[ ] |   H0
         | [ ]A10               TX1/INT3/18[E] |   D3         
         | [ ]A11               RX1/INT2/19[E] |   D2
         | [ ]A12           I2C-SDA/INT1/20[ ] |   D1         
         | [ ]A13           I2C-SCL/INT0/21[ ] |   D0
         | [ ]A14                              |            
         | [ ]A15                              |   Ports:
         |                RST SCK MISO         |    22=A0  23=A1   
         |         ICSP   [ ] [ ] [ ]          |    24=A2  25=A3   
         |                [ ] [ ] [ ]          |    26=A4  27=A5   
         |                GND MOSI 5V          |    28=A6  29=A7   
         | G                                   |    30=C7  31=C6   
         | N 5 5 4 4 4 4 4 3 3 3 3 3 2 2 2 2 5 |    32=C5  33=C4   
         | D 2 0 8 6 4 2 0 8 6 4 2 0 8 6 4 2 V |    34=C3  35=C2   
         |         ~ ~                         |    36=C1  37=C0   
         | @ # # # # # # # # # # # # # # # # @ |    38=D7  39=G2    
         | @ # # # # # # # # # # # # # # # # @ |    40=G1  41=G0   
         |           ~                         |    42=L7  43=L6   
         | G 5 5 4 4 4 4 4 3 3 3 3 3 2 2 2 2 5 |    44=L5  45=L4   
         | N 3 1 9 7 5 3 1 9 7 5 3 1 9 7 5 3 V |    46=L3  47=L2   
         | D                                   |    48=L1  49=L0    SPI:
         |                                     |    50=B3  51=B2     50=MISO 51=MOSI
         |     2560                ____________/    52=B1  53=B0     52=SCK  53=SS 
          \_______________________/         
         
         http://busyducks.com/ascii-art-arduinos
*/
/*
 * The DHT22 data sent via ESP8266 thingspeak.com
 */

#include "dht.h"

//------------------------------------------------------------------  
// [H]    DHT22
//------------------------------------------------------------------ 
dht DHT;
#define DHTPIN 5
float Celsius_Temperature, Humidity;
//------------------------------------------------------------------  
// [E]    ESP8266
//------------------------------------------------------------------ 
#define ESP8266_baudrate  115200
HardwareSerial & ESP8266Serial = Serial1;
//------------------------------------------------------------------  
// [D]    Debug
//------------------------------------------------------------------ 
#define Debug_baudrate    9600
HardwareSerial & dbgSerial = Serial;
#define DEBUG true

#define APSSID "AP SSID"
#define PASS "AP Password"
#define Thingspeak "api.thingspeak.com"   // 182.106.153.149
#define PORT "80"
#define APIKey "XXXXXXXXXXXXXXXX"
#define MyIotWeb "xxx.xxx.xxx"

boolean resule = false;

void setup()
{
  dbgSerial.begin(Debug_baudrate);
  while (!dbgSerial) { // check Debug Serial
    dbgSerial.print("Debug Serial is not ready!!!");
  }
  ESP8266Serial.begin(ESP8266_baudrate);
  while (!ESP8266Serial) { // check ESP8266 Serial
    dbgSerial.print("ESP8266 Serial is not ready!!!");
  }

  resule = sendData("AT\r\n", 3000, DEBUG, "OK"); // check module
  while (resule == false) {
    dbgSerial.print("AT command fail!!");
    resule = sendData("AT\r\n", 3000, DEBUG, "OK"); // check module again
  }
  sendData("AT+RST\r\n", 2000, DEBUG, ""); // reset module
  sendData("AT+CWMODE=1\r\n", 2000, DEBUG, ""); // configure as access point
  sendData("AT+CWJAP=\"" + String(APSSID) + "\",\"" + String(PASS) + "\"\r\n", 20000, DEBUG, "");
  sendData("AT+CIFSR\r\n", 2000, DEBUG, ""); // get ip address
  sendData("AT+CIPMUX=0\r\n", 2000, DEBUG, ""); // configure for multiple connections
}

void loop()
{
  send_DHT22_data();
  delay(20000);
}

void send_DHT22_data()
{
  // read DHT22 data
  DHT.read22(DHTPIN);
  Celsius_Temperature = DHT.temperature;
  Humidity = DHT.humidity;

  // Connect to Thingspeak
  sendData("AT+CIPSTART=\"TCP\",\"" + String(Thingspeak) + "\"," + String(PORT) + "\r\n", 10000, DEBUG, "");

  // send data to thingspeak.com
  String cmd = "GET /update?key=" + String(APIKey) +
               "&field1=" + String(Celsius_Temperature) +
               "&field2=" + String(Humidity) +
               "\r\n";
  resule = sendData("AT+CIPSEND=" + String(cmd.length()) + "\r\n", 2000, DEBUG, ">");
  if (resule) {
    resule = sendData(cmd, 2000, DEBUG, "+IPD,");
//    if (!resule)
//      sendData("AT+CIPCLOSE\r\n", 2000, DEBUG, "");
  }

  delay(10000);

  // Connect to My Iot Web
  sendData("AT+CIPSTART=\"TCP\",\"" + String(MyIotWeb) + "\"," + String(PORT) + "\r\n", 10000, DEBUG, "");
  cmd = "GET /dht22/record/" + String(Celsius_Temperature) +
               "/" + String(Humidity) +
               " HTTP/1.1\r\nHost: iot.chichen.tw\r\nUser-Agent: Chichen IOT Device\r\n" +
               "\r\n";
  resule = sendData("AT+CIPSEND=" + String(cmd.length()) + "\r\n", 2000, DEBUG, ">");
  if (resule) {
    resule = sendData(cmd, 2000, DEBUG, "CLOSE");
//    if (!resule)
//      sendData("AT+CIPCLOSE\r\n", 2000, DEBUG, "");
  }
}

boolean sendData(String command, const int timeout, boolean debug, String compareString)
{
  String response = "";
  String incomeString = "";
  boolean compareResule = false;

  ESP8266Serial.print(command); // send the read character to the ESP8266

  long int time = millis();

  while ( (time + timeout) > millis())
  {
    while (ESP8266Serial.available())
    {

      // The esp has data so display its output to the serial window
      char c = ESP8266Serial.read(); // read the next character.
      response += c;
      incomeString += c;
      if (c == '\n') {
        incomeString = "";
      } else {
        if (incomeString == compareString)
          compareResule = true;
      }
    }
  }

  if (debug)
  {
    dbgSerial.print(response);
  }

  return compareResule;
}
