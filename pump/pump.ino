#include "MicroGear.h" //เรียกlibrary microgear
#include "WiFi.h" //เรียก library wifi
#define APPID "APPID" //appidที่เราสร้างใน netpie
#define KEY "KEY" //keyที่อยู่ในnetpie
#define SECRET "SECRET" //secretที่อยู่ในnetpie
#define ALIAS "ALIAS" //ชื่อdevice esp32 ที่จะต่อinternet
#define pumpPin 25 //pump น้ำเข้าบ้านใช้พิน 25
#define pumpOutPin 26 pump น้ำเข้าถังใช้พิน 26

float hum;
const char* ssid     = "WifiName";
const char* password = "passwordWifi";
float levelNow = 1000;
float rainValue = 10000;
float humNearPipe = 0;
float humOutside = 0;
int pumpValue;
int pumpOutValue;
WiFiServer server(80);
WiFiClient client;
MicroGear microgear(client);

void setup() {
  Serial.begin(115200);
  pinMode(pumpPin, OUTPUT);
  pinMode(pumpOutPin, OUTPUT);
  digitalWrite(pumpOutPin, 1);
  WiFi.begin(ssid, password);
  delay(10);
  while (WiFi.status() != WL_CONNECTED) { //ถ้าต่อไวไฟไม่ได้มันจะวนลูปนี้ไปเรื่อยๆ
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  microgear.on(MESSAGE, onMsghandler); // เรียกฟังชั่น onMsghandler ซึ่งอยู่ด้านล่าง
  microgear.on(CONNECTED, onConnected); // เรียกฟังชั่น onConnected ซึ่งอยู่ด้านล่าง
  microgear.init(KEY, SECRET, ALIAS); //กำหนดค่าเริ่มต้น key secret alias
  microgear.connect(APPID); //สั่งเชื่อมต่อกับ appid ในnetpie
}

void loop() {
  server.begin();
  if (microgear.connected()) { //เช็คว่าถ้าเชื่อมต่อกับ microgear ได้ จะให้ทำข้างล่าง
    microgear.loop(); //จะให้รักษาการทำงาน ให้มันทำงานไปเรื่อยๆไม่หยุด
  } else {
    microgear.connect(APPID); //ถ้าเงื่อนไขแรกไม่ผ่านแล้วหลุดมาใน else จะให้พยายามทำการเชื่อมต่อ appid ของเราใน netpie ใหม่
  }
  delay(1000);
}
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  microgear.setName(ALIAS); //สร้างชื่อ device ของเราในที่นี้จะเป็นชื่อของ esp32
  microgear.subscribe("/waterlevel/ultrasonic"); //รับค่าultrasonic sensor ที่ถูก publish จาก esp อีกอันมาเช็คเปิดปิดปั๊ม
  microgear.subscribe("/waterlevel/rain"); //รับค่าน้ำเปียก (raindrop sensor) ที่ถูก publish จาก esp อีกอัน
}
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  if (String(topic) == String("/testFinalProject/waterlevel/ultrasonic") ) {
    char *h = (char *)msg;
    h[msglen] = '\0';
    levelNow = atof(h); // สองบรรทัดข้างบนเป็นการพยายาม แปลงค่าเซนเซอร์ที่ได้มาจากultrasonicที่เรา subscribe ไปให้เป็นตัวเลข เก็บในรูปตัวแปร levelNow
    if (rainValue < 500) { //ถ้าค่าน้ำเปียกน้อยกว่า 500 ให้ปิดปั๊มทั้งสองตัว
      digitalWrite(pumpOutPin, 1);
      digitalWrite(pumpPin, 1);
    } else {
      if (levelNow > 7.5) { //ถ้าระดับน้ำมากกว่า 7.5 ให้ปิดปั๊มน้ำที่เข้าบ้าน เปิดปั๊มน้ำเข้าถังเก็บน้ำ
        digitalWrite(pumpPin, 1);
        digitalWrite(pumpOutPin, 0);
      } else { //ถ้าไม่ใช่ให้เปิดปั๊มน้ำเข้าบ้านปิดปั๊มน้ำเข้าถังเก็บน้ำ
        digitalWrite(pumpPin, 0);
        digitalWrite(pumpOutPin, 1);
      }
    }
    pumpValue = digitalRead(pumpPin); 
    pumpOutValue = digitalRead(pumpOutPin);
    // สองบรรทัดข้างบนเป็นการอ่านค่าปั๊มว่ามันเปิดหรือปิด เพื่อไปแสดงสถานะบน freeboard ข้างล่าง
    if (pumpValue == 1) {
      microgear.chat("PUMPIN", "CH1OFF"); //ส่งไปอัพเดตสถานะบน freeboard ว่าปั๊มเข้าบ้านปิด
    } else {
      microgear.chat("PUMPIN", "CH1ON"); //ส่งไปอัพเดตสถานะบน freeboard ว่าปั๊มเข้าบ้านเปิด
    }
    if (pumpOutValue == 1) {
      microgear.chat("PUMPOUT", "CH2OFF"); //ส่งไปอัพเดตสถานะบน freeboard ว่าปั๊มเข้าถังปิด
    } else {
      microgear.chat("PUMPOUT", "CH2ON"); //ส่งไปอัพเดตสถานะบน freeboard ว่าปั๊มเข้าถังเปิด
    }
  }
  if (String(topic) == String("/testFinalProject/waterlevel/rain")) {
    char *k = (char *)msg;
    k[msglen] = '\0';
    rainValue = atof(k); // สองบรรทัดข้างบนเป็นการพยายาม แปลงค่าเซนเซอร์ที่ได้มาจากraindropที่เรา subscribe ไปให้เป็นตัวเลข เก็บในรูปตัวแปร rainValue
    Serial.println(rainValue);
  }
}
