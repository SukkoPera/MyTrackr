// We want enums to only use 1 byte
#define _PACKED_ __attribute__ ((packed))

enum _PACKED_ Key {
	KEY_NONE,
	KEY_PREV,
	KEY_NEXT,
	KEY_SELECT,
	KEY_BACK
};

enum _PACKED_ LogRotation {
  LOGROT_OFF = 0,
  LOGROT_HOURLY = 60,
  LOGROT_DAILY = 24,
  LOGROT_WEEKLY = 7,
  LOGROT_MONTHLY = 30
};

enum _PACKED_ DaylightSavingMode {
  DST_AUTO,
  DST_ON,
  DST_OFF
};
