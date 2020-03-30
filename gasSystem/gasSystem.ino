#include "MicroGear.h" //เรียกlibrary microgear
#include "WiFi.h" //เรียก library wifi
#define APPID "APPID" //appidที่เราสร้างใน netpie
#define KEY "KEY" //keyที่อยู่ในnetpie
#define SECRET "SECRET" //secretที่อยู่ในnetpie
#define ALIAS "ALIAS" //ชื่อdevice esp32 ที่จะต่อinternet 
#define valvePin 25 //valve แก๊ส pin 25
#define gasPin A3 // sensor detact gas pin a3

bool btnClose = false;
int gasValue;
int vol;
int valve;
#define BUZZER_PIN          13 //buzzer pin 13

#ifdef  ARDUINO_ARCH_ESP32

#define SOUND_PWM_CHANNEL   0
#define SOUND_RESOLUTION    8 // 8 bit resolution
#define SOUND_ON            (1<<(SOUND_RESOLUTION-1)) // 50% duty cycle
#define SOUND_OFF           0                         // 0% duty cycle

void tone(int pin, int frequency, int duration)  //เป็นฟังชั่นในการสร้างเสียง buzzer แทนการใช้ analogWrite ซึ่งไม่สามารถใช้ analogWrite ใน esp32
{
  ledcSetup(SOUND_PWM_CHANNEL, frequency, SOUND_RESOLUTION);  // Set up PWM channel
  ledcAttachPin(pin, SOUND_PWM_CHANNEL);                      // Attach channel to pin
  ledcWrite(SOUND_PWM_CHANNEL, SOUND_ON);
  delay(duration);
  ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
}

#endif
const char* ssid     = "WifiName";
const char* password = "passwordWifi";
WiFiServer server(80);
WiFiClient client;
MicroGear microgear(client);
void setup() {
  Serial.begin(115200);
  pinMode(valvePin, OUTPUT);
  pinMode(gasPin, INPUT);
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
  digitalWrite(valvePin, 1); //ให้วาล์วแก๊สปิดการทำงานก่อนในตอนแรก
  microgear.chat("VALVEGAS", 0); //ส่งไปบอก freeboard ว่าวาล์วมันปิดในที่นี่ จ ในfrrboard คือปิด
}

void loop() {
  gasValue = analogRead(gasPin); //อ่านค่าเซนเซอร์ดีเทคแก๊ส
  microgear.publish("/gasSystem/gasValue", gasValue, true); //publish ค่าแก๊สเพื่อเอาไปแสดงใน freeboard
  valve = digitalRead(valvePin); //อ่านค่าว่าวาล์วแก๊สเปิดหรือปิดอยู่
  if (gasValue > 2000 && btnClose == false) { //เช็คว่าค่าเซนเซอร์ดีเทคแก๊สมากกว่า 2000 ไหม และปุ่มปิดแก๊สในfreeboard เปิดหรือปิด
    digitalWrite(valvePin, 0); //ให้ปิดวาล์ว
    microgear.chat("VALVEGAS", 1); //
    tone(BUZZER_PIN, 300, 1000); //buzzer แจ้งเตือน
  } else if (gasValue > 2000 && btnClose == true) {
    digitalWrite(valvePin, 0); //ให้ปิดวาล์ว
    microgear.chat("VALVEGAS", 1);
  } else if (gasValue < 2000 && btnClose == true) {
    btnClose = false;
  }
  //  Serial.println(gasValue);
    Serial.println(valve);
  //  Serial.println("---------");
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
}
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.println(topic);
  Serial.print("Incomming message -->");
  char *m = (char *)msg;
  m[msglen] = '\0';
  if (gasValue > 2000) { //ถ้าเซนเซอร์ดีเทคแก๊สได้มากกว่า 2000 
    if (atoi(m) == 1) { //ถ้ากดปุ่มปิดเสียงbuzzer ใน freeboard 
      digitalWrite(valvePin, 0); //วาล์วปิด
      microgear.chat("VALVEGAS", 1); //บอก freeboard valve ปิด
      tone(BUZZER_PIN, 0, 0); //ให้ buzzer เงียบ
    }
  } else {
    if (atoi(m) == 1) { //ถ้าเซนเซอร์ดีเทคแก๊สได้น้อยกว่า 2000 แล้ว ถ้ากดปุ่มปิดวาล์ว ใน freeboard 
      digitalWrite(valvePin, 0);   //วาล์วปิด
      microgear.chat("VALVEGAS", 1); //บอก freeboard valve ปิด
    } else if (atoi(m) == 0) { //ถ้าเซนเซอร์ดีเทคแก๊สได้น้อยกว่า 2000 แล้ว ถ้ากดปุ่มเปิดวาล์ว ใน freeboard 
      digitalWrite(valvePin, 1); //ให้วาล์วเปิด
      microgear.chat("VALVEGAS", 0); //บอก freeboard valve เปิด
    } else if (atoi(m) == 2) { //ถ้าเซนเซอร์ดีเทคแก๊สได้น้อยกว่า 2000 แล้ว ถ้ากดปุ่มปิดเสียง buzzer
      btnClose = true;
      tone(BUZZER_PIN, 0, 0); //ให้buzzer เงียบ
    }
  }
}
