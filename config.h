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
#define SD_CHIPSELECT 10

// Time a fix is considered valid (sec)
#define DATA_VALID_TIME 10

/* Number of decimal digits to use when logging latitude and longitude.
 *
 * 6 digits allows for precision up to 4 inches. See here for details:
 * http://lightmanufacturingsystems.com/heliostats/support/decimal-latitude-longitude-accuracy/
 */
#define LATLON_PREC 6

// Default logging interval (sec)
#define DEFAULT_LOG_INTERVAL 10

/* Define to enable log distance menu
 */
#define ENABLE_DISTANCE_MENU

/* Default logging distance: position will not be logged unless it is at least
 * this far from the previously logged position (meters, 0 = disabled)
 */
#define DEFAULT_LOG_DISTANCE 0

/* Define to enable log rotation menu
 */
#define ENABLE_ROTATION_MENU

/* Default log rotation interval
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

/* Pins keys are connected to. NEXT and SELECT are required, BACK and PREV can
 * be left undefined.
 *
 * Note that by default these are on PD4-7, corresponding to PCINT20-23.
 */
#define KEY_BACK_PIN 7
#define KEY_PREV_PIN 6
#define KEY_NEXT_PIN 5
#define KEY_SELECT_PIN 4

#define ENABLE_SOFT_POWEROFF
#define KEY_INT_VECT PCINT2_vect
#define KEY_PCICR_BIT PCIE2
#define KEY_PCMSK_REG PCMSK2
#ifdef KEY_BACK_PIN
#define PCMSK_BIT_BACK (1 << PCINT20)
#else
#define PCMSK_BIT_BACK 0
#endif
#ifdef KEY_PREV_PIN
#define PCMSK_BIT_PREV (1 << PCINT21)
#else
#define PCMSK_BIT_PREV 0
#endif
#define KEY_PCMSK_BITS (PCMSK_BIT_BACK | PCMSK_BIT_PREV | (1 << PCINT22) | (1 << PCINT23))
// Time interval that SELECT+NEXT must be held to toggle keylock (ms)
#define KEY_LOCK_DELAY 2000

// Time interval from last keypress to turn screen off (ms)
#define SCREEN_OFF_DELAY 30000UL

// "Alive led": pin led is connected to
#define ALIVE_LED_PIN 9

// "Alive led": blink for 10 ms every 15 s
#define ALIVE_LED_ON_TIME 10
#define ALIVE_LED_OFF_TIME 15000UL

// Pin that controls SD/GPS/Screen power (through a MOSFET, of course)
#define PERIPHERALS_POWER_PIN 8

// Analog pin used to measure battery level
#define BATTERY_PIN A0

/* Number of times to sample the battery for each measure (up to 255) and delay
 * between measures (ms).
 */
#define BATTERY_ITERATIONS 10
#define BATTERY_IT_DELAY 20

// Battery measurement interval (s)
#define BATTERY_INTERVAL 60

// Number of ADC steps
#define BATTERY_STEPS 1024

/* Battery level is measured against the internal 1.1v reference, but such
 * reference can be up to 10% off. If you want measurements to be more accurate,
 * measure the AREF pin for your particular chip and insert its value here (mV).
 */
#define REAL_1_1_REF 1100

/* Since the raw battery voltage is greater than 1.1v, a voltage divider is
 * needed to bring the level to one that can be measured. We recommend using a
 * 1M + 330K resistor combination. If you want measurements to be more accurate,
 * measure your resistors with a multimeter and put the exact values here.
 *
 * Note that R1 must always be the bigger resistor.
 */
#define BATTERY_R1 1018000UL
#define BATTERY_R2 331000UL

/* Battery voltage points used to indicate remaining battery charge. These can
 * be fine-tuned as described on the wiki.
 *
 * The default values are fine with my 18650's.
 */
const byte BATTERY_POINTS_NO = 5;
const unsigned int BATTERY_POINT_OFFSET = 150;
const byte BATTERY_POINTS[BATTERY_POINTS_NO] PROGMEM = {222, 204, 191, 185, 166};

/* DEFINE this to DISABLE debug messages
 */
#define NDEBUG

#endif
