#include "MenuHandler.h"

MenuHandler menuHandler;

class SwitcherMenuItem: public StaticMenuItem {
private:
	Menu menu;

public:
	SwitcherMenuItem (PGM_P _name, const Menu& _m): StaticMenuItem (_name), menu (_m) {
	}

	void activate (void) override {
		menuHandler.setCur (menu);
	}
};

/******************************************************************************/

// Forward declarations
extern MenuItem *topMenu[], *subMenuLogOpts[], *subMenuTimeOpts[];

// Item to get back to main menu
const char mlBack[] PROGMEM = "Back";
SwitcherMenuItem lbtop (mlBack, topMenu);


/*******************************************************************************
 * Log Options Menu
 ******************************************************************************/

SwitcherMenuItem lb (mlBack, subMenuLogOpts);

class MenuItemLogFreq: public StaticMenuItem {
private:
	int freq;

public:
	MenuItemLogFreq (PGM_P name, int f): StaticMenuItem (name), freq (f) {
	}

	void activate () override {
		logFreq = freq;

		DPRINT (F("Log Frequency is now "));
		DPRINT (logFreq);
		DPRINTLN (F(" seconds"));

		lb.activate ();
	}
};

class MenuItemLogDist: public StaticMenuItem {
private:
	int dist;

public:
	MenuItemLogDist (PGM_P name, int d): StaticMenuItem (name), dist (d) {
	}

	void activate () override {
		logDist = dist;

		DPRINT (F("Log Distance is now "));
		DPRINT (logDist);
		DPRINTLN (F(" meters"));

		lb.activate ();
	}
};

class MenuItemLogRot: public StaticMenuItem {
private:
	LogRotation rot;

public:
	MenuItemLogRot (PGM_P name, LogRotation r): StaticMenuItem (name), rot (r) {
	}

	void activate () override {
		logRot = rot;

		DPRINT (F("Log Rotation is now "));
		DPRINTLN (logRot);

		lb.activate ();
	}
};

const char mlf1[] PROGMEM = "1 second";
MenuItemLogFreq lf1 (mlf1, 1);
const char mlf5[] PROGMEM = "5 seconds";
MenuItemLogFreq lf5 (mlf5, 5);
const char mlf10[] PROGMEM = "10 seconds";
MenuItemLogFreq lf10 (mlf10, 10);
const char mlf30[] PROGMEM = "30 seconds";
MenuItemLogFreq lf30 (mlf30, 30);
const char mlf60[] PROGMEM = "1 minute";
MenuItemLogFreq lf60 (mlf60, 60);
MenuItem *subMenuLogFreq[] = {&lf1, &lf5, &lf10, &lf30, &lf60, &lb, NULL};

const char mlOff[] PROGMEM = "Off";
MenuItemLogDist ldOff (mlOff, 0);
const char ml10[] PROGMEM = "10 m";
MenuItemLogDist ld10 (ml10, 10);
const char ml100[] PROGMEM = "100 m";
MenuItemLogDist ld100 (ml100, 100);
const char ml250[] PROGMEM = "250 m";
MenuItemLogDist ld250 (ml250, 250);
const char ml1000[] PROGMEM = "1 km";
MenuItemLogDist ld1000 (ml1000, 1000);
MenuItem *subMenuLogDist[] = {&ldOff, &ld10, &ld100, &ld250, &ld1000, &lb, NULL};

MenuItemLogRot lrOff (mlOff, LOGROT_OFF);
const char mlH[] PROGMEM = "Hourly";
MenuItemLogRot lrH (mlH, LOGROT_HOURLY);
const char mlD[] PROGMEM = "Daily";
MenuItemLogRot lrD (mlD, LOGROT_DAILY);
const char mlW[] PROGMEM = "Weekly";
MenuItemLogRot lrW (mlW, LOGROT_WEEKLY);
const char mlM[] PROGMEM = "Monthly";
MenuItemLogRot lrM (mlM, LOGROT_MONTHLY);
MenuItem *subMenuLogRot[] = {&lrOff, &lrH, &lrD, &lrW, &lrM, &lb, NULL};

const char mlFreq[] PROGMEM = "Frequency";
SwitcherMenuItem loFreq (mlFreq, subMenuLogFreq);
const char mlDist[] PROGMEM = "Distance";
SwitcherMenuItem loDist (mlDist, subMenuLogDist);
const char mlRot[] PROGMEM = "Rotation";
SwitcherMenuItem loRot (mlRot, subMenuLogRot);
MenuItem *subMenuLogOpts[] = {&loFreq, &loDist, &loRot, &lbtop, NULL};


/*******************************************************************************
 * Time Options Menu
 ******************************************************************************/

auto ltb = SwitcherMenuItem (mlBack, subMenuTimeOpts);

class MenuItemTimeOffset: public StaticMenuItem {
private:
	int offset;

public:
	MenuItemTimeOffset (PGM_P name, int o): StaticMenuItem (name), offset (o) {
	}

	void activate () override {
		utcOffset = offset;

		DPRINT (F("UTC offset is now "));
		DPRINT (utcOffset);
		DPRINTLN (F(" hours"));

		ltb.activate ();
	}
};

class MenuItemTimeDst: public StaticMenuItem {
private:
	DaylightSavingMode mode;

public:
	MenuItemTimeDst (PGM_P name, DaylightSavingMode m): StaticMenuItem (name), mode (m) {
	}

