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

typedef MenuItem* const * Menu;

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
	boolean shown;

public:

	void begin (const Menu& m, byte _nLines, boolean _shown = false) {
		nLines = _nLines;
		shown = _shown;
		setCur (m);
	}

	void setCur (const Menu& m) {
		cur = m;
		curItem = 0;
		firstItem = 0;
	}

	void show () {
		shown = true;
	}

	void hide () {
		shown = false;
	}

	boolean isShown () const {
		return shown;
	}

	void draw () {
		if (shown) {
			u8g.setFont (u8g_font_6x10);
			u8g.setFontRefHeightExtendedText ();
			u8g.setFontPosTop ();

			byte h = u8g.getFontAscent () - u8g.getFontDescent ();
			byte w = u8g.getWidth ();
			MenuItem *item = NULL;
			for (byte i = 0; i < nLines && (item = reinterpret_cast<MenuItem *> (pgm_read_word (&(cur[firstItem + i])))); i++) {
				byte d = (w - u8g.getStrWidth (item -> getName ())) / 2;
				u8g.setDefaultForegroundColor ();

				if (firstItem + i == curItem) {
					u8g.drawBox (0, 16 + i * h + 1, w, h);
					u8g.setDefaultBackgroundColor ();
				}

				u8g.drawStr (d, 16 + i * h, item -> getName ());
			}
		}
	}

	void prev () {
		if (curItem-- < 1) {
			byte i;
			for (i = 0; pgm_read_word (&(cur[i])); i++)
				;
			curItem = i - 1;

			if (i > nLines) {
				firstItem = curItem - nLines + 1;
			} else {
				firstItem = 0;
			}
		} else if (curItem < firstItem) {
			firstItem--;
		}
	}

	void next () {
		MenuItem *item = reinterpret_cast<MenuItem *> (pgm_read_word (&(cur[++curItem])));
		if (!item) {
			curItem = 0;
			firstItem = 0;
		} else if (curItem >= firstItem + nLines) {
			firstItem++;
		}
	}

	void activate () {
		MenuItem *item = reinterpret_cast<MenuItem *> (pgm_read_word (&(cur[curItem])));
		item -> activate ();
	}
};

#endif
