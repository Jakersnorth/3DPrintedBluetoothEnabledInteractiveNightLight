// John's note, this program is modified from the NeoPixel example code installed using the Arduino Library Installer
// More detail on NeoPixel's and this library can be found here
// https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-installation

// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6

const int RAINBOW_CONTROL_PIN = 8;
const int THEATER_CONTROL_PIN = 12;
const int PULSE_CONTROL_PIN = 10;
const int FORWARD_CONTROL_PIN = 4;
const int BACKWARD_CONTROL_PIN = 5;
const int POWER_CONTROL_PIN = 3;

const int LEFT_CLOUD_PIN = A3;
const int RIGHT_CLOUD_PIN = A5;

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 60

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

uint8_t currRed = 255;
uint8_t currGreen = 0;
uint8_t currBlue = 0;

int moveInd = 0;

bool poweredState = false;
bool prevPowerPin = false;

// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  Serial.begin(9600);
  //Serial1.begin(9600);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  pinMode(RAINBOW_CONTROL_PIN, INPUT_PULLUP);
  pinMode(THEATER_CONTROL_PIN, INPUT_PULLUP);
  pinMode(PULSE_CONTROL_PIN, INPUT_PULLUP);  
  pinMode(FORWARD_CONTROL_PIN, INPUT_PULLUP);
  pinMode(BACKWARD_CONTROL_PIN, INPUT_PULLUP);
  pinMode(POWER_CONTROL_PIN, INPUT_PULLUP);
  pinMode(LEFT_CLOUD_PIN, OUTPUT);
  pinMode(RIGHT_CLOUD_PIN, OUTPUT);
}

void loop() {
  /*
    This code was a quick test to see if I could send color picker data
    from the feather to the leonardo, due to time constraints I did not have a chance
    to implement this functionality
    if(Serial1.available() > 0) {
    currRed = (uint8_t)Serial1.read();
    currGreen = (uint8_t)Serial1.read();
    currBlue = (uint8_t)Serial1.read();
    Serial.println(currRed);
    Serial.println(currGreen);
    Serial.println(currBlue);
  }
  */

  //checks if power button on bluetooth device has been pressed
  bool currPowerControl = digitalRead(POWER_CONTROL_PIN);
  //checks if first instance of current press and sets power state of neopixel accordingly
  if(currPowerControl && !prevPowerPin) {
    poweredState = !poweredState;
    prevPowerPin = true;
    if(poweredState) {
      staticColor(strip.Color(currRed, currGreen, currBlue));
      analogWrite(LEFT_CLOUD_PIN, 255);
      analogWrite(RIGHT_CLOUD_PIN, 255);
    }
    //if signal was to turn off lights, clears neopixel and turns off cloud LEDs
    else {  
      strip.clear();
      strip.show();
      analogWrite(LEFT_CLOUD_PIN, 0);
      analogWrite(RIGHT_CLOUD_PIN, 0);
    }
  }
  //refreshes button press tracking 
  else if(!currPowerControl && prevPowerPin) {
    prevPowerPin = false;
  }

  //checks if the lights are supposed to be on before animating
  if(poweredState) {
    if(digitalRead(RAINBOW_CONTROL_PIN)) {
      theaterChaseRainbow(50);
    } else if(digitalRead(THEATER_CONTROL_PIN)) {
      colorWipe(strip.Color(currRed, currGreen, currBlue), 50);
    } else if(digitalRead(PULSE_CONTROL_PIN)) {
      colorPulse(rgbToHue(currRed, currGreen, currBlue), 10);
    } else if(digitalRead(FORWARD_CONTROL_PIN)) {
      Serial.println("move forward");
      colorMove(strip.Color(currRed, currGreen, currBlue), 1, 50);
    } else if(digitalRead(BACKWARD_CONTROL_PIN)) {
      Serial.println("move backward");
      colorMove(strip.Color(currRed, currGreen, currBlue), -1, 50);
    } 
  }
}


// Some functions from example code for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  strip.clear();
  strip.show();
  int alternateTimer = 0;
  bool leftOn = false;
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match

    //makes the cloud LEDs alternate blinking
    alternateTimer++;
    if(alternateTimer > 10) {
      alternateTimer = 0;
      leftOn = !leftOn;
      analogWrite(LEFT_CLOUD_PIN, leftOn ? 255 : 0);
      analogWrite(RIGHT_CLOUD_PIN, leftOn ? 0 : 255);
    }
    
    delay(wait);                           //  Pause for a moment
  }
  analogWrite(LEFT_CLOUD_PIN, 255);
  analogWrite(RIGHT_CLOUD_PIN, 255);
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

// End of original example code functions, everything below was created by John Akers

//animates a slow dimming and brightening effect
//uses HSV to manipulate brightness without using NeoPixel setBrightness function
//which is suggested to not be be used outside of setup function
void colorPulse(float hue, int wait) {
  int remappedHue = (int)(hue * 65536);
  int value = 255;
  int saturation = 255;
  //repeat pulse pattern 3 times
  for(int i = 0; i < 3; i++) {
    //lower the brightness gradually to 0
    for(int j = 0; j < 255; j++) {
      uint32_t color = strip.gamma32(strip.ColorHSV(hue, saturation, value));
      value--;
      strip.fill(color);
      strip.show();
      
      analogWrite(LEFT_CLOUD_PIN, 255 - j);
      analogWrite(RIGHT_CLOUD_PIN, 255 - j);
      delay(wait);
    }
    //raise the brightness gradually to 255
    for(int k = 0; k < 255; k++) {
      uint32_t color = strip.gamma32(strip.ColorHSV(hue, saturation, value));
      value++;
      strip.fill(color);
      strip.show();
      
      analogWrite(LEFT_CLOUD_PIN, value);
      analogWrite(RIGHT_CLOUD_PIN, value);
      delay(wait);
    }
  }
}

//lights a single light on the strip at a time and moves one step based on provided direction
void colorMove(uint32_t color, int dir, int wait) {
  moveInd = moveInd + dir;
  moveInd = moveInd < 0 ? strip.numPixels() - 1 : moveInd;
  moveInd = moveInd > strip.numPixels() - 1 ? 0 : moveInd;
  if(dir == 1) {
    analogWrite(LEFT_CLOUD_PIN, 255);
    analogWrite(RIGHT_CLOUD_PIN, 0);
  } else {
    analogWrite(LEFT_CLOUD_PIN, 0);
    analogWrite(RIGHT_CLOUD_PIN, 255);
  }
  strip.setPixelColor(moveInd, color);
  for(int i = 0; i < strip.numPixels(); i++) {
    if(i != moveInd) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }
  strip.show();
  delay(wait);
}

//fills the strip was single static color, mainly just for readability
void staticColor(uint32_t color) {
  strip.fill(color);
  strip.show();
}

//rgb to hue calculation modified from https://stackoverflow.com/questions/23090019/fastest-formula-to-get-hue-from-rgb
float rgbToHue(uint8_t r, uint8_t g, uint8_t b) {
  float rS = ((float)r) / 255.0f;
  float gS = ((float)g) / 255.0f;
  float bS = ((float)b) / 255.0f;

  float maxC  = max(max(rS, gS), bS);
  float minC = min(min(rS, gS), bS);

  if(maxC == rS) {
    return (abs(gS - bS) / (maxC - minC)) / 6.0f;
  } else if(maxC == gS) {
    return (2.0f + (abs(bS - rS)/(maxC - minC))) / 6.0f;
  } else {
    return (4.0f + (abs(rS - gS)/(maxC - minC))) / 6.0f;
  }  
}
