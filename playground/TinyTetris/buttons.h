/*******************************************************************************
 * This file is part of MyTrackr.                                              *
 *                                                                             *
 * Copyright (C) 2016-2018 by SukkoPera <software@sukkology.net>               *
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

#define KEY_BACK_PIN 7
#define KEY_PREV_PIN 6
#define KEY_NEXT_PIN 5
#define KEY_SELECT_PIN 4

class Buttons {
private:
  /* Combo debounce time: The combo will be considered valid only after it has
   * been stable for this amount of milliseconds
   */
  static const unsigned long HOLD_TIME = 25;

  /* Combo repeat delay: The combo will repeat after this amount of
   * milliseconds since the first valid pressure was detected.
   */
  static const unsigned long REPEAT_DELAY = 300;

  /* Combo repeat interval: When combo repeat starts, multiple pressions will
   * be reported every this amount of milliseconds.
   */
  static const unsigned long REPEAT_INTERVAL = 200;

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

    if (digitalRead (KEY_PREV_PIN) == LOW)
      b |= KEY_RIGHT;
    if (digitalRead (KEY_NEXT_PIN) == LOW)
      b |= KEY_LEFT;
    if (digitalRead (KEY_SELECT_PIN) == LOW)
      b |= KEY_DOWN;
    if (digitalRead (KEY_BACK_PIN) == LOW)
      b |= KEY_ROTATE;

    return b;
  }

public:
  enum Key {
    KEY_NONE    = 0,
    KEY_LEFT    = 1 << 0,
    KEY_RIGHT   = 1 << 1,
    KEY_ROTATE  = 1 << 2,
    KEY_DOWN    = 1 << 3,
  };

  void begin () {
    pinMode (KEY_BACK_PIN, INPUT_PULLUP);
    pinMode (KEY_PREV_PIN, INPUT_PULLUP);
    pinMode (KEY_NEXT_PIN, INPUT_PULLUP);
    pinMode (KEY_SELECT_PIN, INPUT_PULLUP);
  }

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


