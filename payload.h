// Command Types
enum command : uint8_t
{
	curr_temp,
	target_temp,
	vent_state,
	HVAC
};

// HVAC commands
enum HVAC_State : uint8_t
{
	heat,
	cool,
	off
};

// Vent commands
enum Vent_State : uint8_t
{
	open,
	close
};

// Payload Struct Defintion
typedef struct
{
	uint8_t zoneID;
	command type;
	uint8_t data;
} Payload;
