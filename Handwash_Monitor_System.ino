#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define IR_SENSOR_PIN 4   // IR sensor connected to pin 4
#define GREEN_LED_PIN 8   // Green LED pin
#define RED_LED_PIN 9     // Red LED pin
#define RELAY_PIN 10      // Relay for controlling water valve

unsigned long waterDuration = 15000;   // Water flow time in milliseconds (15 seconds)
unsigned long handwashDuration = 120000; // Handwash timer (2 minutes)
unsigned long previousMillis = 0;  // Variable to store previous time for water flow
unsigned long handwashMillis = 0;  // Variable to store time for handwash countdown
bool waterFlowing = false;
bool handwashingDone = false;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Set pin modes
  pinMode(IR_SENSOR_PIN, INPUT);  // IR sensor pin
  pinMode(GREEN_LED_PIN, OUTPUT); // Green LED pin
  pinMode(RED_LED_PIN, OUTPUT);   // Red LED pin
  pinMode(RELAY_PIN, OUTPUT);     // Relay pin
  
  // Initial states
  digitalWrite(RELAY_PIN, LOW);  // Ensure water is off initially
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  
  // Display startup message
  display.setCursor(0, 0);
  display.print("System Ready");
  display.display();
}

void loop() {
  int val = digitalRead(IR_SENSOR_PIN); // Read the IR sensor

  if (val == HIGH && !waterFlowing) {
    // Sensor detects movement and water is not flowing
    waterFlowing = true;
    previousMillis = millis();  // Start water flow timer
    
    // Green LED ON, Red LED OFF
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH);  // Turn on water (relay activated)
    
    // Display message for water flow
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Water Flowing...");
    display.display();
  }
  
  if (waterFlowing) {
    // If water is flowing, check if 15 seconds have passed
    if (millis() - previousMillis >= waterDuration) {
      waterFlowing = false;
      digitalWrite(RELAY_PIN, LOW);  // Turn off water after 15 seconds

      // Display the handwash countdown
      handwashMillis = millis();  // Start 2-minute countdown
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Handwashing... 2:00");
      display.display();
    }
  }
  
  if (millis() - handwashMillis < handwashDuration && !waterFlowing) {
    // If in handwash phase (2 minutes countdown)
    unsigned long remainingTime = handwashDuration - (millis() - handwashMillis);
    unsigned long remainingSeconds = remainingTime / 1000;
    
    // Update OLED display with the countdown
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Handwashing... ");
    display.print(minutes);
    display.print(":");
    if (seconds < 10) display.print("0");
    display.print(seconds);
    display.display();
  }
  
  if (millis() - handwashMillis >= handwashDuration && !waterFlowing && !handwashingDone) {
    // After 2-minute handwashing, give final 15 seconds of water
    waterFlowing = true;
    previousMillis = millis();  // Start final 15 seconds
    
    digitalWrite(GREEN_LED_PIN, LOW);  // Green LED OFF
    digitalWrite(RED_LED_PIN, HIGH);  // Red LED ON
    digitalWrite(RELAY_PIN, HIGH);    // Turn on water again
    
    // Display final 15 seconds message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Final 15s Water Flow");
    display.display();
  }
  
  if (millis() - previousMillis >= waterDuration && waterFlowing && !handwashingDone) {
    // If final 15 seconds water flow has finished
    waterFlowing = false;
    digitalWrite(RELAY_PIN, LOW);  // Turn off water
    handwashingDone = true;        // Mark the end of handwashing
    
    // Display "Handwashing Done" message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Handwashing Done!");
    display.display();
    
    // Reset after displaying the final message for 2 seconds
    delay(2000);
    handwashingDone = false;
    
    // Reset system and wait for next sensor detection
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  }
}
