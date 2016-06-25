#include "config.h"
#include "debug.h"

//~ #include <PString.h>

//~ #include <SoftwareSerial.h>
//~ SoftwareSerial ss (7, 8);
//#define GPS_SERIAL ss
#define GPS_SERIAL Serial
#define GPS_INTERVAL 50
//#define GPS_BAUD 4800
#define GPS_BAUD 9600


// http://arduiniana.org/libraries/tinygps/
#include <TinyGPS.h>
TinyGPS gps;

// Icons
#include "icons.h"

// SD and CSV helper
#include "CSVWriter.h"
CSVWriter writer;
boolean sdAvailable = false;


// Display
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g (U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

#include "enums.h"

// Time
#include <TimeLib.h>


boolean logEnabled = false;

// Interval between two consecutive log updates
byte logFreq = DEFAULT_LOG_INTERVAL;
unsigned int logDist = DEFAULT_LOG_DISTANCE;
LogRotation logRot = DEFAULT_LOG_ROTATION;		// FIXME: TBD

signed char utcOffset = DEFAULT_TZ_OFFSET;
DaylightSavingMode dstMode = DST_AUTO;

#include "menu.h"

#define HEADER_HEIGHT 16

struct Position {
	float lat;
	float lon;
	boolean valid;
};

struct ValidFloat {
	float value;
	boolean valid;
};

struct GpsFix {
	Position pos;
	ValidFloat alt;			// Meters
	ValidFloat course;		// Degrees from True North
	ValidFloat speed;		// km/h
	ValidFloat hdop;
	TimeElements time;
	unsigned int nsats;
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

unsigned long lastLogMillis = 0;

void draw () {
	static const char naString[] PROGMEM = "N/A";

	u8g.setDefaultForegroundColor ();

	/***************************************************************************
	 * Header
	 **************************************************************************/

	// Sat icon (Blink if currentFix is not valid)
	time_t tt = makeTime (currentFix.time) + (utcOffset + dstOffset ()) * SECS_PER_HOUR;
	if ((currentFix.pos.valid && timeStatus () == timeSet && now () - tt < 10) || ((millis () / 1000) % 2) == 0) {
		u8g.drawXBMP (0, 0, sat_width, sat_height, sat_bits);
	}

	// Number of sats in view
	u8g.setPrintPos (20, 3);
	u8g.print (currentFix.nsats);

	// SD icon (blink if SD failed)
	if (sdAvailable || ((millis () / 1000) % 2) == 0) {
		u8g.drawXBMP (90, 0, microsd_width, microsd_height, microsd_bits);
	}

	// Log status
	if (logEnabled) {
		u8g.setPrintPos (80, 3);
		u8g.print ('L');
	}

	// Battery icon, drawn by hand
	int top = (HEADER_HEIGHT - BAT_HEIGHT) / 2;
	u8g.drawFrame (128 - BAT_WIDTH, top, BAT_WIDTH, BAT_HEIGHT);  // Outside box
	u8g.drawBox (128 - BAT_WIDTH - BAT_TIP, top + (BAT_HEIGHT - BAT_HEIGHT / 2) / 2, BAT_TIP, BAT_HEIGHT / 2);  // Tip
	u8g.drawBox (128 - BAT_WIDTH + 2, top + 2, BAT_WIDTH - 4, BAT_HEIGHT - 4);  // Filling (FIXME: Make proportional to charge)


	/***************************************************************************
	 * Main screen
	 **************************************************************************/
	if (!menuHandler.isShown ()) {
		switch (((millis () / 1000L) / 7) % 3) {
			case 0:
				// Position icon
				u8g.drawXBMP (20, 33, position_width, position_height, position_bits);

				// Coordinates
				if (currentFix.pos.valid) {
					u8g.setPrintPos (52, 32);
					u8g.print (currentFix.pos.lat, 6);
					u8g.setPrintPos (52, 42);
					u8g.print (currentFix.pos.lon, 6);
				} else {
					u8g.setPrintPos (52, 32);
					u8g.print (PSTR_TO_F (naString));
					u8g.setPrintPos (52, 42);
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

Key readKeys (void) {
	Key uiKeyCode = KEY_NONE;

	if (digitalRead (KEY_NEXT_PIN) == LOW)
		uiKeyCode = KEY_NEXT;
	else if (digitalRead (KEY_SELECT_PIN) == LOW)
		uiKeyCode = KEY_SELECT;
#ifdef KEY_PREV_PIN
	else if (digitalRead (KEY_PREV_PIN) == LOW)
		uiKeyCode = KEY_PREV;
#endif
	//~ else if ( digitalRead(uiKeyBack) == LOW )
		//~ uiKeyCode = KEY_BACK;

	return uiKeyCode;
}



void updateMenu (void) {
	static byte last_key_code = KEY_NONE;

	// Avoid repeated presses
	Key uiKeyCode = readKeys ();
	if (uiKeyCode != KEY_NONE && last_key_code == uiKeyCode) {
		return;
	}
	last_key_code = uiKeyCode;

	if (menuHandler.isShown ()) {
		switch (uiKeyCode) {
			case KEY_NEXT:
				menuHandler.next ();
				break;
#ifdef KEY_PREV_PIN
			case KEY_PREV:
				menuHandler.prev ();
				break;
#endif
			case KEY_SELECT:
				menuHandler.activate ();
				break;
			default:
				break;
		}
	} else if (uiKeyCode != KEY_NONE) {
		menuHandler.show ();
	}
}


void setup () {
	DSTART (9600);

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

	pinMode (KEY_NEXT_PIN, INPUT_PULLUP);
	pinMode (KEY_SELECT_PIN, INPUT_PULLUP);
	menuHandler.begin (topMenu, MENU_LINES);

	// We always use these U8G settings
	u8g.setFont (u8g_font_6x10);
	u8g.setFontPosTop ();
	u8g.setFontRefHeightExtendedText ();
}

void loop () {
	decodeGPS ();
	logPosition ();

	updateMenu ();

	// u8g Picture loop
	u8g.firstPage ();
	do {
		draw ();
		menuHandler.draw ();
	} while (u8g.nextPage ());
}

byte dstOffset () {
	switch (dstMode) {
		case DST_ON:
			return 1;
		case DST_OFF:
			return 0;
		case DST_AUTO:
		default:
		{
			/* This is valid for the EU, for other places see
			 * http://www.webexhibits.org/daylightsaving/i.html
			 *
			 * Someone please check the implementation anyway :D.
			 */
			byte d = day (), m = month (), y = year (), h = hour ();
			if ((m == 3 && d >= (31 - (5 * y / 4 + 4) % 7) && h >= 1) ||
			  (m > 3 && m < 10) ||
			  (m == 10 && d <= (31 - (5 * y / 4 + 1) % 7) && h < 1))
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

	unsigned long age;
	gps.f_get_position (&currentFix.pos.lat, &currentFix.pos.lon, &age);
	currentFix.pos.valid = age != TinyGPS::GPS_INVALID_AGE;

	currentFix.alt.value = gps.f_altitude ();
	currentFix.alt.valid = currentFix.alt.value != TinyGPS::GPS_INVALID_F_ALTITUDE;

	currentFix.course.value = gps.f_course ();
	currentFix.course.valid = currentFix.course.value != TinyGPS::GPS_INVALID_F_ANGLE;

	currentFix.speed.value = gps.f_speed_kmph ();
	currentFix.speed.valid = currentFix.speed.value != TinyGPS::GPS_INVALID_F_SPEED;

	currentFix.hdop.value = gps.hdop () / 100.0;
	currentFix.hdop.valid = gps.hdop () != TinyGPS::GPS_INVALID_HDOP;

	currentFix.nsats = gps.satellites () != TinyGPS::GPS_INVALID_SATELLITES ? gps.satellites () : 0;

	// Set the time to the latest GPS reading (which is always UTC)
	int year;
	gps.crack_datetime (&year, &currentFix.time.Month, &currentFix.time.Day, &currentFix.time.Hour, &currentFix.time.Minute, &currentFix.time.Second, NULL, &age);
	currentFix.time.Year = year - 1970;

	if (age < 600) {
		time_t tt = makeTime (currentFix.time);
        setTime (tt);

        // Convert to local time
        adjustTime ((utcOffset + dstOffset ()) * SECS_PER_HOUR);
	}
}


#define GPS_LOG_COLS 9

// GPS Logfile Columns
// Column names taken from http://www.gpsbabel.org/htmldoc-development/fmt_unicsv.html
const char c1[] PROGMEM = "date";
const char c2[] PROGMEM = "time";
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

void logPosition () {
	if (logEnabled) {
		time_t tcur = makeTime (currentFix.time);
		time_t tlast = makeTime (lastLoggedFix.time);
		if (lastLogMillis != 0 && millis () - lastLogMillis < logFreq * 1000UL) {
			//~ DPRINTLN (F("Skipping log because too early"));
		} else if (tcur == tlast) {
			DPRINTLN (F("Skipping log, because fix unchanged"));
		} else if (!currentFix.pos.valid) {
			DPRINTLN (F("Skipping log because no fix detected"));
		} else if (logDist > 0 && lastLoggedFix.pos.valid &&
				TinyGPS::distance_between (lastLoggedFix.pos.lat, lastLoggedFix.pos.lon, currentFix.pos.lat, currentFix.pos.lon) <= logDist) {
			DPRINTLN (F("Skipping log because too close to last fix"));
		} else {
			// Gotta log!
			DPRINTLN (F("Logging GPS fix"));

			if (!writer.openFile ("GPS.CSV", GPS_LOG_COLS, cols)) {
				DPRINTLN (F("GPS CSV init failed"));
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

				// Speed (knots)
				writer.newField ();
				if (currentFix.speed.valid)
					writer.print (currentFix.speed.value);

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
