#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/* Define to actually enable SD support. If disabled logging will be redirected
 * to the serial monitor. Useful fo debugging.
 */
//~ #define ENABLE_SD
#define SD_CHIPSELECT 4

/* Number of decimal digits to use when logging latitude and longitude.
 *
 * 6 digits allows for precision up to 4 inches. See here for details:
 * http://lightmanufacturingsystems.com/heliostats/support/decimal-latitude-longitude-accuracy/
 */
#define LATLON_PREC 6

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

// FIXME: Remove
#define INTx 15000

/* DEFINE this to DISABLE debug messages
 */
//~ #define NDEBUG

#endif
