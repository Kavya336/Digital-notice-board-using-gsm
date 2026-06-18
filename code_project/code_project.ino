#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include <BluetoothSerial.h>

#define NUMBER_OF_DEVICES 4
#define CS_PIN 5
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define BUZZER 26
#define LED1 25
#define LED2 33

HardwareSerial GSMSerial(1);
BluetoothSerial BT;

MD_Parola matrix = MD_Parola(HARDWARE_TYPE, CS_PIN, NUMBER_OF_DEVICES);

String noticeMessage = "NOTICE BOARD READY";
String incoming = "";
String gsmBuffer = "";

void setup() {
  Serial.begin(115200);
  GSMSerial.begin(9600, SERIAL_8N1, 16, 17);
  BT.begin("SmartNoticeBoard");

  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  matrix.begin();
  matrix.setIntensity(8);

  matrix.displayText("SMART NOTICE BOARD", PA_CENTER, 60, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!matrix.displayAnimate());
  matrix.displayText("ECE PROJECT", PA_CENTER, 60, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!matrix.displayAnimate());
  matrix.displayText(noticeMessage.c_str(), PA_LEFT, 60, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  startupLEDs();

  GSMSerial.println("AT");
  delay(1000);
  GSMSerial.println("AT+CMGF=1");
  delay(1000);
  GSMSerial.println("AT+CNMI=1,2,0,0,0");
  delay(1000);

  Serial.println("System Ready!");
}

void loop() {
  if (matrix.displayAnimate()) {
    matrix.displayReset();
  }

  while (BT.available()) {
    char c = BT.read();
    if (c == '\n' || c == '\r') {
      if (incoming.length() > 0) {
        noticeMessage = incoming;
        incoming = "";
        updateDisplay();
      }
    } else {
      incoming += c;
    }
  }

  if (GSMSerial.available()) {
    gsmBuffer = "";
    delay(500);
    while (GSMSerial.available()) {
      gsmBuffer += (char)GSMSerial.read();
    }
    String msg = extractSMS(gsmBuffer);
    if (msg.length() > 0) {
      noticeMessage = msg;
      updateDisplay();
    }
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (incoming.length() > 0) {
        noticeMessage = incoming;
        incoming = "";
        updateDisplay();
      }
    } else {
      incoming += c;
    }
  }
}

void updateDisplay() {
  alertLEDs();
  matrix.displayClear();
  matrix.displayText(noticeMessage.c_str(), PA_LEFT, 60, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  matrix.displayReset();
}

String extractSMS(String raw) {
  int firstLine = raw.indexOf('\n');
  if (firstLine != -1) {
    String msg = raw.substring(firstLine + 1);
    msg.trim();
    if (msg.length() > 0) return msg;
  }
  return "";
}

void startupLEDs() {
  digitalWrite(LED1, HIGH); delay(300);
  digitalWrite(LED2, HIGH); delay(300);
  tone(BUZZER, 1000, 500);
  delay(500);
  digitalWrite(LED1, LOW); delay(300);
  digitalWrite(LED2, LOW); delay(300);
}

void alertLEDs() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    tone(BUZZER, 1000, 200);
    delay(200);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    delay(200);
  }
}
