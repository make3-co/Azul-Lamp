#include <Arduino.h>
#include <ezButton.h>
#include <Adafruit_MAX1704X.h>

// Define the pin for the potentiometer, LED strip, and touch sensor
const int potPin = A0;        // Analog pin where the potentiometer is connected
const int ledPin = 5;         // PWM-capable GPIO pin connected to the LED strip
const int touchPin = 6;       // GPIO pin connected to the touch sensor output

// Create an ezButton object for the touch sensor
ezButton touchButton(touchPin);

// Create an object for the MAX17048 battery monitor
Adafruit_MAX17048 max17048;

// Variables to manage LED state
bool ledState = false;  // LED is initially off

void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);

  // Initialize serial communication
  Serial.begin(115200);

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
  if (ledState) {
    int potValue = analogRead(potPin);           // Read potentiometer value
    int pwmValue = map(potValue, 0, 4095, 0, 255);  // Map to PWM range
    ledcWrite(0, pwmValue);                      // Adjust LED brightness
  } else {
    ledcWrite(0, 0);  // Turn off the LED
  }

// Read battery information from MAX17048
  float voltage = max17048.cellVoltage();       // Get battery voltage (in volts)
  float soc = max17048.cellPercent();           // Get battery state of charge (in percentage)

  // Display battery information
  Serial.print("Battery Voltage: ");
  Serial.print(voltage);
  Serial.print(" V, State of Charge: ");
  Serial.print(soc);
  Serial.println(" %");


  // Debugging output
  Serial.print("LED State: ");
  Serial.println(ledState ? "ON" : "OFF");

  delay(10);
}