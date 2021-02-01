#include <Vector.h>
#include "payload.h"

// Debug
#include "serialSettings.h"

#define PAYLOAD_MAX_QUEUE ZONE_MAX_COUNT * 2
#define ZONE_MAX_COUNT 15
#define AC_MARGIN 2

enum deviceType
{
	sensor,
	vent
};

typedef struct
{
	int currTemp;
	int currTarget;
	Vent_State ventState;
	int vent;
	int sensor;
} Zone;

class zoneManager
{
private:
	HVAC_State ACstate;
	Vector<Zone> zones;
	Vector<Payload> payloadQueue;
	void processData();
	Zone data[ZONE_MAX_COUNT];
	Payload loads[PAYLOAD_MAX_QUEUE];

public:
	zoneManager();
	~zoneManager();
	void addZone();
	void addZone(deviceType type, int deviceAddr);
	void addZone(int sensorAddr, int ventAddr);
	bool addToZone(int zoneID, deviceType type, int deviceAddr);
	bool updateZone(int zoneID, command type, int data);
	Payload getPayload();
	bool havePayloads();
};
