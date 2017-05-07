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
			byte h = u8g2.getAscent () - u8g2.getDescent ();
			byte w = u8g2.getDisplayWidth ();
			MenuItem *item = NULL;
			for (byte i = 0; i < nLines && (item = reinterpret_cast<MenuItem *> (pgm_read_word (&(cur[firstItem + i])))); i++) {
				//~ byte d = (w - u8g2.getStrWidth (item -> getName ())) / 2;
				byte d = 0;		// FIXME
				u8g2.setDrawColor (1);

				if (firstItem + i == curItem) {
					u8g2.drawBox (0, 16 + i * h + 1, w, h);
					u8g2.setDrawColor (0);
				}

				u8g2.setCursor (d, 16 + i * h);
				u8g2.print (item -> getName ());
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
