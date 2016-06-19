#ifndef MENUHANDLER_H_
#define MENUHANDLER_H_

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

/******************************************************************************/


class StaticMenuItem: public MenuItem {
public:
	FlashString name;

	StaticMenuItem (PGM_P _name): name (PSTR_TO_F (_name)) {
	}

	FlashString getName (void) const override {
		return name;
	}
};

/******************************************************************************/


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

#endif