#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define ENABLE_SD
#define SD_CHIPSELECT 4

/* Number of decimal digits to use when logging latitude and longitude.
 *
 * 6 allows for precision to 4 inches. See here for details:
 * http://lightmanufacturingsystems.com/heliostats/support/decimal-latitude-longitude-accuracy/
 */
#define LATLON_PREC 6

// Interval between two consecutive log updates
#define INTx 15000

//~ #define NDEBUG

#endif
