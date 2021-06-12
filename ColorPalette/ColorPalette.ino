#include <FastLED.h>
#include <string.h>

// define LED configuration
#define NUM_LEDS 122
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

// define pins
#define LED_PIN 5

// palette enum mapping encoder to palettes
enum colorPalettes : char
{
	rainbow = '0',
	rainbowStripeNB = '1',
	rainbowStripeLB = '2',
	greenNPurple = '3',
	randCols = '4',
	blackNWhite = '5',
	cloud = '6',
	party = '7',
	america = '8',
	lava = '9',
	ocean = 'a',
	forest = 'b',
	heat = 'c',
	lightning = 'd',
	white = 'e',
	rotate = 'f',
	hex = '#',
};

enum modes : char
{
	stop = 's',
	go = 'g',
};

CRGBPalette16 currentPalette;
TBlendType currentBlending;
CRGB leds[NUM_LEDS];

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
extern const TProgmemPalette16 myGreenPurplePalette_p PROGMEM;
extern const TProgmemPalette16 lightningPalette PROGMEM;
extern const TProgmemPalette16 whitePalette PROGMEM;

// green and purple palette
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

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM = {
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

// blue and yellow palette
const TProgmemPalette16 lightningPalette PROGMEM = {
	CRGB::Blue,
	CRGB::Blue,
	CRGB::Blue,
	CRGB::Blue,

	CRGB::Blue,
	CRGB::Blue,
	CRGB::Yellow,
	CRGB::Yellow,

	CRGB::Blue,
	CRGB::Blue,
	CRGB::Blue,
	CRGB::Blue,

	CRGB::Blue,
	CRGB::Blue,
	CRGB::Yellow,
	CRGB::Yellow};

// all white, didnt use hues because im lazy
const TProgmemPalette16 whitePalette PROGMEM = {
	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,

	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,

	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,

	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray,
	CRGB::Gray};

class TimeUtils
{
public:
	unsigned long previousMillis = 0;
	static const long interval = 200;

	bool checkInterval()
	{
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval)
		{
			previousMillis = currentMillis;
			return true;
		}
		return false;
	}
};

class StreamUtils
{
public:
	bool newData = false;
	static const byte numChars = 64;
	char receivedChars[numChars];

	void readStreamForString()
	{
		static byte ndx = 0;
		char endMarker = '\n';
		char rc;

		while (Serial.available() > 0 && newData == false)
		{
			rc = Serial.read();

			if (rc != endMarker)
			{
				receivedChars[ndx] = rc;
				ndx++;
				if (ndx >= numChars)
				{
					ndx = numChars - 1;
				}
			}
			else
			{
				receivedChars[ndx] = '\0'; // terminate the string
				ndx = 0;
				newData = true;
			}
		}
	}

	char getRecievedChar()
	{
		return receivedChars[0];
	}

	/**
	 * parses a substring to an int
	 * */
	int parseStringToInt(int start, int end)
	{
		int size = end - start;
		char string[size];
		for (int i = 0; i <= size; i++)
		{
			string[i] = receivedChars[i + start];
		}
		int convertedString = atoi(string);
		return convertedString;
	}

	void printCharacters()
	{
		Serial.print("This just in ... ");
		Serial.println(receivedChars);
	}
};

// not in header im lazy
class LightController
{
public:
	int colourStep;
	int paletteStep;
	int currentHue = 0;
	bool usingColour = false;
	bool rotating = false;
	int red = 255;
	int green = 255;
	int blue = 255;
	TimeUtils timer;
	StreamUtils stream;
	bool pauseLights = false;

	void checkForNewData()
	{
		stream.readStreamForString();
		if (stream.newData == true)
		{
			stream.newData = false;
			char newSetting = stream.getRecievedChar();
			setLightMode(newSetting);
		}
	}
	/**
   * sets the lights to their proper values, called every loop
   */
	void setLights()
	{
		if (rotating)
		{
			rotateHue();
		}

		if (usingColour)
		{
			setRGB();
		}

		if (!usingColour && !rotating)
		{
			static uint8_t startIndex = 0;
			startIndex = startIndex + 1; /* motion speed */
			fillLEDsFromPaletteColors(startIndex);
		}
	}