	void activate () override {
		dstMode = mode;

		DPRINT (F("DST mode is now "));
		DPRINTLN (dstMode);

		ltb.activate ();
	}
};

const char mlAuto[] PROGMEM = "Auto";
MenuItemTimeDst da (mlAuto, DST_AUTO);
const char mlOn[] PROGMEM = "On";
MenuItemTimeDst don (mlOn, DST_ON);
MenuItemTimeDst doff (mlOff, DST_OFF);
MenuItem *subMenuDstMode[] = {&da, &don, &doff, &ltb, NULL};

const char mlOm12[] PROGMEM = "-12";
MenuItemTimeOffset om12 (mlOm12, -12);
const char mlOm11[] PROGMEM = "-11";
MenuItemTimeOffset om11 (mlOm11, -11);
const char mlOm10[] PROGMEM = "-10";
MenuItemTimeOffset om10 (mlOm10, -10);
const char mlOm9[] PROGMEM = "-9";
MenuItemTimeOffset om9 (mlOm9, -9);
const char mlOm8[] PROGMEM = "-8";
MenuItemTimeOffset om8 (mlOm8, -8);
const char mlOm7[] PROGMEM = "-7";
MenuItemTimeOffset om7 (mlOm7, -7);
const char mlOm6[] PROGMEM = "-6";
MenuItemTimeOffset om6 (mlOm6, -6);
const char mlOm5[] PROGMEM = "-5";
MenuItemTimeOffset om5 (mlOm5, -5);
const char mlOm4[] PROGMEM = "-4";
MenuItemTimeOffset om4 (mlOm4, -4);
const char mlOm3[] PROGMEM = "-3";
MenuItemTimeOffset om3 (mlOm3, -3);
const char mlOm2[] PROGMEM = "-2";
MenuItemTimeOffset om2 (mlOm2, -2);
const char mlOm1[] PROGMEM = "-1";
MenuItemTimeOffset om1 (mlOm1, -1);
const char mlO0[] PROGMEM = "0";
MenuItemTimeOffset o0 (mlO0, 0);
const char mlOp1[] PROGMEM = "+1";
MenuItemTimeOffset op1 (mlOp1, +1);
const char mlOp2[] PROGMEM = "+2";
MenuItemTimeOffset op2 (mlOp2, +2);
const char mlOp3[] PROGMEM = "+3";
MenuItemTimeOffset op3 (mlOp3, +3);
const char mlOp4[] PROGMEM = "+4";
MenuItemTimeOffset op4 (mlOp4, +4);
const char mlOp5[] PROGMEM = "+5";
MenuItemTimeOffset op5 (mlOp5, +5);
const char mlOp6[] PROGMEM = "+6";
MenuItemTimeOffset op6 (mlOp6, +6);
const char mlOp7[] PROGMEM = "+7";
MenuItemTimeOffset op7 (mlOp7, +7);
const char mlOp8[] PROGMEM = "+8";
MenuItemTimeOffset op8 (mlOp8, +8);
const char mlOp9[] PROGMEM = "+9";
MenuItemTimeOffset op9 (mlOp9, +9);
const char mlOp10[] PROGMEM = "+10";
MenuItemTimeOffset op10 (mlOp10, +10);
const char mlOp11[] PROGMEM = "+11";
MenuItemTimeOffset op11 (mlOp11, +11);
const char mlOp12[] PROGMEM = "+12";
MenuItemTimeOffset op12 (mlOp12, +12);
MenuItem *subMenuUtcOffset[] = {
	&om12, &om11, &om10, &om9, &om8, &om7, &om6, &om5, &om4, &om3, &om2, &om1,
	&o0,
	&op1, &op2, &op3, &op4, &op5, &op6, &op7, &op8, &op9, &op10, &op11, &op12,
	&ltb,
	NULL
};

const char mlUtcOffset[] PROGMEM = "UTC Offset";
SwitcherMenuItem toOff (mlUtcOffset, subMenuUtcOffset);
const char mlDst[] PROGMEM = "Daylight Saving";
SwitcherMenuItem toDst (mlDst, subMenuDstMode);
MenuItem *subMenuTimeOpts[] = {&toOff, &toDst, &lbtop, NULL};


/*******************************************************************************
 * Main Menu
 ******************************************************************************/

class StartStopMenuItem: public MenuItem {
public:
	FlashString getName (void) const override {
		if (logEnabled)
			return F("Stop");
		else
			return F("Start");
	}

	void activate (void) override {
		logEnabled = !logEnabled;

		DPRINT (F("Logging is now "));
		DPRINTLN (logEnabled ? F("ON") : F("OFF"));
	}
};

class ExitMenuItem: public StaticMenuItem {
public:
	ExitMenuItem (): StaticMenuItem (PSTR("Exit")) {
	}

	void activate (void) override {
		menuHandler.show = false;
	}
};

StartStopMenuItem ss;
const char mlLogOpts[] PROGMEM = "Log Options";
SwitcherMenuItem lo (mlLogOpts, subMenuLogOpts);
const char mlTimeOpts[] PROGMEM = "Time Options";
SwitcherMenuItem to (mlTimeOpts, subMenuTimeOpts);
ExitMenuItem ex;
MenuItem *topMenu[] = {&ss, &lo, &to, &ex, NULL};
