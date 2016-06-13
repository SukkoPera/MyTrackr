#include "debug.h"

//~ #include <PString.h>

//~ #include <SoftwareSerial.h>
//~ SoftwareSerial ss (7, 8);
//#define GPS_SERIAL ss
#define GPS_SERIAL Serial
#define GPS_INTERVAL 100
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

// Time
#include <TimeLib.h>

/* Offset hours from gps time (UTC)
 * Ideally, it should be possible to learn the time zone based on the GPS
 * position data.  However, that would require a complex library, probably
 * incorporating some sort of database using Eric Muller's time zone shape maps,
 * at http://efele.net/maps/tz/
 *
 * FIXME: Put into config menu
 */
const byte timeZoneOffset = 1;   // Central European Time
//const byte timeZoneOffset = -5;  // Eastern Standard Time (USA)
//const byte timeZoneOffset = -4;  // Eastern Daylight Time (USA)
//const byte timeZoneOffset = -8;  // Pacific Standard Time (USA)
//const byte timeZoneOffset = -7;  // Pacific Daylight Time (USA)


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
	TimeElements time;
	unsigned int nsats;
};

GpsFix fix;

time_t lastLoggedFix;
unsigned long lastLogMillis = 0;

void draw () {
	u8g.setFontPosTop ();

	// Sat icon (Blink if fix is not valid)
	time_t tt = makeTime (fix.time) + (timeZoneOffset + dstOffset ()) * SECS_PER_HOUR;
	if ((fix.pos.valid && timeStatus () == timeSet && now () - tt < 10) || ((millis () / 1000) % 2) == 0) {
		u8g.drawXBMP (0, 0, sat_width, sat_height, sat_bits);
	}

	// Number of sats in view
	u8g.setFont (u8g_font_6x10);
	u8g.setPrintPos (20, 3);
	u8g.print (fix.nsats);

	// Main
	switch (((millis () / 1000L) / 7) % 3) {
		case 0:
			// Position icon
			u8g.drawXBMP (20, 33, position_width, position_height, position_bits);

			// Coordinates
			if (fix.pos.valid) {
				u8g.setPrintPos (52, 32);
				u8g.print (fix.pos.lat, 6);
				u8g.setPrintPos (52, 42);
				u8g.print (fix.pos.lon, 6);
			} else {
				u8g.setPrintPos (52, 32);
				u8g.print (F("N/A"));
				u8g.setPrintPos (52, 42);
				u8g.print (F("N/A"));
			}
			break;
		case 1: {
			// Compass icon
			u8g.drawXBMP (20, 33, compass_width, compass_height, compass_bits);

			u8g.setPrintPos (52, 32);
			if (fix.course.valid) {
				u8g.print (fix.course.value);
				u8g.print ((char) 176);   // Degrees symbol, font-dependent
			} else {
				u8g.print (F("N/A"));
			}

			u8g.setPrintPos (52, 42);
			if (fix.speed.valid) {
				u8g.print (fix.speed.value);
				u8g.print (F(" km/h"));
			} else {
				u8g.print (F("N/A"));
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
				u8g.print (F("N/A"));
				u8g.setPrintPos (52, 42);
				u8g.print (F("N/A"));
			}
			break;
	}

	// SD icon (blink if SD failed)
#ifdef ENABLE_SD
	if (sdAvailable || ((millis () / 1000) % 2) == 0) {
		u8g.drawXBMP (90, 0, microsd_width, microsd_height, microsd_bits);
	}
#endif

	// Battery icon, drawn by hand
	int top = (HEADER_HEIGHT - BAT_HEIGHT) / 2;
	u8g.drawFrame (128 - BAT_WIDTH, top, BAT_WIDTH, BAT_HEIGHT);  // Outside box
	u8g.drawBox (128 - BAT_WIDTH - BAT_TIP, top + (BAT_HEIGHT - BAT_HEIGHT / 2) / 2, BAT_TIP, BAT_HEIGHT / 2);  // Tip
	u8g.drawBox (128 - BAT_WIDTH + 2, top + 2, BAT_WIDTH - 4, BAT_HEIGHT - 4);  // Filling (FIXME: Make proportional to charge)
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

	fix.pos.valid = false;
	fix.alt.valid = false;
	fix.course.valid = false;
	fix.speed.valid = false;
	lastLoggedFix = -1;
	lastLogMillis = 0;

	// Ready!
	pinMode (LED_BUILTIN, OUTPUT);
	for (int i = 0; i < 3; i++) {
		digitalWrite (LED_BUILTIN, HIGH);
		delay (50);
		digitalWrite (LED_BUILTIN, LOW);
		delay (50);
	}
}

void loop () {
	decodeGPS ();

	// u8g Picture loop
	u8g.firstPage ();
	do {
		draw ();
	} while (u8g.nextPage ());

	logPosition ();
}

/* This is valid for the EU, for other places see
 * http://www.webexhibits.org/daylightsaving/i.html
 *
 * Someone please check the formula anyway :D.
 */
boolean dstOffset () {
	byte d = day (), m = month (), y = year (), h = hour ();
	if ((m == 3 && d >= (31 - (5 * y / 4 + 4) % 7) && h >= 1) ||
	  (m > 3 && m < 10) ||
	  (m == 10 && d <= (31 - (5 * y / 4 + 1) % 7) && h < 1))
		return 1;
	else
		return 0;
}

void decodeGPS () {
	unsigned long start = millis ();
	do {
		while (GPS_SERIAL.available ()) {
			char c = GPS_SERIAL.read ();
			//DPRINT (c);
			gps.encode (c);
		}
	} while (millis () - start < GPS_INTERVAL);

	unsigned long age;
	gps.f_get_position (&fix.pos.lat, &fix.pos.lon, &age);
	fix.pos.valid = age != TinyGPS::GPS_INVALID_AGE;

	fix.alt.value = gps.f_altitude ();
	fix.alt.valid = fix.alt.value != TinyGPS::GPS_INVALID_F_ALTITUDE;

	fix.course.value = gps.f_course ();
	fix.course.valid = fix.course.value != TinyGPS::GPS_INVALID_F_ANGLE;

	fix.speed.value = gps.f_speed_kmph ();
	fix.speed.valid = fix.speed.value != TinyGPS::GPS_INVALID_F_SPEED;

	fix.nsats = gps.satellites () != TinyGPS::GPS_INVALID_SATELLITES ? gps.satellites () : 0;

	// Set the time to the latest GPS reading
	int year;
	gps.crack_datetime (&year, &fix.time.Month, &fix.time.Day, &fix.time.Hour, &fix.time.Minute, &fix.time.Second, NULL, &age);
	fix.time.Year = year - 1970;

	if (age < 600) {
		time_t tt = makeTime (fix.time);
        setTime (tt);
        adjustTime ((timeZoneOffset + dstOffset ()) * SECS_PER_HOUR);
	}
}


#define GPS_LOG_COLS 9

void logPosition () {
	// GPS Logfile
	// Column names taken from http://www.gpsbabel.org/htmldoc-development/fmt_unicsv.html
	static const char* cols[GPS_LOG_COLS] = {
		"no",
		"date",
		"time",
		"lat",
		"lon",
		"ele",
		"speed",
		"head",
		"sat"
	};

	time_t tt = makeTime (fix.time);
	if (tt == lastLoggedFix) {
		DPRINTLN (F("Skipping log, because fix unchanged"));
	} else if (!fix.pos.valid) {
		DPRINTLN (F("Skipping log because no fix detected"));
	} else {
		if (lastLogMillis == 0 || millis () - lastLogMillis >= INTx) {
			DPRINTLN (F("Logging GPS fix"));

			if (!writer.openFile ("/gps.csv", GPS_LOG_COLS, cols)) {
				DPRINTLN (F("GPS CSV init failed"));
			} else {
				// Record No
				writer.newRecord ();
				writer.print (0);

				// Date
				writer.newField ();
				writer.print (fix.time.Year + 1970);
				writer.print ('/');
				if (fix.time.Month < 10)
					writer.print (0);
				writer.print (fix.time.Month);
				writer.print ('/');
				if (fix.time.Day < 10)
					writer.print (0);
				writer.print (fix.time.Day);

				// Time
				writer.newField ();
				if (fix.time.Hour < 10)
					writer.print (0);
				writer.print (fix.time.Hour);
				writer.print (':');
				if (fix.time.Minute < 10)
					writer.print (0);
				writer.print (fix.time.Minute);
				writer.print (':');
				if (fix.time.Second < 10)
					writer.print (0);
				writer.print (fix.time.Second);
				//~ writer.print ('.');
				//~ if (hundredths < 100)
					//~ writer.print (0);
				//~ if (hundredths < 10)
					//~ writer.print (0);
				//~ writer.print (hundredths);

				// Latitude
				writer.newField ();
				writer.print (fix.pos.lat, LATLON_PREC);

				// Longitude
				writer.newField ();
				writer.print (fix.pos.lon, LATLON_PREC);

				// Altitude
				writer.newField ();
				if (fix.alt.valid)
					writer.print (fix.alt.value);

				// Speed (knots)
				writer.newField ();
				if (fix.speed.valid)
					writer.print (fix.speed.value);

				// Course/Track/Heading
				writer.newField ();
				if (fix.course.valid)
					writer.print (fix.course.value);

				// Number of sats
				writer.newField ();
				writer.print (fix.nsats);

				writer.endFile ();

				lastLoggedFix = tt;
			}

			lastLogMillis = millis ();
		}
	}
}
