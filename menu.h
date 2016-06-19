#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)
#define F_TO_PSTR(s) reinterpret_cast<PGM_P> (s)
#define FlashString const __FlashStringHelper *

class MenuItem {
public:
	virtual FlashString getName (void) const = 0;

	virtual void activate (void) {
	}
};

typedef MenuItem** Menu;

class StaticMenuItem: public MenuItem {
public:
	FlashString name;

	StaticMenuItem (PGM_P _name): name (PSTR_TO_F (_name)) {
	}

	FlashString getName (void) const override {
		return name;
	}
};


class MenuHandler {
private:
	Menu cur;
	byte nLines;	// No. of lines on screen
	byte curItem;
	byte firstItem;

public:
	boolean show;

	void begin (const Menu& m, byte _nLines, boolean _show = false) {
		nLines = _nLines;
		show = _show;
		setCur (m);
	}

	void setCur (const Menu& m) {
		cur = m;
		curItem = 0;
		firstItem = 0;
	}

	void draw () {
		if (show) {
			u8g.setFont (u8g_font_6x10);
			u8g.setFontRefHeightExtendedText ();
			u8g.setFontPosTop ();

			byte h = u8g.getFontAscent () - u8g.getFontDescent ();
			byte w = u8g.getWidth ();
			for (byte i = 0; cur[i] && i < nLines; i++) {
				byte d = (w - u8g.getStrWidth (cur[firstItem + i] -> getName ())) / 2;
				u8g.setDefaultForegroundColor ();

				if (firstItem + i == curItem) {
					u8g.drawBox (0, 16 + i * h + 1, w, h);
					u8g.setDefaultBackgroundColor ();
				}

				u8g.drawStr (d, 16 + i * h, cur[firstItem + i] -> getName ());
			}
		}
	}

	void prev () {
		if (curItem-- < 1) {
			byte i;
			for (i = 0; cur[i]; i++)
				;
			curItem = i - 1;

			if (i > nLines) {
				firstItem = curItem - nLines + 1;
			} else {
				firstItem = 0;
			}
		} else if (curItem < firstItem)
			firstItem--;
	}

	void next () {
		if (!cur[++curItem]) {
			curItem = 0;
			firstItem = 0;
		} else if (curItem >= firstItem + nLines)
			firstItem++;
	}

	void activate () {
		cur[curItem] -> activate ();
	}
};



/******************************************************************************/


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
auto lbtop = SwitcherMenuItem (mlBack, topMenu);

/*******************************************************************************
 * Log Options Menu
 ******************************************************************************/

auto lb = SwitcherMenuItem (mlBack, subMenuLogOpts);

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
auto lf1 = MenuItemLogFreq (mlf1, 1);
const char mlf5[] PROGMEM = "5 seconds";
auto lf5 = MenuItemLogFreq (mlf5, 5);
const char mlf10[] PROGMEM = "10 seconds";
auto lf10 = MenuItemLogFreq (mlf10, 10);
const char mlf30[] PROGMEM = "30 seconds";
auto lf30 = MenuItemLogFreq (mlf30, 30);
const char mlf60[] PROGMEM = "1 minute";
auto lf60 = MenuItemLogFreq (mlf60, 60);
MenuItem *subMenuLogFreq[] = {&lf1, &lf5, &lf10, &lf30, &lf60, &lb, NULL};

const char mlOff[] PROGMEM = "Off";
auto ldOff = MenuItemLogDist (mlOff, 0);
const char ml10[] PROGMEM = "10 m";
auto ld10 = MenuItemLogDist (ml10, 10);
const char ml100[] PROGMEM = "100 m";
auto ld100 = MenuItemLogDist (ml100, 100);
const char ml250[] PROGMEM = "250 m";
auto ld250 = MenuItemLogDist (ml250, 250);
const char ml1000[] PROGMEM = "1 km";
auto ld1000 = MenuItemLogDist (ml1000, 1000);
MenuItem *subMenuLogDist[] = {&ldOff, &ld10, &ld100, &ld250, &ld1000, &lb, NULL};

auto lrOff = MenuItemLogRot (mlOff, LOGROT_OFF);
const char mlH[] PROGMEM = "Hourly";
auto lrH = MenuItemLogRot (mlH, LOGROT_HOURLY);
const char mlD[] PROGMEM = "Daily";
auto lrD = MenuItemLogRot (mlD, LOGROT_DAILY);
const char mlW[] PROGMEM = "Weekly";
auto lrW = MenuItemLogRot (mlW, LOGROT_WEEKLY);
const char mlM[] PROGMEM = "Monthly";
auto lrM = MenuItemLogRot (mlM, LOGROT_MONTHLY);
MenuItem *subMenuLogRot[] = {&lrOff, &lrH, &lrD, &lrW, &lrM, &lb, NULL};

