#include <FastLED.h>
#include <string.h>
#define LED_PIN 5
#define NUM_LEDS 150
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType currentBlending;
CRGB leds[NUM_LEDS];

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
extern const TProgmemPalette16 myGreenPurplePalette_p PROGMEM;

const long interval = 200;
unsigned long previousMillis = 0;  

bool useHex = false;
const byte numChars = 64;
char receivedChars[numChars];   // an array to store the received data
bool newData = false;
bool pauseLights = false; 
bool useRotatingColors = false; 
int currentHue = 200;
enum hexKeys : char {
	zero = '0',
	one = '1',
	two = '2',
	three = '3',
	four = '4',
	five = '5',
	six = '6',
	seven = '7',
	eight = '8',
	nine = '9',
	hex = '#',
	a = 'a',
	b = 'b',
	c = 'c',
	d = 'd',
	e = 'e',
	f = 'f'
};

void setup() {

	delay(3000); // power-up safety delay
	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(BRIGHTNESS);
	Serial.begin(9600);
	currentPalette = RainbowColors_p;
	currentBlending = LINEARBLEND;
	static uint8_t startIndex = 0;
	startIndex = startIndex + 1; /* motion speed */
	fillLEDsFromPaletteColors(startIndex);
	FastLED.show();
}

void loop() {
  	recvWithEndMarker();
    showNewData();
}

void fillLEDsFromPaletteColors(uint8_t colorIndex) {
	uint8_t brightness = 255;
  	for (int i = 0; i < NUM_LEDS; i++) {
    	leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    	colorIndex += 3;
  	}
}


void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
   
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void showNewData()
{
  	if(newData) {
		if(receivedChars[0] == 's') {
			pauseLights = true;
		} else if(receivedChars[0] == 'g'){
			pauseLights = false;
		} else if(pauseLights) {
			pickColor();
		}
		newData = false;
  	}
  	if(!pauseLights) {
		if(!useHex && !useRotatingColors) {
			static uint8_t startIndex = 0;
			startIndex = startIndex + 1; /* motion speed */
			fillLEDsFromPaletteColors(startIndex);
		} else if(useRotatingColors && !useHex) {
			// rotateColor();
		}

		FastLED.delay(1000 / UPDATES_PER_SECOND);
		FastLED.show();
  	}
}

void pickColor() {
    char firstCharacter = receivedChars[0];
    if(firstCharacter != 's' &&  firstCharacter != 'g') {
		useRotatingColors = false;
		useHex = false;
		if(firstCharacter == hex) {
			setUpRGB();
		} else {
			selectPreset();
		}
    } 
}

void setUpRGB() {
	useRotatingColors = false; // just in case
  	useHex = true;
  	int red = parseColorToInt(1, 3);
    int green = parseColorToInt(4, 6);
    int blue = parseColorToInt(7, 9);
  	CRGB color = CRGB(red, green, blue);
    fill_solid(leds, NUM_LEDS, color);
}

// void rotateColor() {
// 	if(checkInterval()) {
// 		if(currentHue == 255) {
// 			currentHue = 0;
// 		} else {
// 			currentHue++;
// 		}
// 		CRGB rbgColor = CRGB(currentHue, 0, 0);
// 		fill_solid(leds, NUM_LEDS, rbgColor);
// 	}
// }

bool checkInterval() {
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;
		return true;
	}
	return false;
}

void selectPreset() {
  char firstCharacter = receivedChars[0];
  switch (firstCharacter)
  {
    case one:
		currentPalette = RainbowColors_p;
		currentBlending = LINEARBLEND;
		break;
    case two:
		currentPalette = RainbowStripeColors_p;
		currentBlending = NOBLEND;
		break;
    case three:
		currentPalette = RainbowStripeColors_p;
		currentBlending = LINEARBLEND;
		break;
    case four:
		currentPalette = myGreenPurplePalette_p;
		currentBlending = LINEARBLEND;
		break;
    case five:
		SetupTotallyRandomPalette();
		currentBlending = LINEARBLEND;
		break;
    case six:
		SetupBlackAndWhiteStripedPalette();
		currentBlending = NOBLEND;
		break;
    case seven:
		currentPalette = CloudColors_p;
		currentBlending = LINEARBLEND;
		break;
    case eight:
		currentPalette = PartyColors_p;
		currentBlending = LINEARBLEND;
		break;
    case nine:
		currentPalette = myRedWhiteBluePalette_p;
		currentBlending = LINEARBLEND;
		break;
    case zero:
		currentPalette = LavaColors_p;
		currentBlending = LINEARBLEND;
		break;
    case a:
		currentPalette = OceanColors_p;
		currentBlending = LINEARBLEND;
		break;
    case b:
		currentPalette = ForestColors_p;
		currentBlending = LINEARBLEND;
		break;
    case c:
		currentPalette = HeatColors_p;
		currentBlending = LINEARBLEND;
		break;
    case d:
		useRotatingColors = true;
      	break;
    default: 
      	break;
  }
}

int parseColorToInt(int start, int end) {
	int size = end - start;
	char colourString[size];
	for(int i = 0; i <= size; i++) {
		colourString[i] = receivedChars[i + start];
	}
	int colour = atoi(colourString);
	return colour;
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
	for (int i = 0; i < 16; i++)
	{
		currentPalette[i] = CRGB(random8(), random8(), random8());
	}
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
	// 'black out' all 16 palette entries...
	fill_solid(currentPalette, 16, CRGB::Black);
	// and set every fourth one to white.
	currentPalette[0] = CRGB::White;
	currentPalette[4] = CRGB::White;
	currentPalette[8] = CRGB::White;
	currentPalette[12] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  	CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  	CRGB green = CHSV(HUE_GREEN, 255, 255);
	CRGB black = CRGB::Black;

	currentPalette = CRGBPalette16(
		green, green, black, black,
		purple, purple, black, black,
		green, green, black, black,
		purple, purple, black, black);
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
  {
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black};

const TProgmemPalette16 myGreenPurplePalette_p PROGMEM =
  {
    CRGB::Green,
    CRGB::Green,
    CRGB::Black,
    CRGB::Black,

    CRGB::Purple,
    CRGB::Purple,
    CRGB::Black,
    CRGB::Black,

    CRGB::Green,
    CRGB::Green,
    CRGB::Black,
    CRGB::Black,
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Black,
    CRGB::Black};
