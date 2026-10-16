#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// กำหนดขนาดหน้าจอ OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// กำหนดขาพินใช้งาน
#define SOIL_PIN A0        // ขาอ่านค่าจาก Capacitive Soil Moisture Sensor
#define RELAY_PIN 2        // ขาสั่งงานรีเลย์เปิด-ปิดปั๊มน้ำ
#define LED_RED 3          // ขาไฟ LED สีแดง
#define LED_YELLOW 4       // ขาไฟ LED สีเหลือง
#define LED_GREEN 5        // ขาไฟ LED สีเขียว

// ค่าการปรับเทียบเซนเซอร์ความชื้น (Calibrate Value)
// **หมายเหตุ:** แนะนำให้อ่านค่าในอากาศแห้ง (Air) และค่าในน้ำ (Water) แล้วเอามาแก้ไขตัวเลข 2 ตัวนี้
const int AirValue = 620;   // ค่า Analog ตอนเซนเซอร์แห้งสนิท (0%)
const int WaterValue = 280; // ค่า Analog ตอนจุ่มเซนเซอร์ลงน้ำ (100%)

bool isWatering = false;    // ตัวแปรเช็คสถานะการรดน้ำ

void setup() {
  Serial.begin(9600);

  // กำหนดโหมดของพิน
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // ตั้งค่าเริ่มต้นให้ปิดรีเลย์ (โมดูลรีเลย์ส่วนใหญ่เป็น Active LOW: HIGH คือปิด, LOW คือเปิด)
  digitalWrite(RELAY_PIN, HIGH);

  // ปิดไฟทุกดวงก่อน
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  // เริ่มต้นการทำงานของหน้าจอ OLED (Address 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // ถ้าเชื่อมต่อจอไม่ได้ ให้หยุดทำงานตรงนี้
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println(F("System Ready..."));
  display.display();
  delay(1500);
}

void loop() {
  // 1. อ่านค่า Analog จากเซนเซอร์
  int rawValue = analogRead(SOIL_PIN);

  // 2. แปลงค่าเป็นเปอร์เซ็นต์ (0 - 100%)
  int moisturePercent = map(rawValue, AirValue, WaterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); // จำกัดค่าให้อยู่ช่วง 0-100%

  // 3. เงื่อนไขการสั่งงานปั๊มน้ำและไฟแสดงสถานะ
  if (moisturePercent <= 1) {
    // ความชื้นต่ำ: เริ่มสั่งรดน้ำ และติดไฟสีแดง
    isWatering = true;
    digitalWrite(RELAY_PIN, LOW); // เปิดปั๊มน้ำ (Active LOW)

    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);

  } else if (isWatering && moisturePercent < 50) {
    // อยู่ระหว่างการรดน้ำเติมความชื้น: ทำงานต่อจนกว่าจะถึงค่าที่กำหนด
    digitalWrite(RELAY_PIN, LOW); // ปั๊มทำงานต่อ

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH); // ติดไฟสีเหลืองบอกสถานะกำลังรดน้ำ
    digitalWrite(LED_GREEN, LOW);

  } else if (moisturePercent >= 50) {
    // ความชื้นถึงเกณฑ์: หยุดรดน้ำ และติดไฟสีเขียว
    isWatering = false;
    digitalWrite(RELAY_PIN, HIGH); // ปิดปั๊มน้ำ

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  // 4. แสดงผลบนหน้าจอ OLED
  display.clearDisplay();

  // หัวข้อ
  display.setTextSize(1);
  display.setCursor(15, 0);
  display.println(F("SMART IRRIGATION"));
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

  // แสดงเปอร์เซ็นต์
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print(F("Soil: "));
  display.print(moisturePercent);
  display.print(F("%"));

  // แสดงข้อความสถานะความชื้นตามเงื่อนไข
  display.setTextSize(1);
  display.setCursor(10, 48);

  if (moisturePercent <= 29) {
    display.println(F("Moisture: LOW"));
  } else if (moisturePercent >= 50) {
    display.println(F("Moisture: NORMAL"));
  } else {
    display.println(F("Status: WATERING..."));
  }

  display.display();

  // ปริ้นต์ดูผ่าน Serial Monitor
  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" | Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  delay(1000); // อ่านค่าอัปเดตทุกๆ 1 วินาที
}