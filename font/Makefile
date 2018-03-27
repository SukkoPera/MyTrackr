u8g_font_mytrackr.h: mytrackr-6x10.bdf
	bdf2u8g -f 1 -b 32 -e 127 mytrackr-6x10.bdf u8g_font_mytrackr u8g_font_mytrackr.h
	sed -i 's/"u8g.h"/<U8glib.h>/' u8g_font_mytrackr.h

clean:
	rm u8g_font_mytrackr.h
