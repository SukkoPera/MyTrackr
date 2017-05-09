#include <TimeLib.h>

// We want enums to only use 1 byte
#ifdef __GNUC__
	#define _PACKED_ __attribute__ ((packed))
#else
	#define _PACKED_
#endif

enum _PACKED_ Key {
	KEY_NONE,
	KEY_PREV,
	KEY_NEXT,
	KEY_SELECT,
	KEY_BACK
};

#ifdef ENABLE_ROTATION_MENU
enum _PACKED_ LogRotation {
	LOGROT_OFF = 0,
	LOGROT_HOURLY = 60,
	LOGROT_DAILY = 24,
	LOGROT_WEEKLY = 7,
	LOGROT_MONTHLY = 30
};
#endif

enum _PACKED_ DaylightSavingMode {
	DST_AUTO,
	DST_ON,
	DST_OFF
};

struct Position {
	float lat;
	float lon;
	boolean valid;
};

struct ValidFloat {
	float value;
	boolean valid;
};

struct GpsFix {
	Position pos;
	ValidFloat alt;			// Meters
	ValidFloat course;		// Degrees from True North
	ValidFloat speed;		// km/h
	ValidFloat hdop;
	TimeElements time;
	unsigned int nsats;
};
