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

	virtual bool isSelected (void) {
		return false;
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
	Menu cur;			// Current item
	byte nLines;		// No. of lines on screen
	byte curItem;		// Index of currently selected item
	byte firstItem;		// Index of first visible item
	boolean shown;		// True if menu is visible

	MenuItem *getMenuItem (int n) {
		return reinterpret_cast<MenuItem *> (pgm_read_ptr (&(cur[n])));
	}

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
		curItem = 0;
		firstItem = 0;
	}

	void hide () {
		shown = false;
	}

	boolean isShown () const {
		return shown;
	}

	void draw () {
		if (shown) {
			const byte h = u8g.getFontAscent () - u8g.getFontDescent ();		// Line height
			const byte scrW = u8g.getWidth ();									// Screen width
			//~ const byte scrH = u8g.getHeight ();									// Screen height
			MenuItem *item = NULL;
			for (byte i = 0; i < nLines && (item = getMenuItem (firstItem + i)); i++) {
				byte d = (scrW - u8g.getStrWidth (item -> getName ())) / 2;
				u8g.setDefaultForegroundColor ();

				if (firstItem + i == curItem) {
					u8g.drawBox (0, HEADER_HEIGHT + i * h + 1, scrW, h);
					u8g.setDefaultBackgroundColor ();
				}

				u8g.setPrintPos (d, HEADER_HEIGHT + i * h);
				u8g.print (item -> getName ());

				if (item -> isSelected ()) {
					u8g.print (F(" *"));
				}
			}

			// Show hints that there are items off screen
			if (firstItem > 0) {
				if (curItem == firstItem)		// Up hint is shown on first line
					u8g.setDefaultBackgroundColor ();
				else
					u8g.setDefaultForegroundColor ();
				u8g.setPrintPos (scrW - 6, HEADER_HEIGHT);
				u8g.print ((char) 0x7B);
			}

			if (item && getMenuItem (firstItem + nLines)) {
				if (curItem == firstItem + nLines - 1)		// Down hint is shown on last line
					u8g.setDefaultBackgroundColor ();
				else
					u8g.setDefaultForegroundColor ();
				u8g.setPrintPos (scrW - 6, HEADER_HEIGHT + (nLines - 1) * h);
				u8g.print ((char) 0x7D);
			}
		}
	}

	void prev () {
		if (curItem-- < 1) {
			byte i;

			// Scrolling backwards past first item, wrap down to the last one
			for (i = 0; getMenuItem (i); i++)
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
		MenuItem *item = getMenuItem (++curItem);
		if (!item) {
			curItem = 0;
			firstItem = 0;
		} else if (curItem >= firstItem + nLines) {
			firstItem++;
		}
	}

	void activate () {
		MenuItem *item = getMenuItem (curItem);
		item -> activate ();
	}

	void back () {
		byte i;
		for (i = 0; pgm_read_ptr (&(cur[i])); i++)
				;
		MenuItem *item = getMenuItem (i - 1);
		item -> activate ();
	}
};

#endif
