 #include "MicroGear.h" //เรียกlibrary microgear
#include "WiFi.h" //เรียก libraryเพื่อต่อ wifi
#include <DHT.h> //เรียกlibraryวัดความชื้นสัมพันธ์
#define APPID "APPID" //appidที่เราสร้างใน netpie
#define KEY "KEY" //keyที่อยู่ในnetpie
#define SECRET "SECRET" //secretที่อยู่ในnetpie
#define ALIAS "ALIAS"  //ชื่อdevice esp32 ที่จะต่อinternet
#define dhtPin 25 // pin เบอร์ 25 ต่อ sensor วัดความชื้น
const char* ssid     = "WifiName";
const char* password = "passwordWifi";
float hum;
float humtwo;
DHT dht(dhtPin, DHT22); //สร้าง object
WiFiServer server(80);
WiFiClient client;
MicroGear microgear(client);

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { //ถ้าต่อไวไฟไม่ได้มันจะวนลูปนี้ไปเรื่อยๆ
    delay(500);
    Serial.print(".");
  }
  microgear.on(MESSAGE, onMsghandler); //หลุดมาบรรทัดล่างจะต่อไวไฟได้แล้ว เรียกฟังชั่นonMsghandler ด้านล่าง free board ที่เรา subscribe ส่งค่าเซนเซอร์ที่เราอยากได้มาไหม
  microgear.on(CONNECTED, onConnected); // เรียกฟังชั่น onConnected ซึ่งอยู่ด้านล่าง
  microgear.init(KEY, SECRET, ALIAS); //กำหนดค่าเริ่มต้น key secret alias
  microgear.connect(APPID); //สั่งเชื่อมต่อกับ appid ในnetpie
}

void loop() {
  hum = dht.readHumidity(); //อ่านค่าความชื่นแล้วเก็บในตัวแปรชื่อ hum
//  Serial.println(hum);
  server.begin(); // สั่งให้ server เริ่มทำงาน (ที่เราพยายามต่อไวไฟ)
if (microgear.connected()) {  //เช็คว่าถ้าเชื่อมต่อกับ microgear ได้ จะให้ ส่งค่าเซนเซอร์ไปอัพเดตในfreeboard ที่สร้างขึ้น
    microgear.loop(); //รักษาการทำงาน ให้มันทำงานไปเรื่อยๆไม่หยุด
    microgear.publish("/waterlevel/myhum2", hum, true); //ส่งค่าไปอัพเดตความชื้นใน freeboard
  } else {
    microgear.connect(APPID); //ถ้าเงื่อนไขแรกไม่ผ่านแล้วหลุดมาใน else จะให้พยายามทำการเชื่อมต่อ appid ของเราใน netpie ใหม่
  }
  delay(500);
}
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  microgear.setName(ALIAS); //สร้างชื่อ device ของเราในที่นี้จะเป็นชื่อของ esp32
  microgear.subscribe("/waterlevel/hum"); //สั่งให้มัน รับค่า (subscribe)ความชื้นอีกตัวจาก esp ตัวอื่นที่ ส่งค่าเซนเซอร์แบบ publish(ใครจะรับค่าก็ได้)
}
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char *h = (char *)msg;
  h[msglen] = '\0'; 
  humtwo = atof(h); // สองบรรทัดข้างบนเป็นการพยายาม แปลงค่าเซนเซอร์ที่ได้มาจากที่เรา subscribe ไปให้เป็นตัวเลข
  if (humtwo - hum > 11) {
    microgear.publish("/waterlevel/alertHum", "Warning", true);  //ถ้าผลต่างระหว่างเซนเซอร์มากกว่า 11 ให้ freeboard ที่สร้างแจ้งเตือนว่า warning (น้ำอาจรั่ว)
  } else {
    microgear.publish("/waterlevel/alertHum", "Normal", true); // ถ้าไม่ต่างแสดงว่าระบบประปาปกติ
  }
}
