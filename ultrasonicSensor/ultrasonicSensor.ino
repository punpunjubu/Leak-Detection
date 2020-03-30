#include <ESP8266WiFi.h> //เรียกlibrary wifi esp8266
#include <MicroGear.h> //เรียกlibrary microgear
#include <Ultrasonic.h> //เรียกultrasonic
#define APPID "APPID" //appidที่เราสร้างใน netpie
#define KEY "KEY" //keyที่อยู่ในnetpie
#define SECRET "SECRET" //secretที่อยู่ในnetpie
#define ALIAS "ALIAS" //ชื่อdevice esp8266 ที่จะต่อinternet
#define ultraPin D3 //pin 3 วัดระยะน้ำว่าสูงเท่าไหร่
#define led D5 // pin 5 สั่งไฟติด
const char* ssid     = "WifiName";
const char* password = "passwordWifi";
float ultra;
Ultrasonic ultrasonic(ultraPin); //สร้าง object
WiFiServer server(80);
WiFiClient client;
MicroGear microgear(client);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password); //สั่งlibrary wifi ให้เริ่มทำงาน
  pinMode(led, OUTPUT); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  microgear.on(CONNECTED, onConnected);
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);
}

void loop() {
  ultrasonic.MeasureInCentimeters(); //ให้ ultrasonic วัดค่าเป็น centimeter
  ultra = ultrasonic.RangeInCentimeters; //u;trasonic อ่านค่าระยะทาง
  Serial.println(ultra);
  server.begin();
  if (microgear.connected()) { //เช็คว่าถ้าเชื่อมต่อกับ microgear ได้ จะให้ ส่งค่าเซนเซอร์ไปอัพเดตในfreeboard ที่สร้างขึ้น
    digitalWrite(led, 1); //เชื่อมต่อได้ไฟจะติด
    microgear.loop(); //รักษาการทำงาน ให้มันทำงานไปเรื่อยๆไม่หยุด
    microgear.publish("/waterlevel/ultrasonic", ultra, true); //ส่งค่าultrasonic ให้ไปอัพเดตบน freeboard แล้วให้เก็บค่าล่าสุดไว้ (true = เก็บ)
  } else {
    digitalWrite(led, 0); //เชื่อมต่อmicrogear ไม่ได้ไฟไม่ติด
    microgear.connect(APPID); /ถ้าเงื่อนไขแรกไม่ผ่านแล้วหลุดมาใน else จะให้พยายามทำการเชื่อมต่อ appid ของเราใน netpie ใหม่
  }
  delay(1000);
}
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  microgear.setName(ALIAS); //สร้างชื่อ device ของเราในที่นี้จะเป็นชื่อของ esp8266
}
