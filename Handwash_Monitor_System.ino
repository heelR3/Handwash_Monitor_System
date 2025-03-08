#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>

// OLED display size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Fingerprint Sensor Settings
#define rxPin 2  // RX pin of fingerprint sensor
#define txPin 3  // TX pin of fingerprint sensor
SoftwareSerial mySerial(rxPin, txPin);  // Create a software serial port for fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// IR Sensor and Actuators
#define irPin 8     // IR sensor pin
#define buzzerPin 6 // Buzzer pin
#define relayPin 10  // Relay pin

void setup() {
  // Start serial communication
  Serial.begin(9600);
  mySerial.begin(57600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }

  // Clear display and set text properties
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // Initialize Fingerprint Sensor
  if (finger.verifyPassword()) {
    display.setCursor(0, 10);
    display.print(F("Fingerprint ready"));
    display.display();
    delay(2000);
  } else {
    display.setCursor(0, 10);
    display.print(F("Sensor error"));
    display.display();
    while (true);
  }

  // Initialize pins
  pinMode(irPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  
  digitalWrite(buzzerPin, LOW);
  digitalWrite(relayPin, LOW);
}

void loop() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("HAND WASH");
  display.display();
  if (digitalRead(irPin) == LOW) {  // IR sensor triggered
    // Activate buzzer for 0.5 sec
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);

    // Activate relay for 15 sec
    digitalWrite(relayPin, HIGH);
    delay(15000);
    digitalWrite(relayPin, LOW);

    // Start countdown (2 min)
    for (int i = 105; i >= 0; i--) {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.print(F("Time left: "));
      display.print(i);
      display.print(F("sec"));
      display.display();
      
      if (i == 20) {  // At 1 min 15 sec
        digitalWrite(relayPin, HIGH);
        delay(15000);
        digitalWrite(relayPin, LOW);
      }
      
      delay(1000);
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Successful");
    display.display();
    delay(1000);
    
    // Prompt for fingerprint
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print(F("Place finger"));
    display.display();

    while (readFingerprint() == -1) {
      delay(500);
    }

    // Fingerprint verified
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print(F("Verified"));
    display.display();
    delay(2000);
  }
}

// Function to check fingerprint
int readFingerprint() {
  int p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK) {
      p = finger.fingerFastSearch();
      if (p == FINGERPRINT_OK) {
        return finger.fingerID;  // Return finger ID if found
      }
    }
  }
  return -1;  // Return -1 if fingerprint not found
}
