#include <Arduino.h>

#include <Adafruit_DotStar.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <avr/dtostrf.h>

#define NUMPIXELS 1 // Number of LEDs in strip
// control the LEDs:
#define DATAPIN 7
#define CLOCKPIN 8

Adafruit_DotStar strip = Adafruit_DotStar(
    NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// SSD1306 128x32 display connected to I2C (SCL=A1, SDA=A2)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Compile-time switch: Set to 1 to enable calibration mode, 0 for normal shutter speed timing
#define CALIBRATE_IR 0


// put function declarations here:
void introScreen(char *);
void drawSensorBarPlot(int);
void drawShutterResults(char *);

static constexpr int threshold = 600; // Threshold for the IR-receiver, set by results of the 'calibration' compile and observations

static constexpr int pin = A3; // Analog input (trinketM0 pin A3) is used to measure the resistance of the IR-receiver (is pulled high, and goes low to ground when IR light hits the sensor)
int sensorValue = 0;           // analog read value

float start = 0;                 // timer var for shutter open time
float stop = 0;                  // timer var for shutter close time
float shutterDuration_us = 0;    // duration of shutter being open (microseconds)
float shutterDuration_s = 0;     // duration of shutter being open (microseconds)
float shutterDuration_s_inv = 0; // duration of shutter being open (seconds, denominator under "1")

int shutterSpeed = 0;

bool isOpen = false; // if shutter is open or closed (state)

char dispLine[30] = "none yet"; // used for the sprintf setup and display

void setup() // put your setup code here, to run once:
{
  Serial.begin(9600);

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off

  // initialize digital pin 13 as an output (Trinket M0 onboard LED), though it's not really needed...
  pinMode(13, OUTPUT);

  // Initialize I2C with pins (SCL=A1, SDA=A2)
  // (For Adafruit Trinket M0, the I2C is on pins as specified)
  Wire.begin();

  // Initialize SSD1306 display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (true)
    {
      ;
    } // but keep running program? hmm maybe no
  }

  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  introScreen(dispLine);

  // delay(2000);
}

void loop()
{
  sensorValue = analogRead(pin);

#if CALIBRATE_IR == 1 // CALIBRATION MODE: Display sensor value bar plot

  drawSensorBarPlot(sensorValue);

  // Serial.print("Sensor Value: ");
  // Serial.println(sensorValue);
  // delay(500);

#else // NORMAL MODE: Measure shutter speed
  // shutter opens
  if (sensorValue < threshold && !isOpen) // analog reads high until pulled low when IR hits the sensor at shutter open time
  {
    start = micros();
    isOpen = true;
    // no need to show the dotstar, also I don't want to slow the program down
    // digitalWrite(13, HIGH); // turn the LED on (HIGH is the voltage level)
    // strip.setPixelColor(0, 0, 64, 0);
    // strip.show(); // green
  }

  // Shutter closes
  else if (sensorValue > threshold && isOpen) // analog reads high again when sensor is blocked (closed shutter)
  {
    stop = micros();
    isOpen = false;

    // calculate the shutter stats
    shutterDuration_us = stop - start;
    shutterDuration_s = (shutterDuration_us / 1000000);

    if (shutterDuration_s < 1)
    {
      shutterDuration_s_inv = 1 / (shutterDuration_s);
      char floatStr[10];
      dtostrf(shutterDuration_s_inv, 4, 0, floatStr); // 6 width, 0 decimal places
      sprintf(dispLine, "Speed = 1/%s", floatStr);
    }
    else
    {
      char floatStr[10];
      dtostrf(shutterDuration_s, 5, 2, floatStr); // 5 width, 2 decimal places
      sprintf(dispLine, "Speed = %ss", floatStr);
    }

    Serial.println("Shutter detected! duration (microseconds):");
    Serial.println(shutterDuration_us);
    Serial.println("Duration (seconds):");
    Serial.println(shutterDuration_s);
    Serial.println(dispLine);

    drawShutterResults(dispLine);
    delay(2000);
    introScreen(dispLine);

    // digitalWrite(13, LOW); // turn the LED off by making the voltage LOW
    // strip.setPixelColor(0, 0, 0, 0);
    // strip.show(); // off
  }

#endif
}



// put function definitions here:

// Intro and ready for new shutter event screen
void introScreen(char *dispLine)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Shutter Time Sensor"));
  display.println(F("     (ready)"));
  display.println("last: ");
  display.print(dispLine);
  display.display();
}

// Function to draw a shutter sensed result
void drawShutterResults(char *dispLine)
{
  // Clear display
  display.clearDisplay();

  // Draw title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Shutter detected!");
  display.println(dispLine);
  display.println("(2sec before reset)");
  display.display();
}

// Function to draw a bar plot of the sensor value (calibration use only, when bool calibrateIR = true; )
void drawSensorBarPlot(int sensorValue)
{
  // Clear display
  display.clearDisplay();

  sprintf(dispLine, "Sensor Value = %d", sensorValue);

  // Draw title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(dispLine);

  // Map sensor value (0-1023) to bar width (0-120 pixels)
  // and height for visualization (max 20 pixels tall)
  int barWidth = map(sensorValue, 0, 1023, 0, 120);
  int barHeight = 12;
  int barY = 18;

  // Draw border for the bar plot area
  display.drawRect(4, barY - 2, 122, barHeight + 4, SSD1306_WHITE);

  // Draw filled bar
  if (barWidth > 0)
  {
    display.fillRect(6, barY, barWidth, barHeight, SSD1306_WHITE);
  }

  // Display sensor value as number
  // display.setCursor(0, 24);
  // display.print(F("Value: "));
  // display.println(sensorValue);

  display.display();
}
