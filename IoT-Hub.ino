#include <RFM69.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include "serialSettings.h"
#include "pinDefs.h"
#include "lcdSettings.h"
#include "radioSettings.h"

char buff[61]; //61 max payload for radio

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

int i = 0;
bool newData = false;

void loop()
{
	if (newData) // put your main code here, to run repeatedly:
	{
		tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
		DEBUGln(i);
		tft.setCursor(0, 32);
		tft.println(i);
		newData = false;
	}

	if (radio.receiveDone())
	{
		int DATALEN = radio.DATALEN;
		uint8_t DATA[DATALEN];

		for (byte i = 0; i < radio.DATALEN; i++)
		{
			DATA[i] = radio.DATA[i];
		}

		if (radio.ACKRequested())
		{
			radio.sendACK();
			DEBUGln("ACK sent");
		}

		tft.setCursor(0, 0);
		tft.setTextColor(ILI9341_DARKCYAN, ILI9341_BLACK);

		for (byte i = 0; i < DATALEN; i++)
		{
			tft.print((char)DATA[i]);
			DEBUG((char)DATA[i]);
		}
		tft.print(' ');

		tft.println();
		DEBUGln();
	}
}

void pin_ISR()
{
	newData = true;
	i++;
}
