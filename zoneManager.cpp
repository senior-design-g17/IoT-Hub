#include "zoneManager.h"

zoneManager::zoneManager()
{
	zones.setStorage(data, (size_t)0);
	payloadQueue.setStorage(loads, (size_t)0);
}

zoneManager::~zoneManager()
{
	zones.~Vector();
	payloadQueue.~Vector();
}

void zoneManager::addZone()
{
	Zone zone;

	zones.push_back(zone);
}

void zoneManager::addZone(deviceType type, int deviceAddr)
{
	Zone zone;

	switch (type)
	{
	case vent:
		zone.vent = deviceAddr;
		break;
	case sensor:
		zone.sensor = deviceAddr;
		break;
	}

	zones.push_back(zone);
}

void zoneManager::addZone(int sensorAddr, int ventAddr)
{
	Zone zone;
	zone.vent = ventAddr;
	zone.sensor = sensorAddr;

	zones.push_back(zone);
}

// Returns false for invalid args
bool zoneManager::addToZone(int zoneID, deviceType type, int deviceAddr)
{
	if (zoneID >= zones.size())
		return false;

	switch (type)
	{
	case vent:
		zones[zoneID].vent = deviceAddr;
		break;
	case sensor:
		zones[zoneID].sensor = deviceAddr;
		break;
	default:
		return false;
		break;
	}

	return true;
}

// returns false for invalid args
bool zoneManager::updateZone(int zoneID, command type, int data)
{
	DEBUG("Zone ID: ");
	DEBUGln(zoneID);

	if (zoneID >= zones.size())
		return false;

	switch (type)
	{
	case curr_temp:
		zones[zoneID].currTemp = data;
		break;
	case target_temp:
		zones[zoneID].currTarget = data;
		break;
	case vent_state:
		zones[zoneID].ventState = (Vent_State)data;
	default:
		return false;
		break;
	}

	processData();

	return true;
}

Payload zoneManager::getPayload()
{
	Payload ret = payloadQueue.back();
	payloadQueue.pop_back();

	return ret;
}

bool zoneManager::havePayloads()
{
	return !payloadQueue.empty();
}

void zoneManager::processData()
{
	int needCool = 0;
	int needHeat = 0;

	// Find if we need heat or cool
	for (int i = 0; i < zones.size(); i++)
	{
		// Too hot turn on ac
		if (zones[i].currTemp > zones[i].currTarget)
		{
			needCool++;
		}
		// Too cold turn on heat
		else if (zones[i].currTemp < zones[i].currTarget)
		{
			needHeat++;
		}
	}

	// if all zones are good turn off
	if (needHeat == 0 && needCool == 0)
	{
		ACstate = off;

		Payload load;
		load.type = HVAC;
		load.data = off;

		payloadQueue.push_back(load);
	}
	// Only update AC state
	// Always when AC state is off
	// Only if either cooling/heating is no longer needed
	else if (ACstate == off || (needCool == 0 || needHeat == 0))
	{
		ACstate = needCool >= needHeat ? cool : heat;

		Payload load;
		load.zoneID = 0;
		load.type = HVAC;
		load.data = ACstate;

		payloadQueue.push_back(load);
	}

	// Update vent states
	for (int i = 0; i < zones.size(); i++)
	{
		Payload load;
		load.zoneID = zones[i].vent;
		load.type = vent_state;

		// Too hot turn on ac
		if (zones[i].currTemp > zones[i].currTarget)
		{
			// open vent if we're cooling
			if (ACstate == cool)
				load.data = open;

			// close otherwise
			else
				load.data = close;
		}
		// Too cold turn on heat
		else if (zones[i].currTemp < zones[i].currTarget)
		{
			// open vent if we're heat
			if (ACstate == heat)
				load.data = open;

			// close otherwise
			else
				load.data = close;
		}
		// equal means close
		else
		{
			load.data = close;
		}

		// Update only if it doesnt match current state
		if (load.data == zones[i].ventState)
		{
			payloadQueue.push_back(load);
			zones[i].ventState = (Vent_State)load.data;
		}
	}
}