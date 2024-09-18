#include <Arduino.h>
#include <ezButton.h>
#include <Adafruit_MAX1704X.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin definitions
constexpr int potPin = A0;
constexpr int ledPin = 5;
constexpr int touchPin = 6;

// OLED display settings
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr int OLED_ADDRESS = 0x3C;

// PWM settings
constexpr int pwmChannel = 0;
constexpr int pwmFrequency = 5000;
constexpr int pwmResolution = 8;

// Object initializations
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ezButton touchButton(touchPin);
Adafruit_MAX17048 max17048;

bool ledState = false;

void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);

  // Initialize serial communication
  Serial.begin(115200);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) { // Address 0x3C for 128x64
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Configure PWM
  ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
  ledcAttachPin(ledPin, pwmChannel);

  // Configure the touch button with debounce
  touchButton.setDebounceTime(50); // Set debounce time to 50ms

  // Initialize the MAX17048 battery monitor
  if (!max17048.begin()) {
    Serial.println("MAX17048 not found. Check wiring!");
    while (1);
  }
  Serial.println("MAX17048 found!");
}

void handleTouchButton() {
  touchButton.loop();
  if (touchButton.isPressed()) {
    ledState = !ledState;
    Serial.println("Touch detected, toggling LED state");
  }
}

void updateLED() {
  int potValue = analogRead(potPin);
  int pwmValue = ledState ? map(potValue, 0, 4095, 0, 255) : 0;
  ledcWrite(pwmChannel, pwmValue);
}

void updateDisplay() {
  float voltage = max17048.cellVoltage();
  float soc = max17048.cellPercent();
  int potValue = analogRead(potPin);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Battery: ");
  display.print(voltage);
  display.println(" V");
  display.print("Charge: ");
  display.print(soc);
  display.println(" %");

  display.print("Touch Pin: ");
  display.println(ledState ? "Pressed" : "Released");

  display.print("Pot Value: ");
  display.println(potValue);

  display.print("LED State: ");
  display.println(ledState ? "ON" : "OFF");

  display.display();
}

void loop() {
  handleTouchButton();
  updateLED();
  updateDisplay();

  Serial.print("LED State: ");
  Serial.println(ledState ? "ON" : "OFF");

  delay(10);
}