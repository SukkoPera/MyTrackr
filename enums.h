enum Key {
	KEY_NONE,
	KEY_PREV,
	KEY_NEXT,
	KEY_SELECT,
	KEY_BACK
};

enum LogRotation {
  LOGROT_OFF = 0,
  LOGROT_HOURLY = 60,
  LOGROT_DAILY = 24,
  LOGROT_WEEKLY = 7,
  LOGROT_MONTHLY = 30
};

enum DaylightSavingMode {
  DST_AUTO,
  DST_ON,
  DST_OFF
};
