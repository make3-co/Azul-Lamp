#include <Arduino.h>
#include <ezButton.h>
#include <Adafruit_MAX1704X.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pin for the potentiometer, LED strip, and touch sensor
const int potPin = A0;        // Analog pin where the potentiometer is connected
const int ledPin = 5;         // PWM-capable GPIO pin connected to the LED strip
const int touchPin = 6;       // GPIO pin connected to the touch sensor output

// USB power detection pin (depending on your setup, might need to be adjusted)
const int usbPin = 36;        // GPIO pin to detect USB power (adjust as needed)

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create an ezButton object for the touch sensor
ezButton touchButton(touchPin);

// Create an object for the MAX17048 battery monitor
Adafruit_MAX17048 max17048;

// Variables to manage LED state
bool ledState = false;  // LED is initially off

void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);
  pinMode(usbPin, INPUT);

  // Initialize serial communication
  Serial.begin(115200);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Configure PWM
  const int pwmChannel = 0;
  const int pwmFrequency = 5000; // 5 kHz frequency
  const int pwmResolution = 8;   // 8-bit resolution
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

void loop() {
  // Update the touch button state
  touchButton.loop();

  // Check if the touch button was pressed
  if (touchButton.isPressed()) {
    ledState = !ledState;  // Toggle LED state
    Serial.println("Touch detected, toggling LED state");
  }

  // If the LED is supposed to be on, adjust brightness with the potentiometer
  int potValue = analogRead(potPin);           // Read potentiometer value
  if (ledState) {
    int pwmValue = map(potValue, 0, 4095, 0, 255);  // Map to PWM range
    ledcWrite(0, pwmValue);                      // Adjust LED brightness
  } else {
    ledcWrite(0, 0);  // Turn off the LED
  }

  // Read battery information from MAX17048
  float voltage = max17048.cellVoltage();       // Get battery voltage (in volts)
  float soc = max17048.cellPercent();           // Get battery state of charge (in percentage)

  // Detect power source (USB or Battery)
  int usbVoltage = analogRead(usbPin);          // Read the analog value from the USB pin
  Serial.print("USB Pin Voltage: ");
  Serial.println(usbVoltage);                   // Debugging: Print the USB pin voltage

  String powerSource;
  if (usbVoltage > 3000) {  // Adjust threshold as needed
    powerSource = "USB";
  } else {
    powerSource = "Battery";
  }

  // Display battery information, touch pin state, pot value, LED state, and power source on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Power: ");
  display.println(powerSource);

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

  // Debugging output
  Serial.print("LED State: ");
  Serial.println(ledState ? "ON" : "OFF");

  delay(10);
}