	/**
   * fills the leds with the current palette colours
   */
	void fillLEDsFromPaletteColors(uint8_t colorIndex)
	{
		uint8_t brightness = 255;
		for (int i = 0; i < NUM_LEDS; i++)
		{
			leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
			colorIndex += 3;
		}
	}
	/**
   * Sets the current light palette when the palette step changes
   */
	void setLightPalette(char paletteSetting)
	{
		usingColour = false;
		rotating = false;
		switch (paletteSetting)
		{
		case rainbow:
			currentPalette = RainbowColors_p;
			currentBlending = LINEARBLEND;
			break;
		case rainbowStripeNB:
			currentPalette = RainbowStripeColors_p;
			currentBlending = NOBLEND;
			break;
		case rainbowStripeLB:
			currentPalette = RainbowStripeColors_p;
			currentBlending = LINEARBLEND;
			break;
		case greenNPurple:
			currentPalette = myGreenPurplePalette_p;
			currentBlending = LINEARBLEND;
			break;
		case randCols:
			setupTotallyRandomPalette();
			currentBlending = LINEARBLEND;
			break;
		case blackNWhite:
			setupBlackAndWhiteStripedPalette();
			currentBlending = NOBLEND;
			break;
		case cloud:
			currentPalette = CloudColors_p;
			currentBlending = LINEARBLEND;
			break;
		case party:
			currentPalette = PartyColors_p;
			currentBlending = LINEARBLEND;
			break;
		case america:
			currentPalette = myRedWhiteBluePalette_p;
			currentBlending = LINEARBLEND;
			break;
		case lava:
			currentPalette = LavaColors_p;
			currentBlending = LINEARBLEND;
			break;
		case ocean:
			currentPalette = OceanColors_p;
			currentBlending = LINEARBLEND;
			break;
		case forest:
			currentPalette = ForestColors_p;
			currentBlending = LINEARBLEND;
			break;
		case heat:
			currentPalette = HeatColors_p;
			currentBlending = LINEARBLEND;
			break;
		case lightning:
			currentPalette = lightningPalette;
			currentBlending = LINEARBLEND;
			break;
		case white:
			currentPalette = whitePalette;
			currentBlending = LINEARBLEND;
			break;
		default:
			break;
		}
	}

	void setLightMode(char setting)
	{
		switch (setting)
		{
		case hex:
			usingColour = true;
			rotating = false;
			setupRGB();
			break;
		case rotate:
			rotating = true;
			usingColour = false;
			setupRotation();
			break;
		case stop:
			pauseLights = true;
			break;
		case go:
			pauseLights = false;
			break;
		default:
			setLightPalette(setting);
			break;
		}
	}

	/**
   * sets all the lights to the given hue
   */
	void useHSV(int hue)
	{
		CRGB hsvColour = CHSV(hue, 255, 255);
		fill_solid(leds, NUM_LEDS, hsvColour);
	}

	void setupRotation()
	{
		currentHue = 0;
		useHSV(currentHue);
	}

	void rotateHue()
	{
		if (timer.checkInterval())
		{
			if (currentHue == 255)
			{
				currentHue = 0;
			}
			else
			{
				currentHue++;
			}
			useHSV(currentHue);
		}
	}

	// This function fills the palette with totally random colors. (STOLEN) (FASTLED example code)
	void setupTotallyRandomPalette()
	{
		for (int i = 0; i < 16; i++)
		{
			currentPalette[i] = CRGB(random8(), random8(), random8());
		}
	}

	// This function sets up a palette of black and white stripes,
	// using code.  Since the palette is effectively an array of
	// sixteen CRGB colors, the various fill_* functions can be used
	// to set them up. (STOLEN) (FASTLED example code)
	void setupBlackAndWhiteStripedPalette()
	{
		// 'black out' all 16 palette entries...
		fill_solid(currentPalette, 16, CRGB::Black);
		// and set every fourth one to white.
		currentPalette[0] = CRGB::White;
		currentPalette[4] = CRGB::White;
		currentPalette[8] = CRGB::White;
		currentPalette[12] = CRGB::White;
	}

	void setupRGB()
	{
		red = stream.parseStringToInt(1, 3);
		green = stream.parseStringToInt(4, 6);
		blue = stream.parseStringToInt(7, 9);
		setRGB();
	}

	void setRGB()
	{
		CRGB color = CRGB(red, green, blue);
		fill_solid(leds, NUM_LEDS, color);
	}
};

LightController lightController;

/**
 * arduino setup function
 */
void setup()
{
	delay(3000); // power-up safety delay
	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	Serial.begin(9600);
	FastLED.setBrightness(BRIGHTNESS);
	currentPalette = RainbowColors_p;
	currentBlending = LINEARBLEND;
	static uint8_t startIndex = 0;
	startIndex = startIndex + 1; /* motion speed */
	lightController.fillLEDsFromPaletteColors(startIndex);
	FastLED.show();
}

/**
 * arduino loop function
 */
void loop()
{
	lightController.checkForNewData();
	if (!lightController.pauseLights)
	{
		lightController.setLights();

		FastLED.delay(1000 / UPDATES_PER_SECOND);
		FastLED.show();
	}
}
