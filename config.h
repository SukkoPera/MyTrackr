#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/* Define to actually enable SD support through the SdFatLib library. If both
 * this and ENABLE_SD_FAT16 are not defined, logging will be redirected to the
 * serial monitor, useful fo debugging.
 */
//~ #define ENABLE_SD_FATLIB

/* Define this to enable SD support through the FAT16 library. This saves flash
 * space but has a number of restrictions, such as only supporting 8+3 filenames
 * and not supporting SDHC (i.e. > 2 GB) cards.
 *
 * Get the library at https://github.com/greiman/Fat16
 */
#define ENABLE_SD_FAT16

// SD Slave-Select (SS, also known as CS) pin
#define SD_CHIPSELECT 4

/* Number of decimal digits to use when logging latitude and longitude.
 *
 * 6 digits allows for precision up to 4 inches. See here for details:
 * http://lightmanufacturingsystems.com/heliostats/support/decimal-latitude-longitude-accuracy/
 */
#define LATLON_PREC 7

// Default logging interval (sec)
#define DEFAULT_LOG_INTERVAL 10

/* Default logging distance: position will not be logged unless it is at least
 * this far from the previously logged position (meters, 0 = disabled)
 */
#define DEFAULT_LOG_DISTANCE 0

/* Default log rotation interval: NOT YET IMPLEMENTED
 */
#define DEFAULT_LOG_ROTATION LOGROT_DAILY

/* Default offset hours from gps time (UTC).
 * Ideally, it should be possible to learn the time zone based on the GPS
 * position data.  However, that would require a complex library, probably
 * incorporating some sort of database using Eric Muller's time zone shape maps,
 * at http://efele.net/maps/tz/
 *
 * We default to Central European Time (GMT+1).
 */
#define DEFAULT_TZ_OFFSET 1

/* Number of lines on screen to use for the menu. Font-size dependent
 */
#define MENU_LINES 5

// Pins keys are connected to. PREV can also be left undefined.
//~ #define KEY_PREV_PIN A2
#define KEY_NEXT_PIN A1
#define KEY_SELECT_PIN A0


/* DEFINE this to DISABLE debug messages
 */
#define NDEBUG

#endif
