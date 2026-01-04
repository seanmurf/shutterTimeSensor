#include <Arduino.h>

#include <Adafruit_DotStar.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define NUMPIXELS 1 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN 7
#define CLOCKPIN 8

Adafruit_DotStar strip = Adafruit_DotStar(
    NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// SSD1306 128x32 display connected to I2C (SCL=A1, SDA=A2)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// put function declarations here:
// int myFunction(int, int);
void drawSensorBarPlot(int);

// Threshold for the IR-receiver
static constexpr int threshold = 850;
// Analog input 7 is used to measure the voltage across the IR-receiver
static constexpr int pin = A3;

int sensorValue = 0;
float start = 0;
float stop = 0;

bool isOpen = false;
bool test = true;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off

  // initialize digital pin 13 as an output for simple blink
  pinMode(13, OUTPUT);

  // Initialize I2C with custom pins (SCL=A1, SDA=A2)
  // For Adafruit Trinket M0, the I2C is on pins as specified
  Wire.begin();
  
  // Initialize SSD1306 display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ShutterTimeSensor"));
  display.display();
  delay(1000);
}

void loop()
{
  int shutterSpeed = 0;

  sensorValue = analogRead(pin);

  // Update the display with the sensor value bar plot
  drawSensorBarPlot(sensorValue);

  // digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(1000);              // wait for a second
  // digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  // delay(1000);              // wait for a second

  bool calibrateIR = true;

      if (calibrateIR)
  {
    Serial.print("Sensor Value: ");
    Serial.println(sensorValue);
    delay(500);
  }
  else
  {

    // shutter opens
    if (sensorValue > threshold && !isOpen)
    {
      start = micros();
      isOpen = true;
      digitalWrite(13, HIGH); // turn the LED on (HIGH is the voltage level)
      strip.setPixelColor(0, 0, 64, 0);
      strip.show(); // green
    }
    // Shutter closes
    else if (sensorValue < threshold && isOpen)
    {
      stop = micros();
      Serial.println(stop - start);
      shutterSpeed = 1 / ((stop - start) / 1000000);
      Serial.println("ShutterSpeed:");
      Serial.println(shutterSpeed);
      isOpen = false;
      digitalWrite(13, LOW); // turn the LED off by making the voltage LOW
      strip.setPixelColor(0, 0, 0, 0);
      strip.show(); // off
    }
  }
}

// put function definitions here:

// Function to draw a bar plot of the sensor value
void drawSensorBarPlot(int sensorValue) {
  // Clear display
  display.clearDisplay();
  
  // Draw title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Sensor Value"));
  
  // Map sensor value (0-1023) to bar width (0-120 pixels)
  // and height for visualization (max 20 pixels tall)
  int barWidth = map(sensorValue, 0, 1023, 0, 120);
  int barHeight = 12;
  int barY = 18;
  
  // Draw border for the bar plot area
  display.drawRect(4, barY - 2, 122, barHeight + 4, SSD1306_WHITE);
  
  // Draw filled bar
  if (barWidth > 0) {
    display.fillRect(6, barY, barWidth, barHeight, SSD1306_WHITE);
  }
  
  // Display sensor value as number
  display.setCursor(0, 24);
  display.print(F("Value: "));
  display.println(sensorValue);
  
  display.display();
}
