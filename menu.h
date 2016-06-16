class MenuItem {
public:
	virtual const char *getName (void) const = 0;

	virtual void activate (void) {
	}
};

typedef MenuItem** Menu;

class StaticMenuItem: public MenuItem {
public:
	const char* name;

	StaticMenuItem (const char* _name): name (_name) {
	}

	const char* getName (void) const override {
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
			size_t i;
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
	SwitcherMenuItem (const char* _name, const Menu& _m): StaticMenuItem (_name), menu (_m) {
	}

	void activate (void) override {
		menuHandler.setCur (menu);
	}
};

/******************************************************************************/



// Forward declarations
extern MenuItem *topMenu[], *subMenuLogOpts[], *subMenuTimeOpts[];

// Item to get back to main menu
auto lbtop = SwitcherMenuItem ("Back", topMenu);


/*******************************************************************************
 * Log Options Menu
 ******************************************************************************/

auto lb = SwitcherMenuItem ("Back", subMenuLogOpts);

class MenuItemLogFreq: public StaticMenuItem {
private:
	int freq;

public:
	MenuItemLogFreq (const char* name, int f): StaticMenuItem (name), freq (f) {
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
	MenuItemLogDist (const char* name, int d): StaticMenuItem (name), dist (d) {
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
	MenuItemLogRot (const char* name, LogRotation r): StaticMenuItem (name), rot (r) {
	}

	void activate () override {
		logRot = rot;

		DPRINT (F("Log Rotation is now "));
		DPRINT (logRot);

		lb.activate ();
	}
};

auto lf0 = MenuItemLogFreq ("1 second", 1);
auto lf1 = MenuItemLogFreq ("5 seconds", 5);
auto lf2 = MenuItemLogFreq ("10 seconds", 10);
auto lf3 = MenuItemLogFreq ("30 seconds", 30);
auto lf4 = MenuItemLogFreq ("60 seconds", 60);
MenuItem *subMenuLogFreq[] = {&lf0, &lf1, &lf2, &lf3, &lf4, &lb, NULL};

auto ldOff = MenuItemLogDist ("Off", 0);
auto ld10 = MenuItemLogDist ("10 m", 10);
auto ld100 = MenuItemLogDist ("100 m", 100);
auto ld250 = MenuItemLogDist ("250 m", 250);
auto ld1000 = MenuItemLogDist ("1 km", 1000);
MenuItem *subMenuLogDist[] = {&ldOff, &ld10, &ld100, &ld250, &ld1000, &lb, NULL};

auto lrOff = MenuItemLogRot ("Off", LOGROT_OFF);
auto lrH = MenuItemLogRot ("Hourly", LOGROT_HOURLY);
auto lrD = MenuItemLogRot ("Daily", LOGROT_DAILY);
auto lrW = MenuItemLogRot ("Weekly", LOGROT_WEEKLY);
auto lrM = MenuItemLogRot ("Monthly", LOGROT_MONTHLY);
MenuItem *subMenuLogRot[] = {&lrOff, &lrH, &lrD, &lrW, &lrM, &lb, NULL};

auto loFreq = SwitcherMenuItem ("Frequency", subMenuLogFreq);
auto loDist = SwitcherMenuItem ("Distance", subMenuLogDist);
auto loRot = SwitcherMenuItem ("Rotation", subMenuLogRot);
MenuItem *subMenuLogOpts[] = {&loFreq, &loDist, &loRot, &lbtop, NULL};


/*******************************************************************************
 * Time Options Menu
 ******************************************************************************/

auto ltb = SwitcherMenuItem ("Back", subMenuTimeOpts);

class MenuItemTimeOffset: public StaticMenuItem {
private:
	int offset;

public:
	MenuItemTimeOffset (const char* name, int o): StaticMenuItem (name), offset (o) {
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
	MenuItemTimeDst (const char* name, DaylightSavingMode m): StaticMenuItem (name), mode (m) {
	}

	void activate () override {
		dstMode = mode;

		DPRINT (F("DST mode is now "));
		DPRINT (dstMode);

		ltb.activate ();
	}
};

auto da = MenuItemTimeDst ("Auto", DST_AUTO);
auto don = MenuItemTimeDst ("On", DST_ON);
auto doff = MenuItemTimeDst ("Off", DST_OFF);
MenuItem *subMenuDstMode[] = {&da, &don, &doff, &ltb, NULL};

auto om12 = MenuItemTimeOffset ("-12", -12);
auto om11 = MenuItemTimeOffset ("-11", -11);
auto om10 = MenuItemTimeOffset ("-10", -10);
auto om9 = MenuItemTimeOffset ("-9", -9);
auto om8 = MenuItemTimeOffset ("-8", -8);
auto om7 = MenuItemTimeOffset ("-7", -7);
auto om6 = MenuItemTimeOffset ("-6", -6);
auto om5 = MenuItemTimeOffset ("-5", -5);
auto om4 = MenuItemTimeOffset ("-4", -4);
auto om3 = MenuItemTimeOffset ("-3", -3);
auto om2 = MenuItemTimeOffset ("-2", -2);
auto om1 = MenuItemTimeOffset ("-1", -1);
auto o0 = MenuItemTimeOffset ("0", 0);
auto op1 = MenuItemTimeOffset ("+1", +1);
auto op2 = MenuItemTimeOffset ("+2", +2);
auto op3 = MenuItemTimeOffset ("+3", +3);
auto op4 = MenuItemTimeOffset ("+4", +4);
auto op5 = MenuItemTimeOffset ("+5", +5);
auto op6 = MenuItemTimeOffset ("+6", +6);
auto op7 = MenuItemTimeOffset ("+7", +7);
auto op8 = MenuItemTimeOffset ("+8", +8);
auto op9 = MenuItemTimeOffset ("+9", +9);
auto op10 = MenuItemTimeOffset ("+10", +10);
auto op11 = MenuItemTimeOffset ("+11", +11);
auto op12 = MenuItemTimeOffset ("+12", +12);
MenuItem *subMenuUtcOffset[] = {
	&om12, &om11, &om10, &om9, &om8, &om7, &om6, &om5, &om4, &om3, &om2, &om1,
	&o0,
	&op1, &op2, &op3, &op4, &op5, &op6, &op7, &op8, &op9, &op10, &op11, &op12,
	&ltb,
	NULL
};

auto toOff = SwitcherMenuItem ("UTC Offset", subMenuUtcOffset);
auto toDst = SwitcherMenuItem ("Daylight Saving", subMenuDstMode);
MenuItem *subMenuTimeOpts[] = {&toOff, &toDst, &lbtop, NULL};


/*******************************************************************************
 * Main Menu
 ******************************************************************************/

class StartStopMenuItem: public MenuItem {
public:
	const char *getName (void) const override {
		if (running)
			return "Stop";
		else
			return "Start";
	}

	void activate (void) override {
		running = !running;
//    mvprintw (23, 0, "Logging is now %s", running ? "ON" : "OFF");
	}
};

class ExitMenuItem: public StaticMenuItem {
public:
	ExitMenuItem (): StaticMenuItem ("Exit") {
	}

	void activate (void) override {
		menuHandler.show = false;
	}
};

auto ss = StartStopMenuItem ();
auto lo = SwitcherMenuItem ("Log Options", subMenuLogOpts);
auto to = SwitcherMenuItem ("Time Options", subMenuTimeOpts);
auto ex = ExitMenuItem ();
MenuItem *topMenu[] = {&ss, &lo, &to, &ex, NULL};
