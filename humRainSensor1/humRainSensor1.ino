#include <DHT.h> //เรียกlibraryวัดความชื้นสัมพันธ์
#include <ESP8266WiFi.h> //เรียกlibrary wifi esp8266
#include <MicroGear.h> //เรียกlibrary microgear
#define APPID "APPID" //appidที่เราสร้างใน netpie
#define KEY "KEY" //keyที่อยู่ในnetpie
#define SECRET "SECRET" //secretที่อยู่ในnetpie
#define ALIAS "ALIAS"  //ชื่อdevice esp8266 ที่จะต่อinternet
#define dhtPin D6 //pin 26 วัดความชื้น
#define led D3 //pin 3 แสดงไฟ
#define rainPin A0 //pin a0 วัดค่าน้ำเปียก
int rainValue;
const char* ssid     = "WifiName";
const char* password = "passwordWifi";
DHT dht(dhtPin, DHT22); //สร้าง object
WiFiServer server(80);
WiFiClient client;
MicroGear microgear(client);
float hum;

void setup() {
  Serial.begin(115200);
  dht.begin(); //เริ่มการทำงานเซนเซอร์วัดความชื้น
  WiFi.begin(ssid, password); //สั่งlibrary wifi ให้เริ่มทำงาน
  pinMode(led, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) { //ถ้าต่อไวไฟไม่ได้มันจะวนลูปนี้ไปเรื่อยๆ
    delay(500);
    Serial.print(".");
  }
  microgear.on(CONNECTED, onConnected); // เรียกฟังชั่น onConnected ซึ่งอยู่ด้านล่าง
  microgear.init(KEY, SECRET, ALIAS); //กำหนดค่าเริ่มต้น key secret alias
  microgear.connect(APPID); //สั่งเชื่อมต่อกับ appid ในnetpie
}

void loop() {
  hum = dht.readHumidity(); //อ่านค่าความชื่นแล้วเก็บในตัวแปรชื่อ hum
  rainValue = analogRead(rainPin); //อ่านค่าน้ำเปียกแล้วเก็บในตัวแปรชื่อ rainValue
  server.begin(); // สั่งให้ server เริ่มทำงาน (ที่เราพยายามต่อไวไฟ)
//  Serial.println(hum);
//  Serial.println("---------");
//  Serial.println(rainValue);
//  Serial.println("---------");
  if (microgear.connected()) { //เช็คว่าถ้าเชื่อมต่อกับ microgear ได้ จะให้ ส่งค่าเซนเซอร์ไปอัพเดตในfreeboard ที่สร้างขึ้น
    digitalWrite(led, 1); //ถ้าเชื่อมต่อได้ไฟจะติด
    microgear.loop(); //รักษาการทำงาน ให้มันทำงานไปเรื่อยๆไม่หยุด
    microgear.publish("/waterlevel/hum", hum, true); //ส่งค่าความชื้นไปอัพเดตใน freeboard
    microgear.publish("/waterlevel/rain", rainValue, true); //ส่งค่าน้ำเปียกไปอัพเดตใน freeboard
  } else {
    digitalWrite(led, 0); //ถ้าเชื่อมต่อไม่ได้ไฟจะไม่ติด
    microgear.connect(APPID); //ถ้าเงื่อนไขแรกไม่ผ่านแล้วหลุดมาใน else จะให้พยายามทำการเชื่อมต่อ appid ของเราใน netpie ใหม่
  }
  delay(500);
}
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  microgear.setName(ALIAS); //สร้างชื่อ device ของเราในที่นี้จะเป็นชื่อของ esp8266
}
