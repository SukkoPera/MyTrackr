/*
 * U8glib:
 * Lo sketch usa 27.392 byte (84%) dello spazio disponibile per i programmi. Il massimo è 32.256 byte.
 * Le variabili globali usano 1.430 byte (69%) di memoria dinamica, lasciando altri 618 byte liberi per le variabili locali. Il massimo è 2.048 byte.
 */

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
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display (-1);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

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
	ValidFloat hdop;
	TimeElements time;
	unsigned int nsats;
};

GpsFix fix;

time_t lastLoggedFix;
unsigned long lastLogMillis = 0;

void draw () {
  display.clearDisplay ();
  
	// Sat icon (Blink if fix is not valid)
	time_t tt = makeTime (fix.time) + (timeZoneOffset + dstOffset ()) * SECS_PER_HOUR;
	if ((fix.pos.valid && timeStatus () == timeSet && now () - tt < 10) || ((millis () / 1000) % 2) == 0) {
    display.drawXBitmap (0, 0, sat_bits, sat_width, sat_height, 1);
	}

	// Number of sats in view
  display.setTextSize (1);
  display.setTextColor (WHITE);
  display.setCursor (20, 3);
  display.print (fix.nsats);

	// Main
	switch (((millis () / 1000L) / 7) % 3) {
		case 0:
			// Position icon
      display.drawXBitmap (20, 33, position_bits, position_width, position_height, 1);

			// Coordinates
			if (fix.pos.valid) {
				display.setCursor (52, 32);
				display.print (fix.pos.lat, 6);
				display.setCursor (52, 42);
				display.print (fix.pos.lon, 6);
			} else {
				display.setCursor (52, 32);
				display.print (F("N/A"));
				display.setCursor (52, 42);
				display.print (F("N/A"));
			}
			break;
		case 1: {
			// Compass icon
      display.drawXBitmap (20, 33, compass_bits, compass_width, compass_height, 1);

			display.setCursor (52, 32);
			if (fix.course.valid) {
				display.print (fix.course.value);
				display.print ((char) 176);   // Degrees symbol, font-dependent
			} else {
				display.print (F("N/A"));
			}

			display.setCursor (52, 42);
			if (fix.speed.valid) {
				display.print (fix.speed.value);
				display.print (F(" km/h"));
			} else {
				display.print (F("N/A"));
			}
			break;
		} case 2:
			// Clock icon
     display.drawXBitmap (20, 33, clock_bits, clock_width, clock_height, 1);

			if (timeStatus () != timeNotSet) {
				display.setCursor (52, 32);
				display.print (day ());
				display.print ('/');
				display.print (month ());
				display.print ('/');
				display.print (year ());

				display.setCursor (52, 42);
				if (hour () < 10)
					display.print (0);
				display.print (hour ());
				display.print (':');
				if (minute () < 10)
					display.print (0);
				display.print (minute ());
				display.print (':');
				if (second () < 10)
					display.print (0);
				display.print (second ());
			} else {
				display.setCursor (52, 32);
				display.print (F("N/A"));
				display.setCursor (52, 42);
				display.print (F("N/A"));
			}
			break;
	}

	// SD icon (blink if SD failed)
#ifdef ENABLE_SD
	if (sdAvailable || ((millis () / 1000) % 2) == 0) {
    display.drawXBitmap (90, 0, microsd_bits, microsd_width, microsd_height, 1);
	}
#endif

	// Battery icon, drawn by hand
	int top = (HEADER_HEIGHT - BAT_HEIGHT) / 2;
	display.drawRect (128 - BAT_WIDTH, top, BAT_WIDTH, BAT_HEIGHT, 1);  // Outside box
	display.fillRect (128 - BAT_WIDTH - BAT_TIP, top + (BAT_HEIGHT - BAT_HEIGHT / 2) / 2, BAT_TIP, BAT_HEIGHT / 2, 1);  // Tip
	display.fillRect (128 - BAT_WIDTH + 2, top + 2, BAT_WIDTH - 4, BAT_HEIGHT - 4, 1);  // Filling (FIXME: Make proportional to charge)

  display.display ();
}

void setup () {
	DSTART (9600);

  display.begin (SSD1306_SWITCHCAPVCC, 0x3C);

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
	fix.hdop.valid = false;
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
	logPosition ();
  draw ();
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

	fix.hdop.value = gps.hdop () / 100.0;
	fix.hdop.valid = gps.hdop () != TinyGPS::GPS_INVALID_HDOP;

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


#define GPS_LOG_COLS 10

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
		"hdop",
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

				// Horizontal Dilution of Precision (HDOP)
				writer.newField ();
				if (fix.hdop.valid)
					writer.print (fix.hdop.value);

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
