#include "debug.h"

#include <PString.h>

#include <SoftwareSerial.h>
SoftwareSerial ss (7, 8);
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

#ifdef ENABLE_SD
#include <SD.h>

boolean sdAvailable = false;
#endif

// Display
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g (U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

// Time
#include <TimeLib.h>

// FIXME: Put into config menu
// Offset hours from gps time (UTC)
const byte timeZoneOffset = 1;   // Central European Time
//const byte timeZoneOffset = -5;  // Eastern Standard Time (USA)
//const byte timeZoneOffset = -4;  // Eastern Daylight Time (USA)
//const byte timeZoneOffset = -8;  // Pacific Standard Time (USA)
//const byte timeZoneOffset = -7;  // Pacific Daylight Time (USA)

// Ideally, it should be possible to learn the time zone
// based on the GPS position data.  However, that would
// require a complex library, probably incorporating some
// sort of database using Eric Muller's time zone shape
// maps, at http://efele.net/maps/tz/

#define HEADER_HEIGHT 16

struct Position {
	float lat;
	float lon;
	float alt;		// Meters
	boolean valid;
};

struct ValidFloat {
	float value;
	boolean valid;
};

struct GpsFix {
	Position pos;
	ValidFloat course;
	ValidFloat speed;		// km/h
	unsigned long age;
	unsigned int nsats;
};

GpsFix fix;

void draw () {
	u8g.setFontPosTop ();

	// Sat icon
	u8g.drawXBMP (0, 0, sat_width, sat_height, sat_bits);

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
				u8g.print ("?");
				u8g.setPrintPos (52, 42);
				u8g.print ("?");
			}
			break;
		case 1: {
			// Compass icon
			u8g.drawXBMP (20, 33, compass_width, compass_height, compass_bits);

			u8g.setPrintPos (52, 32);
			if (fix.course.valid) {
				u8g.print (fix.course.value);
				u8g.print ((char) 176);   // degrees symbol
			} else {
				u8g.print ("N/A");
			}

			u8g.setPrintPos (52, 42);
			if (fix.speed.valid) {
				u8g.print (fix.speed.value);
				u8g.print (" km/h");
			} else {
				u8g.print ("N/A");
			}
			break;
		} case 2:
			// Clock icon
			u8g.drawXBMP (20, 33, clock_width, clock_height, clock_bits);

			if (timeStatus () != timeNotSet) {
				u8g.setPrintPos (52, 32);
				u8g.print (day ());
				u8g.print ("/");
				u8g.print (month ());
				u8g.print ("/");
				u8g.print (year ());

				u8g.setPrintPos (52, 42);
				if (hour () < 10)
					u8g.print (0);
				u8g.print (hour ());
				u8g.print (":");
				if (minute () < 10)
					u8g.print (0);
				u8g.print (minute ());
				u8g.print (":");
				if (second () < 10)
					u8g.print (0);
				u8g.print (second ());
			} else {
				u8g.setPrintPos (52, 32);
				u8g.print ("?");
				u8g.setPrintPos (52, 42);
				u8g.print ("?");
			}
			break;
	}

	// SD icon
#ifdef ENABLE_SD
	if (sdAvailable) {
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
	DSTART ();

	// See if the card is present and can be initialized:
#ifdef ENABLE_SD
	if (!SD.begin (SD_CHIPSELECT)) {
		DPRINTLN (F("Card failed, or not present"));
	} else {
		DPRINTLN (F("SD ok"));
		sdAvailable = true;
	}
#endif

	GPS_SERIAL.begin (GPS_BAUD);

	DPRINT (F("Using TinyGPS "));
	DPRINTLN (TinyGPS::library_version ());

	fix.pos.valid = false;
	fix.course.valid = false;
	fix.speed.valid = false;
	fix.age = TinyGPS::GPS_INVALID_AGE;

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

	gps.f_get_position (&fix.pos.lat, &fix.pos.lon, &fix.age);
	fix.pos.valid = fix.age != TinyGPS::GPS_INVALID_AGE;

	fix.course.value = gps.f_course ();
	fix.course.valid = fix.course.value != TinyGPS::GPS_INVALID_F_ANGLE;

	fix.speed.value = gps.f_speed_kmph ();
	fix.speed.valid = fix.speed.value != TinyGPS::GPS_INVALID_F_SPEED;

	fix.nsats = gps.satellites ();

	// Set the time to the latest GPS reading
	int year;
	byte month, day, hour, minute, second;
	gps.crack_datetime (&year, &month, &day, &hour, &minute, &second, NULL, &fix.age);
	if (fix.age < 600) {
        setTime (hour, minute, second, day, month, year);
        adjustTime ((timeZoneOffset + dstOffset ()) * SECS_PER_HOUR);
	}
}
