#include "config.h"
#include "debug.h"

#if 0
	#include <SoftwareSerial.h>
	SoftwareSerial swSerial (4, 3);
	#define GPS_SERIAL swSerial
#else
	#define GPS_SERIAL Serial
#endif

#define GPS_BAUD 9600

#define GPS_INTERVAL 50

// http://arduiniana.org/libraries/tinygps/
#include <TinyGPS.h>
TinyGPS gps;

// Icons
#include "icons.h"

// SD and CSV helper
#include "CSVWriter.h"
CSVWriter writer;
boolean sdAvailable = false;

// Keys
#include "Buttons.h"
Buttons buttons;

// Display
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g (U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

#include "enums.h"

// Time
#include <TimeLib.h>

// "Alive led": get library at https://github.com/SukkoPera/AABlink
#include <AABlink.h>
AABlinkShort aliveLed;

#include <EEPROM.h>
const word EEPROM_DATA_OFFSET = 18;
const word EEPROM_GOOD_SIGNATURE = 0x1944;

#ifdef ENABLE_SOFT_POWEROFF
#include <LowPower.h>

ISR (KEY_INT_VECT) {
	// Nothing to do, actually ;)
	DPRINTLN (F("Woke up!"));
}

#include <avr/io.h>
#include <avr/wdt.h>

#define softReset() do {wdt_enable (WDTO_30MS); while(1) {}} while (0)
#endif


// *** Global variables ***

// True if logging is currently enabled
boolean logEnabled = false;

boolean keylockEnabled = false;

struct Options {
	// Interval between two consecutive log updates
	byte logFreq;

#ifdef ENABLE_DISTANCE_MENU
	int logDist;
#endif

#ifdef ENABLE_ROTATION_MENU
	LogRotation logRot;
#endif

#ifdef ENABLE_BACKLIGHT_MENU
	byte backlightTimeout;
#endif

	int8_t utcOffset;
	DaylightSavingMode dstMode;
};

Options options = {
	DEFAULT_LOG_INTERVAL,
#ifdef ENABLE_DISTANCE_MENU
	DEFAULT_LOG_DISTANCE,
#endif
#ifdef ENABLE_ROTATION_MENU
	DEFAULT_LOG_ROTATION,
#endif
#ifdef ENABLE_BACKLIGHT_MENU
	DEFAULT_BACKLIGHT_TIMEOUT,
#endif
	DEFAULT_TZ_OFFSET,
	DST_AUTO
};

GpsFix currentFix = {
	{0, 0, false},
	{0, false},
	{0, false},
	{0, false},
	{0, false},
	{0, 0, 0, 0, 0, 0, 0},
	0
};

GpsFix lastLoggedFix = {
	{0, 0, false},
	{0, false},
	{0, false},
	{0, false},
	{0, false},
	{0, 0, 0, 0, 0, 0, 0},
	0
};

// millis() of last time fix was logged to SD
unsigned long lastLogMillis = 0;

// Battery voltage (mV)
unsigned int batteryVoltage = 0;

// Battery percentage
byte batPerc = 0;


// Menu (This goes after globals, as it needs to access some of them)
#include "menu.h"

bool batteryLow () {
	return batPerc < 20;
}

bool fixValid () {
	time_t tt = makeTime (currentFix.time) + (options.utcOffset + dstOffset ()) * SECS_PER_HOUR;
	return currentFix.pos.valid && timeStatus () == timeSet && now () - tt < DATA_VALID_TIME;
}

void draw () {
	static const char naString[] PROGMEM = "N/A";

	u8g.setDefaultForegroundColor ();

	/***************************************************************************
	 * Header
	 **************************************************************************/

	// Sat icon (Blink if currentFix is not valid)
	if (fixValid () || ((millis () / 1000) % 2) == 0) {
		u8g.drawXBMP (0, 0, sat_width, sat_height, sat_bits);
	}

	// Number of sats in view
	u8g.setPrintPos (20, 3);
	u8g.print (currentFix.nsats);

	// Log status
	if (logEnabled) {
		u8g.setPrintPos (36, 3);
		u8g.print ('L');
	}

	// Keylock
	if (keylockEnabled) {
		u8g.drawXBMP (74, 0, padlock_width, padlock_height, padlock_bits);
	}

	// SD icon (blink if SD failed)
	if (sdAvailable || ((millis () / 1000) % 2) == 0) {
		u8g.drawXBMP (90, 0, microsd_width, microsd_height, microsd_bits);
	}

	// Battery icon, drawn by hand (Blink if almost dead)
	int top = (HEADER_HEIGHT - BAT_HEIGHT) / 2;
	if (!batteryLow () || ((millis () / 1000) % 2) == 0) {
		int batFillWidth = (BAT_WIDTH - 2 - 2) * batPerc / 100.0;
		u8g.drawFrame (128 - BAT_WIDTH, top, BAT_WIDTH, BAT_HEIGHT);  // Outside box
		u8g.drawBox (128 - BAT_WIDTH - BAT_TIP, top + (BAT_HEIGHT - BAT_HEIGHT / 2) / 2, BAT_TIP, BAT_HEIGHT / 2);  // Tip
		u8g.drawBox (128 - batFillWidth - 2, top + 2, batFillWidth, BAT_HEIGHT - 4);  // Filling
	}


	/***************************************************************************
	 * Main screen
	 **************************************************************************/
	if (!menuHandler.isShown ()) {
		switch (((millis () / 1000L) / 6) % 3) {
			case 0:
				// Position icon
				u8g.drawXBMP (20, 33, position_width, position_height, position_bits);

				// Coordinates
				if (currentFix.pos.valid) {
					u8g.setPrintPos (52, 27);
					u8g.print (currentFix.pos.lat, LATLON_PREC);
					u8g.setPrintPos (52, 37);
					u8g.print (currentFix.pos.lon, LATLON_PREC);
				} else {
					u8g.setPrintPos (52, 27);
					u8g.print (PSTR_TO_F (naString));
					u8g.setPrintPos (52, 37);
					u8g.print (PSTR_TO_F (naString));
				}

				u8g.setPrintPos (52, 47);
				if (currentFix.alt.valid) {
					u8g.print (currentFix.alt.value);
					u8g.print (F(" m"));
				} else {
					u8g.print (PSTR_TO_F (naString));
				}

				break;
			case 1: {
				// Compass icon
				u8g.drawXBMP (20, 33, compass_width, compass_height, compass_bits);

				u8g.setPrintPos (52, 32);
				if (currentFix.course.valid) {
					u8g.print (currentFix.course.value);
					u8g.print ((char) 176);   // Degrees symbol, font-dependent
				} else {
					u8g.print (PSTR_TO_F (naString));
				}

				u8g.setPrintPos (52, 42);
				if (currentFix.speed.valid) {
					u8g.print (currentFix.speed.value);
					u8g.print (F(" Km/h"));
				} else {
					u8g.print (PSTR_TO_F (naString));
				}
				break;
			} case 2:
				// Clock icon
				u8g.drawXBMP (20, 33, clock_width, clock_height, clock_bits);

				if (timeStatus () != timeNotSet) {
					u8g.setPrintPos (52, 32);
					u8g.print (day ());
					u8g.print ('/');
					u8g.print (month ());
					u8g.print ('/');
					u8g.print (year ());

					u8g.setPrintPos (52, 42);
					if (hour () < 10)
						u8g.print (0);
					u8g.print (hour ());
					u8g.print (':');
					if (minute () < 10)
						u8g.print (0);
					u8g.print (minute ());
					u8g.print (':');
					if (second () < 10)
						u8g.print (0);
					u8g.print (second ());
				} else {
					u8g.setPrintPos (52, 32);
					u8g.print (PSTR_TO_F (naString));
					u8g.setPrintPos (52, 42);
					u8g.print (PSTR_TO_F (naString));
				}
				break;
		}
	}
}

void handleKeys (void) {
#ifdef ENABLE_BACKLIGHT_MENU
	// True if screen is off
	static boolean sleeping = false;

	// millis() of last keypress
	static unsigned long lastKeyPress = 0;
#endif

	// millis() when SELECT + NEXT press started
	static unsigned long lockComboStart = 0;

	byte keys = buttons.read ();
	if (keys != Buttons::KEY_NONE) {
#ifdef ENABLE_BACKLIGHT_MENU
		lastKeyPress = millis ();

		if (sleeping) {
			// Turn screen back on
			u8g.sleepOff ();
			sleeping = false;
		} else {
#endif
			// Holding SELECT + NEXT will enable keylock
			if ((keys & (Buttons::KEY_SELECT | Buttons::KEY_NEXT)) == (Buttons::KEY_SELECT | Buttons::KEY_NEXT)) {
				if (lockComboStart == 0) {
					// Just started holding
					lockComboStart = millis ();
				} else if (millis () - lockComboStart > KEY_LOCK_DELAY) {
					keylockEnabled = !keylockEnabled;
					lockComboStart = 0;
				}
			} else {
				lockComboStart = 0;

				if (!keylockEnabled) {
					// Handle keypress
					if (menuHandler.isShown ()) {
						// SELECT key has precedence
						if (keys & Buttons::KEY_SELECT)
							menuHandler.activate ();
						else if (keys & Buttons::KEY_NEXT)
							menuHandler.next ();
#ifdef KEY_PREV_PIN
						else if (keys & Buttons::KEY_PREV)
							menuHandler.prev ();
#endif
					} else {
						// Open menu
						menuHandler.show ();
					}
				}
			}
		}
#ifdef ENABLE_BACKLIGHT_MENU
	} else if (options.backlightTimeout > 0 && millis () - lastKeyPress >= options.backlightTimeout * 1000UL) {
		u8g.sleepOn ();
		sleeping = true;
	}
#endif

	// u8g Picture loop
#ifdef ENABLE_BACKLIGHT_MENU
	if (!sleeping) {
#endif
		u8g.firstPage ();
		do {
			draw ();
			menuHandler.draw ();
		} while (u8g.nextPage ());
#ifdef ENABLE_BACKLIGHT_MENU
	}
#endif
}


void measureBattery () {
	// millis() of last time battery was measured
	static unsigned long lastBatteryMillis = 0;

	if (lastBatteryMillis == 0 || millis () - lastBatteryMillis > BATTERY_INTERVAL * 1000UL) {
		unsigned long tot = 0;
		for (byte b = 0; b < BATTERY_ITERATIONS; b++) {
			tot += analogRead (BATTERY_PIN);
			delay (BATTERY_IT_DELAY);
		}

		float reading = tot / BATTERY_ITERATIONS;

		DPRINT (F("Battery raw reading: "));
		DPRINTLN (reading);

#ifdef REAL_1_1_REF
		batteryVoltage = REAL_1_1_REF / (float) BATTERY_STEPS * reading;
#else
		batteryVoltage = 1100 / (float) BATTERY_STEPS * reading;
#endif

#if defined (BATTERY_R1) and defined (BATTERY_R2)
		batteryVoltage *= (BATTERY_R1 + BATTERY_R2) / BATTERY_R2;
#endif

		DPRINT (F("Battery voltage: "));
		DPRINTLN (batteryVoltage);

		// Percentage
		batPerc = 100;
		for (byte i = 0; i < BATTERY_POINTS_NO; i++) {
			unsigned int t = (pgm_read_byte (&BATTERY_POINTS[i]) + BATTERY_POINT_OFFSET) * 10;
			if (batteryVoltage < t) {
				batPerc -= 100 / BATTERY_POINTS_NO;
			}
		}

		DPRINT (F("Battery percentage: "));
		DPRINTLN (batPerc);

		lastBatteryMillis = millis ();
	}
}

void updateLed () {
	if (batteryLow ()) {
		aliveLed.setNBlinks (3);
	} else if (!fixValid ()) {
		aliveLed.setNBlinks (2);
	} else {
		aliveLed.setNBlinks (1);
	}
}


#ifdef ENABLE_SOFT_POWEROFF
void powerOff () {
	DPRINTLN (F("Power OFF"));
	Serial.end ();

	// Turn off screen
	u8g.sleepOn ();

	// Make sure i2c bus is disabled
	TWCR = 0;

	if (logEnabled) {
		// Stop logging and SD
		logEnabled = false;
		writer.end ();
	}

	// Make sure SPI bus is disabled
	SPCR = 0;

#ifdef PERIPHERALS_POWER_PIN
	// Turn off peripherals
	digitalWrite (PERIPHERALS_POWER_PIN, HIGH);
#endif

	// Allow buffers to be flushed and key to be released
	delay (1000);

	// Enable pin-change interrupt on key pins
	PCICR |= (1 << KEY_PCICR_BIT);
	KEY_PCMSK_REG = KEY_PCMSK_BITS;

	// Enter power down state with ADC and BOD module disabled.
	LowPower.powerDown (SLEEP_FOREVER, ADC_OFF, BOD_OFF);

	DPRINTLN (F("Power ON"));
	PCICR &= ~(1 << KEY_PCICR_BIT);

	// OK, usually I don't like this, but it makes things much easir this time!
	softReset ();
}
#endif

void setup () {
#ifdef ENABLE_SOFT_POWEROFF
	wdt_disable ();
#endif

	DSTART (9600);

	// Load data from EEPROM
	word sig = 0x1234;
	EEPROM.get (EEPROM_DATA_OFFSET + sizeof (Options), sig);
	if (sig == EEPROM_GOOD_SIGNATURE) {
		// EEPROM data is good
		DPRINTLN (F("EEPROM data is good"));
		EEPROM.get (EEPROM_DATA_OFFSET, options);
	} else {
		DPRINTLN (F("EEPROM data is invalid"));
	}

#ifdef PERIPHERALS_POWER_PIN
	// Turn on peripherals
	pinMode (PERIPHERALS_POWER_PIN, OUTPUT);
	digitalWrite (PERIPHERALS_POWER_PIN, LOW);
	delay (100);
#endif

	// See if the card is present and can be initialized:
	if (!writer.begin (SD_CHIPSELECT)) {
		DPRINTLN (F("SD Card failed or not present"));
	} else {
		DPRINTLN (F("SD ok"));
		sdAvailable = true;
	}

	GPS_SERIAL.begin (GPS_BAUD);

	DPRINT (F("Using TinyGPS "));
	DPRINTLN (TinyGPS::library_version ());

#ifdef KEY_BACK_PIN
	pinMode (KEY_BACK_PIN, INPUT_PULLUP);
#endif
#ifdef KEY_PREV_PIN
	pinMode (KEY_PREV_PIN, INPUT_PULLUP);
#endif
	pinMode (KEY_NEXT_PIN, INPUT_PULLUP);
	pinMode (KEY_SELECT_PIN, INPUT_PULLUP);
	menuHandler.begin (topMenu, MENU_LINES);

#ifdef REAL_1_1_REF
	analogReference (INTERNAL);

	// The ADC needs some readings after references has been switched
	for (byte i = 0; i < 128; i++)
		analogRead (BATTERY_PIN);
#endif

	// We always use these U8G settings
	u8g.begin ();
	u8g.setFont (u8g_font_6x10);
	u8g.setFontPosTop ();
	u8g.setFontRefHeightExtendedText ();

	// Alive led
	aliveLed.begin (ALIVE_LED_PIN, ALIVE_LED_ON_TIME, ALIVE_LED_OFF_TIME);
	aliveLed.blink ();
}

void loop () {
	decodeGPS ();
	logPosition ();
	updateLed ();
	measureBattery ();
	handleKeys ();
	aliveLed.loop ();
}

byte dstOffset () {
	switch (options.dstMode) {
		case DST_ON:
			return 1;
		case DST_OFF:
			return 0;
		case DST_AUTO:
		default:
		{
			/* This function returns the DST offset for the current UTC time.
			 * This is valid for the EU, for other places see
			 * http://www.webexhibits.org/daylightsaving/i.html
			 *
			 * Results have been checked for 2012-2030 (but should be correct
			 * since 1996 to 2099) against the following references:
			 * - http://www.uniquevisitor.it/magazine/ora-legale-italia.php
			 * - http://www.calendario-365.it/ora-legale-orario-invernale.html
			 */
			byte d = day (), m = month (), h = hour ();
			unsigned int y = year ();

			// Day in March that DST starts on, at 1 am
			byte dstOn = (31 - (5 * y / 4 + 4) % 7);

			// Day in October that DST ends  on, at 2 am
			byte dstOff = (31 - (5 * y / 4 + 1) % 7);

			if ((m > 3 && m < 10) ||
				(m == 3 && (d > dstOn || (d == dstOn && h >= 1))) ||
				(m == 10 && (d < dstOff || (d == dstOff && h <= 1))))
				return 1;
			else
				return 0;
		}
	}
}

void decodeGPS () {
#ifdef GPS_INTERVAL
	unsigned long start = millis ();
	do {
		while (GPS_SERIAL.available ()) {
#endif
			char c = GPS_SERIAL.read ();
			gps.encode (c);
#ifdef GPS_INTERVAL
		}
	} while (millis () - start < GPS_INTERVAL);
#endif

	currentFix.nsats = gps.satellites () != TinyGPS::GPS_INVALID_SATELLITES ? gps.satellites () : 0;

	unsigned long age;
	gps.f_get_position (&currentFix.pos.lat, &currentFix.pos.lon, &age);
	currentFix.pos.valid = age != TinyGPS::GPS_INVALID_AGE && currentFix.nsats > 0;

	currentFix.alt.value = gps.f_altitude ();
	currentFix.alt.valid = currentFix.alt.value != TinyGPS::GPS_INVALID_F_ALTITUDE;

	currentFix.course.value = gps.f_course ();
	currentFix.course.valid = currentFix.course.value != TinyGPS::GPS_INVALID_F_ANGLE;

	currentFix.speed.value = gps.f_speed_kmph ();
	currentFix.speed.valid = currentFix.speed.value != TinyGPS::GPS_INVALID_F_SPEED;

	currentFix.hdop.value = gps.hdop () / 100.0;
	currentFix.hdop.valid = gps.hdop () != TinyGPS::GPS_INVALID_HDOP;


	// Set the time to the latest GPS reading (which is always UTC)
	int year;
	gps.crack_datetime (&year, &currentFix.time.Month, &currentFix.time.Day, &currentFix.time.Hour, &currentFix.time.Minute, &currentFix.time.Second, NULL, &age);
	currentFix.time.Year = year - 1970;

	if (age < 600) {
		time_t tt = makeTime (currentFix.time);
		setTime (tt);

		// Convert to local time
		adjustTime ((options.utcOffset + dstOffset ()) * SECS_PER_HOUR);
	}
}


#define GPS_LOG_COLS 9

// GPS Logfile Columns
// Column names taken from http://www.gpsbabel.org/htmldoc-development/fmt_unicsv.html
const char c1[] PROGMEM = "utc_d";
const char c2[] PROGMEM = "utc_t";
const char c3[] PROGMEM = "lat";
const char c4[] PROGMEM = "lon";
const char c5[] PROGMEM = "ele";
const char c6[] PROGMEM = "speed";
const char c7[] PROGMEM = "head";
const char c8[] PROGMEM = "hdop";
const char c9[] PROGMEM = "sat";

const char* const cols[GPS_LOG_COLS] PROGMEM = {
	c1, c2, c3, c4, c5, c6, c7, c8, c9
};

#define MAX_FILENAME (8 + 1)

void itoa_2digits (int val, char *s) {
	*s = (val / 10) + '0';

	*(s + 1) = val % 10 + '0';

	*(s + 2) = '\0';
}

const char* makeFileName (char fn[MAX_FILENAME]) {
#ifdef ENABLE_ROTATION_MENU
	switch (options.logRot) {
		default:
		case LOGROT_OFF:
			// Use a generic "GPS.CSV"
			fn[0] = 'G';
			fn[1] = 'P';
			fn[2] = 'S';
			fn[3] = '\0';
			break;
		case LOGROT_HOURLY:
			// YYMMDD'H'x.csv (x: 0=A, 1=B... 23=W), e.g.: 170508HA.csv
			itoa_2digits (currentFix.time.Year - 30, fn);
			itoa_2digits (currentFix.time.Month, fn + 2);
			itoa_2digits (currentFix.time.Day, fn + 4);
			fn[6] = 'H';
			fn[7] = currentFix.time.Hour + 'A';
			fn[8] = '\0';
			break;
		case LOGROT_DAILY:
			// YYYYMMDD.CSV, e.g.: 20170508.CSV
			itoa (tmYearToCalendar (currentFix.time.Year), fn, 10);
			itoa_2digits (currentFix.time.Month, fn + 4);
			itoa_2digits (currentFix.time.Day, fn + 6);
			break;
		case LOGROT_WEEKLY: {
			// YYYY'W'WW.csv, e.g.: 2017W14.csv
			itoa (tmYearToCalendar (currentFix.time.Year), fn, 10);
			fn[4] = 'W';

			// https://en.wikipedia.org/wiki/ISO_week_date#Calculation
			byte daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
			int d = 10 + currentFix.time.Day;
			for (byte i = 1; i < currentFix.time.Month; i++)
				d += daysPerMonth[i - 1];
			d -= (currentFix.time.Wday == 1 ? 7 : currentFix.time.Wday - 1);
			itoa_2digits (d / 7, fn + 5);
			break;
		} case LOGROT_MONTHLY:
			// YYYY'M'MM.csv, e.g.: 2017M05.csv
			itoa (tmYearToCalendar (currentFix.time.Year), fn, 10);
			fn[4] = 'M';
			itoa_2digits (currentFix.time.Month, fn + 5);
			break;
	}

	// Append extension
	strncat_P (fn, PSTR (".CSV"), MAX_FILENAME);
#else
	// Use a generic "GPS.CSV"
	strncpy_P (fn, PSTR ("GPS.CSV"), MAX_FILENAME);
#endif

	return fn;
}

void logPosition () {
	if (logEnabled) {
		time_t tcur = makeTime (currentFix.time);
		time_t tlast = makeTime (lastLoggedFix.time);
		if (lastLogMillis != 0 && millis () - lastLogMillis < options.logFreq * 1000UL) {
			//~ DPRINTLN (F("Skipping log because too early"));
		} else if (tcur == tlast) {
			DPRINTLN (F("Skipping log, because fix unchanged"));
		} else if (!currentFix.pos.valid) {
			DPRINTLN (F("Skipping log because no fix detected"));
#ifdef ENABLE_DISTANCE_MENU
		} else if (options.logDist > 0 && lastLoggedFix.pos.valid &&
				TinyGPS::distance_between (lastLoggedFix.pos.lat, lastLoggedFix.pos.lon, currentFix.pos.lat, currentFix.pos.lon) < options.logDist) {
			DPRINTLN (F("Skipping log because too close to last fix"));
#endif
		} else {
			// Gotta log!
			DPRINTLN (F("Logging GPS fix"));

			char fn[MAX_FILENAME];
			makeFileName (fn);
			DPRINT (F("Opening logging file: "));
			DPRINTLN (fn);
			if (!writer.openFile (fn, GPS_LOG_COLS, cols)) {
				DPRINTLN (F("Cannot open logfile"));
				sdAvailable = false;
				logEnabled = false;
			} else {
				// Date
				writer.print (currentFix.time.Year + 1970);
				writer.print ('/');
				if (currentFix.time.Month < 10)
					writer.print (0);
				writer.print (currentFix.time.Month);
				writer.print ('/');
				if (currentFix.time.Day < 10)
					writer.print (0);
				writer.print (currentFix.time.Day);

				// Time
				writer.newField ();
				if (currentFix.time.Hour < 10)
					writer.print (0);
				writer.print (currentFix.time.Hour);
				writer.print (':');
				if (currentFix.time.Minute < 10)
					writer.print (0);
				writer.print (currentFix.time.Minute);
				writer.print (':');
				if (currentFix.time.Second < 10)
					writer.print (0);
				writer.print (currentFix.time.Second);
				//~ writer.print ('.');
				//~ if (hundredths < 100)
					//~ writer.print (0);
				//~ if (hundredths < 10)
					//~ writer.print (0);
				//~ writer.print (hundredths);

				// Latitude
				writer.newField ();
				writer.print (currentFix.pos.lat, LATLON_PREC);

				// Longitude
				writer.newField ();
				writer.print (currentFix.pos.lon, LATLON_PREC);

				// Altitude
				writer.newField ();
				if (currentFix.alt.valid)
					writer.print (currentFix.alt.value);

				// Speed (km/h, use explicit label for gpsbabel understanding)
				writer.newField ();
				if (currentFix.speed.valid) {
					writer.print (currentFix.speed.value);
					writer.print (F("kmh"));
				}

				// Course/Track/Heading
				writer.newField ();
				if (currentFix.course.valid)
					writer.print (currentFix.course.value);

				// Horizontal Dilution of Precision (HDOP)
				writer.newField ();
				if (currentFix.hdop.valid)
					writer.print (currentFix.hdop.value);

				// Number of sats
				writer.newField ();
				writer.print (currentFix.nsats);

				writer.endRecord ();

				if (!writer.closeFile ()) {
					DPRINTLN (F("CSV file close failed"));
					sdAvailable = false;
					logEnabled = false;
				} else {
					lastLoggedFix = currentFix;
				}

				lastLogMillis = millis ();
			}
		}
	}
}
