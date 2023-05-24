#include <HX711_ADC.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include "dht.h"

// Temperature PIN
#define dht_apin A6

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3    // Chip Select goes to Analog 3
#define LCD_CD A2    // Command/Data goes to Analog 2
#define LCD_WR A1    // LCD Write goes to Analog 1
#define LCD_RD A0    // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// motor inputs
const int ENA_PIN = 11; // the Arduino pin connected to the EN1 pin L298N
const int IN1_PIN = 47; // the Arduino pin connected to the IN1 pin L298N
const int IN2_PIN = 53;


// the Arduino pin connected to the IN2 pin L298N
const int IN3_PIN = 51; // the Arduino pin connected to the IN1 pin L298N
const int IN4_PIN = 49; // the Arduino pin connected to the IN2 pin L298N
const int ENB_PIN = 10 ; // the Arduino pin connected to the EN1 pin L298
int motorSpeedA = 0;
int motorSpeedB = 0;

// Rotary Encoder Inputs
const int CLK = 37;
const int DT = 39;
const int SW = 41;

// Load cell definitions
const int HX711_dout = 43; //mcu > HX711 dout pin
const int HX711_sck = 45; //mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck); //HX711 1

float firstvalue = 0;
float secondvalue = 0;
bool firsttime = true;
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;


// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Color definitions
#define ILI9341_BLACK 0x0000       /*   0,   0,   0 */
#define ILI9341_NAVY 0x000F        /*   0,   0, 128 */
#define ILI9341_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define ILI9341_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define ILI9341_MAROON 0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE 0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE 0x7BE0       /* 128, 128,   0 */
#define ILI9341_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define ILI9341_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define ILI9341_BLUE 0x001F        /*   0,   0, 255 */
#define ILI9341_GREEN 0x07E0       /*   0, 255,   0 */
#define ILI9341_CYAN 0x07FF        /*   0, 255, 255 */
#define ILI9341_RED 0xF800         /* 255,   0,   0 */
#define ILI9341_MAGENTA 0xF81F     /* 255,   0, 255 */
#define ILI9341_YELLOW 0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE 0xFFFF       /* 255, 255, 255 */
#define ILI9341_ORANGE 0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define ILI9341_PINK 0xF81F

#define YP A3 // must be an analog pin
#define XM A2 // must be an analog pin
#define YM 9  // can be a digital pin
#define XP 8  // can be a digital pin

// Touch For New ILI9341 TP
#define TS_LEFT 120
#define TS_RIGHT 900
#define TS_TOP 70
#define TS_BOTTOM 920

// Timer settings
#define PERIOD 5000

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 100);

Elegoo_GFX_Button buttons[5]; // 4 buttons in total: Joystick / Stop joystick, tensile/bending
dht DHT;

void setup(void)
{

  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  
  // Load cell
  Serial.println("LoadCell calibration Starting...");
  LoadCell.begin();
  float calibrationValue;
  calibrationValue = 22.06;
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Load cell startup is complete");}
Serial.println("INITIALIZATION COMPLETE");
Serial.println("You can now adjust with joystick. Type 'start' to start test");

  // motor setup
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));

#ifdef USE_Elegoo_SHIELD_PINOUT
  Serial.println(F("Using Elegoo 2.8\" TFT Arduino Shield Pinout"));
#else
  Serial.println(F("Using Elegoo 2.8\" TFT Breakout Board Pinout"));
#endif

  Serial.print("TFT size is ");
  Serial.print(tft.width());
  Serial.print("x");
  Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325)
  {
    Serial.println(F("Found ILI9325 LCD driver"));
  }
  else if (identifier == 0x9328)
  {
    Serial.println(F("Found ILI9328 LCD driver"));
  }
  else if (identifier == 0x4535)
  {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }
  else if (identifier == 0x7575)
  {
    Serial.println(F("Found HX8347G LCD driver"));
  }
  else if (identifier == 0x9341)
  {
    Serial.println(F("Found ILI9341 LCD driver"));
  }
  else if (identifier == 0x8357)
  {
    Serial.println(F("Found HX8357D LCD driver"));
  }
  else if (identifier == 0x0101)
  {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  }
  else
  {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier = 0x9341;
  }

  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  uint16_t btnWidth = 110;
  uint16_t btnHeight = 60;
  uint16_t btnY = 90 + (btnHeight / 2);

  uint16_t tensileBtnX = 30 + (btnWidth / 2);
  uint16_t bendingBtnX = 180 + (btnWidth / 2);

  uint16_t joystickBtnX = 180 + (btnWidth / 2);
  uint16_t testingBtnX = 180 + (btnWidth / 2);

  // 30 pixels from the left including 1/2 width, 108 from the top including 1/2 height, 111 width, 60 height, outline, fill, text colour, text, text size
  buttons[1].initButton(&tft, tensileBtnX, btnY, btnWidth, btnHeight, ILI9341_WHITE, CYAN, ILI9341_BLACK, "Joystick", 2);
  buttons[2].initButton(&tft, bendingBtnX, btnY, btnWidth, btnHeight, ILI9341_WHITE, RED, ILI9341_BLACK, "Stop", 2);

  buttons[3].initButton(&tft, tensileBtnX, btnY + 85, btnWidth, btnHeight, ILI9341_WHITE, ILI9341_WHITE, ILI9341_BLACK, "Tensile", 2);
  buttons[4].initButton(&tft, bendingBtnX, btnY + 85, btnWidth, btnHeight, ILI9341_WHITE, ILI9341_WHITE, ILI9341_BLACK, "Bending", 2);
  buttons[1].drawButton(); // attempting to draw a button not in the loop

  delay(1000);
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000

