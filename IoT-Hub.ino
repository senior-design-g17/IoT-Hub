#include <RFM69.h>
#include <SPI.h>
#include <Adafruit_GFX.h> //https://learn.adafruit.com/adafruit-gfx-graphics-library
#include <Adafruit_ILI9341.h>

// #include "payload.h" // payload is inherrited from zoneManager
#include "serialSettings.h"
#include "pinDefs.h"
#include "lcdSettings.h"
#include "radioSettings.h"
#include "buttonSettings.h"
#include "zoneManager.h"

// Init Radio Object
RFM69 radio;

// Init LCD Object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

zoneManager ZONE_MANAGER;

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
	pinMode(AC_FAN, OUTPUT);
	pinMode(AC_HEAT, OUTPUT);
	pinMode(AC_COOL, OUTPUT);
	pinMode(TFT_BL, OUTPUT);
	digitalWrite(TFT_BL, HIGH);

	attachInterrupt(digitalPinToInterrupt(BUTTON_INT), pin_ISR, FALLING);

	// DEBUG
	tft.setCursor(0, 0);
	tft.setTextSize(3);

	ZONE_MANAGER.addZone(2, 3);
}

buttonState button = none;
bool newData = false;

void loop()
{
	// button handling
	if (newData)
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
			tft.print((char)248);
			tft.print("F ");
			break;
		default:
			break;
		}

		ZONE_MANAGER.updateZone(payload.zoneID, payload.type, payload.data);
	}

	if (ZONE_MANAGER.havePayloads())
	{
		Payload comm = ZONE_MANAGER.getPayload();

		if (comm.type == HVAC)
		{
			tft.setCursor(0, 128);

			switch ((HVAC_State)comm.data)
			{
			case off:
				tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
				tft.print("OFF ");
				break;
			case heat:
				tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
				tft.print("HEAT");
				break;
			case cool:
				tft.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
				tft.print("COOL");
				break;
			}

			setHVACpins((HVAC_State)comm.data);
		}
		else if (comm.type == vent_state)
		{
			tft.setCursor(0, 96);

			switch ((Vent_State)comm.data)
			{
			case open:
				tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
				tft.print("Open ");
				break;
			case close:
				tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
				tft.print("Close");
				break;
			}

			sendPayload(comm);
		}
	}
}

void pin_ISR()
{
	int rawData = analogRead(BUTTON_ANA);

	if (rawData < BUTTON_1)
		button = down;
	else if (rawData < BUTTON_2)
		button = up;
	else if (rawData < BUTTON_3)
		button = left;
	else if (rawData < BUTTON_4)
		button = right;
	else
		button = none;

	newData = button != none;
}

void setHVACpins(HVAC_State state)
{
	switch (state)
	{
	case off:
		digitalWrite(AC_FAN, LOW);
		digitalWrite(AC_HEAT, LOW);
		digitalWrite(AC_COOL, LOW);
		break;
	case heat:
		digitalWrite(AC_FAN, HIGH);
		digitalWrite(AC_HEAT, HIGH);
		digitalWrite(AC_COOL, LOW);
		break;
	case cool:
		digitalWrite(AC_FAN, HIGH);
		digitalWrite(AC_HEAT, LOW);
		digitalWrite(AC_COOL, HIGH);
		break;
	}
}

bool sendPayload(Payload load)
{
	DEBUGln(load.zoneID);
	DEBUGln(load.type);
	DEBUGln(load.data);

	if (radio.sendWithRetry(load.zoneID, (const void *)(&load), sizeof(load), RETRY_COUNT, RETRY_WAIT))
	{
		DEBUGln("ACK received!");
		return false;
	}

	return true;
}