#include <RFM69.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
// #include <Fonts/Org_01.h> https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

#include "serialSettings.h"
#include "pinDefs.h"
#include "lcdSettings.h"
#include "radioSettings.h"
#include "buttonSettings.h"

// Init Radio Object
RFM69 radio;

// Init LCD Object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
#ifdef SERIAL_EN
	Serial.begin(SERIAL_BAUD);
	delay(2000);
#endif

	// RADIO
	if (!radio.initialize(FREQUENCY, HUBID, NETWORKID))
		DEBUGln("radio.init() FAIL");
	else
		DEBUGln("radio.init() SUCCESS");

	radio.setHighPower();
	radio.setCS(RFM69_CS);

	// LCD
	tft.begin();
	tft.fillScreen(ILI9341_BLACK);
	tft.setRotation(DEFAULT_ROTATION);
	tft.cp437(true);
	yield();

	// Pins
	pinMode(BUTTON_INT, INPUT);
	pinMode(BUTTON_ANA, INPUT);
	attachInterrupt(digitalPinToInterrupt(BUTTON_INT), pin_ISR, FALLING);

	// DEBUG
	tft.setCursor(0, 0);
	tft.setTextSize(3);
}

buttonState button = none;
bool newData = false;

void loop()
{
	if (newData) // put your main code here, to run repeatedly:
	{
		tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
		DEBUGln(button);
		tft.setCursor(0, 0);
		tft.print(button);
		tft.print("     ");
		newData = false;
	}

	if (radio.receiveDone())
	{
		Payload payload;

		if (radio.DATALEN == sizeof(Payload))
			payload = *(Payload *)radio.DATA;

		if (radio.ACKRequested())
		{
			radio.sendACK();
			DEBUGln("ACK sent");
		}

		DEBUGln(payload.type);
		DEBUGln(payload.data);

		switch (payload.type)
		{
		case curr_temp:
			tft.setCursor(0, 32);
			tft.setTextColor(ILI9341_PINK, ILI9341_BLACK);
			tft.print(payload.data);
			tft.print((char)249);
			tft.print("F ");
			break;
		case target_temp:
			tft.setCursor(0, 64);
			tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
			tft.print(payload.data);
			tft.print((char)250);
			tft.print("F ");
			break;
		default:
			break;
		}
	}
}

void pin_ISR()
{
	int rawData = analogRead(BUTTON_ANA);

	if(rawData < BUTTON_1)
		button = down;
	else if(rawData < BUTTON_2)
		button = up;
	else if(rawData < BUTTON_3)
		button = left;
	else if(rawData < BUTTON_4)
		button = right;
	else
		button = none;

	newData = button != none;
}