bool bendingTestButtonPressed = false;
bool bendingTestStarted = false;

bool tensileTestButtonPressed = false;
bool tensileTestStarted = false;

bool joystickActive = false; // jos
bool tnbClickable = false; // t & b

void loop(void)
{

  static unsigned long time;
  static unsigned long next = 1000;
  static unsigned long last;

  // less then 1ms
  if (time == millis())
    return;

  time = millis();
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);

  if (time >= next)
  {
    DHT.read11(dht_apin);

    Serial.print("Current humidity = ");
    Serial.print(DHT.humidity);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(DHT.temperature);
    Serial.println("C  ");

    tft.print("Temperature: ");
    tft.print(DHT.temperature);
    tft.println("C");
    tft.print("Humidity: ");
    tft.print(DHT.humidity);
    tft.println("%");
  }

  last = (time / PERIOD) * PERIOD;
  next = last + PERIOD;

  tft.setCursor(0, 30);
  tft.println("Press 'Joystick' for adjusting the sample's position.");
  tft.println("Press 'Stop' when you are done and choose the type of test.");

  if (joystickActive)
  { // checking if joystick was pressed
    joystick();
  }

  if (tensileTestButtonPressed && !tensileTestStarted)
  {
    tensileTestStarted = true;
    tensileTest();
    Serial.println("Tensile test endend");
  }

  if (bendingTestButtonPressed && !bendingTestStarted)
  {
    bendingTestStarted = true;
    bendingTest();
  }

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {

    // landscape
    p.x = map(p.x, TS_TOP, TS_BOTTOM, tft.width(), 0);
    p.y = (tft.height() - map(p.y, TS_RIGHT, TS_LEFT, tft.height(), 0));

    Serial.print("X = ");
    Serial.print(p.x);
    Serial.print("\tY = ");
    Serial.print(p.y);
    Serial.print("\tPressure = ");
    Serial.println(p.z);
  }

  // go thru all the buttons, checking if they were pressed
  for (uint8_t b = 0; b < 5; b++) // b value changed from 2 to 3
  {
    if (buttons[b].contains(p.y, p.x))
    {
      Serial.print("Pressing: ");
      Serial.println(b);
      buttons[b].press(true); // tell the button it is pressed
    }
    else
    {
      buttons[b].press(false); // tell the button it is NOT pressed
    }
  }

  // Joystick button
  if (buttons[1].justReleased())
  {
    Serial.print("Released: ");
    Serial.println(1);
    buttons[1].drawButton(); // draw normal
  }

  if (buttons[1].justPressed())
  {
    buttons[1].drawButton(true); // draw invert!
    delay(500);
    buttons[2].drawButton(); // drawing "Stop" button
    joystickActive = true; // code for joystick on
  }
 
  if (buttons[2].justReleased())
  {
    Serial.print("Released: ");
    Serial.println(2);
    buttons[2].drawButton(); // draw normal
  }

  if (joystickActive)
  {
    if (buttons[2].justPressed())
    {
      buttons[2].drawButton(true); // draw invert!
      delay(500);
      joystickActive = false;
      buttons[2].drawButton();
      // code for joystick off
      buttons[3].drawButton(); // drawing tensile and bending buttons once stop is clicked
      buttons[4].drawButton();
      // Tensile & Bending clickable
      tnbClickable = true;
      tft.setCursor(147, 200);
      tft.setTextColor(WHITE);
      tft.println("or");
    }
  }

  for (uint8_t b = 3; b < 5; b++) // third loop for tensile/bending click
  {
    if (buttons[b].justReleased())
    {
      Serial.print("Released: ");
      Serial.println(b);
      buttons[b].drawButton();
    }

    if (tnbClickable)
    {
      if (buttons[b].justPressed())
      {
        buttons[b].drawButton(true); // draw invert!
        delay(500);
        tft.setCursor(80, 65);
        tft.setTextColor(RED);
        tft.println("Testing...");
        if (b == 4)
        {
          Serial.print("you pressed bending");
          // code for bending test
          bendingTestButtonPressed = true;
        }
        if (b == 3)
        {
          Serial.print("you pressed tensile");
          // code for tensile test
          tensileTestButtonPressed = true;
        }
      }
    }
  }
}