const char mlFreq[] PROGMEM = "Frequency";
auto loFreq = SwitcherMenuItem (mlFreq, subMenuLogFreq);
const char mlDist[] PROGMEM = "Distance";
auto loDist = SwitcherMenuItem (mlDist, subMenuLogDist);
const char mlRot[] PROGMEM = "Rotation";
auto loRot = SwitcherMenuItem (mlRot, subMenuLogRot);
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
auto da = MenuItemTimeDst (mlAuto, DST_AUTO);
const char mlOn[] PROGMEM = "On";
auto don = MenuItemTimeDst (mlOn, DST_ON);
auto doff = MenuItemTimeDst (mlOff, DST_OFF);
MenuItem *subMenuDstMode[] = {&da, &don, &doff, &ltb, NULL};

const char mlOm12[] PROGMEM = "-12";
auto om12 = MenuItemTimeOffset (mlOm12, -12);
const char mlOm11[] PROGMEM = "-11";
auto om11 = MenuItemTimeOffset (mlOm11, -11);
const char mlOm10[] PROGMEM = "-10";
auto om10 = MenuItemTimeOffset (mlOm10, -10);
const char mlOm9[] PROGMEM = "-9";
auto om9 = MenuItemTimeOffset (mlOm9, -9);
const char mlOm8[] PROGMEM = "-8";
auto om8 = MenuItemTimeOffset (mlOm8, -8);
const char mlOm7[] PROGMEM = "-7";
auto om7 = MenuItemTimeOffset (mlOm7, -7);
const char mlOm6[] PROGMEM = "-6";
auto om6 = MenuItemTimeOffset (mlOm6, -6);
const char mlOm5[] PROGMEM = "-5";
auto om5 = MenuItemTimeOffset (mlOm5, -5);
const char mlOm4[] PROGMEM = "-4";
auto om4 = MenuItemTimeOffset (mlOm4, -4);
const char mlOm3[] PROGMEM = "-3";
auto om3 = MenuItemTimeOffset (mlOm3, -3);
const char mlOm2[] PROGMEM = "-2";
auto om2 = MenuItemTimeOffset (mlOm2, -2);
const char mlOm1[] PROGMEM = "-1";
auto om1 = MenuItemTimeOffset (mlOm1, -1);
const char mlO0[] PROGMEM = "0";
auto o0 = MenuItemTimeOffset (mlO0, 0);
const char mlOp1[] PROGMEM = "+1";
auto op1 = MenuItemTimeOffset (mlOp1, +1);
const char mlOp2[] PROGMEM = "+2";
auto op2 = MenuItemTimeOffset (mlOp2, +2);
const char mlOp3[] PROGMEM = "+3";
auto op3 = MenuItemTimeOffset (mlOp3, +3);
const char mlOp4[] PROGMEM = "+4";
auto op4 = MenuItemTimeOffset (mlOp4, +4);
const char mlOp5[] PROGMEM = "+5";
auto op5 = MenuItemTimeOffset (mlOp5, +5);
const char mlOp6[] PROGMEM = "+6";
auto op6 = MenuItemTimeOffset (mlOp6, +6);
const char mlOp7[] PROGMEM = "+7";
auto op7 = MenuItemTimeOffset (mlOp7, +7);
const char mlOp8[] PROGMEM = "+8";
auto op8 = MenuItemTimeOffset (mlOp8, +8);
const char mlOp9[] PROGMEM = "+9";
auto op9 = MenuItemTimeOffset (mlOp9, +9);
const char mlOp10[] PROGMEM = "+10";
auto op10 = MenuItemTimeOffset (mlOp10, +10);
const char mlOp11[] PROGMEM = "+11";
auto op11 = MenuItemTimeOffset (mlOp11, +11);
const char mlOp12[] PROGMEM = "+12";
auto op12 = MenuItemTimeOffset (mlOp12, +12);
MenuItem *subMenuUtcOffset[] = {
	&om12, &om11, &om10, &om9, &om8, &om7, &om6, &om5, &om4, &om3, &om2, &om1,
	&o0,
	&op1, &op2, &op3, &op4, &op5, &op6, &op7, &op8, &op9, &op10, &op11, &op12,
	&ltb,
	NULL
};

const char mlUtcOffset[] PROGMEM = "UTC Offset";
auto toOff = SwitcherMenuItem (mlUtcOffset, subMenuUtcOffset);
const char mlDst[] PROGMEM = "Daylight Saving";
auto toDst = SwitcherMenuItem (mlDst, subMenuDstMode);
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

auto ss = StartStopMenuItem ();
const char mlLogOpts[] PROGMEM = "Log Options";
auto lo = SwitcherMenuItem (mlLogOpts, subMenuLogOpts);
const char mlTimeOpts[] PROGMEM = "Time Options";
auto to = SwitcherMenuItem (mlTimeOpts, subMenuTimeOpts);
auto ex = ExitMenuItem ();
MenuItem *topMenu[] = {&ss, &lo, &to, &ex, NULL};
