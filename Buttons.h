 /*******************************************************************************
 * This file is part of LeoBraille.                                            *
 *                                                                             *
 * Copyright (C) 2016 by SukkoPera <software@sukkology.net>                    *
 *                                                                             *
 * LeoBraille is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation, either version 3 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * LeoBraille is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with LeoBraille. If not, see <http://www.gnu.org/licenses/>.          *
 *******************************************************************************
 *
 * Watashitachi no ookina yume
 */

#ifndef BUTTONS_H_INCLUDED
#define BUTTONS_H_INCLUDED

#include "config.h"
#include "common.h"
#include "debug.h"

class Buttons {
private:
	/* Combo debounce time: The combo will be considered valid only after it has
	 * been stable for this amount of milliseconds
	 */
	static const unsigned long HOLD_TIME = 40;

	/* Combo repeat delay: The combo will repeat after this amount of
	 * milliseconds since the first valid pressure was detected.
	 */
	static const unsigned long REPEAT_DELAY = 800;

	/* Combo repeat interval: When combo repeat starts, multiple pressions will
	 * be reported every this amount of milliseconds.
	 */
	static const unsigned long REPEAT_INTERVAL = 250;

	// Reads and debounces
	byte readDebounced () {
		static byte oldKeys = 0;
		static unsigned long pressedOn = 0;

		byte ret = 0;

		byte keys = readRaw ();
		//~ DPRINT (F("Keys = "));
		//~ DPRINTLN (allKeys, BIN);

		if (keys == oldKeys) {
			if (millis () - pressedOn >= HOLD_TIME) {
				// Same combo hold long enough
				ret = keys;
			} else {
				// Combo hold not yet long enough
			}
		} else {
			// Keys bouncing
			oldKeys = keys;
			pressedOn = millis ();
		}

		return ret;
	}

	// Returns an 8-bit mask, where each button maps to bit
	byte readRaw () {
	  	byte b = 0;

#ifdef KEY_PREV_PIN
		if (digitalRead (KEY_PREV_PIN) == LOW)
			b |= KEY_PREV;
#endif
		if (digitalRead (KEY_NEXT_PIN) == LOW)
			b |= KEY_NEXT;
		if (digitalRead (KEY_SELECT_PIN) == LOW)
			b |= KEY_SELECT;
#ifdef KEY_BACK_PIN
		if (digitalRead (KEY_SELECT) == LOW)
			b |= KEY_BACK;
#endif

	  return b;
	}

public:
	enum _PACKED_ Key {
		KEY_NONE    = 0,
		KEY_PREV    = 1 << 0,
		KEY_NEXT    = 1 << 1,
		KEY_SELECT  = 1 << 2,
		KEY_BACK    = 1 << 3,
	};

	// Reads and repeats
	byte read () {
		static byte oldKeys = 0;
		static unsigned long nextRepeat = 0;

		byte ret = KEY_NONE;

		byte keys = readDebounced ();
		if (keys != oldKeys) {
			// First press of new combo, return it and wait for repeat delay
			oldKeys = keys;
			ret = keys;
			nextRepeat = millis () + REPEAT_DELAY;
		} else if (keys != 0 && millis () >= nextRepeat) {
			// Combo kept pressed, return it and wait for repeat interval
			ret = keys;
			nextRepeat = millis () + REPEAT_INTERVAL;
		}

		return ret;
	}
};

#endif
