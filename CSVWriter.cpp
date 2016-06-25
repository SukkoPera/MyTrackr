#include "CSVWriter.h"
#include "debug.h"

#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)
#define F_TO_PSTR(s) reinterpret_cast<PGM_P> (s)
#define FlashString const __FlashStringHelper *

#if defined (ENABLE_SD_FATLIB)
	SdFat CSVWriter::SD;
#elif defined (ENABLE_SD_FAT16)
	SdCard CSVWriter::SD;
#endif

boolean CSVWriter::begin (byte ssPin) {
#if defined (ENABLE_SD_FATLIB)
	return SD.begin (ssPin);
#elif defined (ENABLE_SD_FAT16)
	return SD.begin (ssPin) && Fat16::init (&SD);
#else
	return true;
#endif
}

boolean CSVWriter::openFile (const char* path, byte ncols, const char* const cols[]) {
	boolean ret = true;

#if defined (ENABLE_SD_FATLIB) || defined (ENABLE_SD_FAT16)
	if (!file.open (path, O_WRITE | O_APPEND)) {
		// File does not exist, create it and write header line
		if (!file.open (path, O_WRITE | O_CREAT)) {
			DPRINT (F("File open failed"));
			if (SD.errorCode) {
				DPRINT (F(": 0x"));
				DPRINTLN (SD.errorCode, HEX);
			} else {
				DPRINTLN ();
			}

			ret = false;
		} else {
#endif
			// File does not exist, create it and write header line
			for (byte i = 0; i < ncols; i++) {
				FlashString colName = PSTR_TO_F (pgm_read_word (&(cols[i])));
				print (colName);
				if (i < ncols - 1)
					newField ();
			}

			endRecord ();
#if defined (ENABLE_SD_FATLIB) || defined (ENABLE_SD_FAT16)
		}
	}
#endif

	return ret;
}

size_t CSVWriter::write (uint8_t c) {
#if defined (ENABLE_SD_FATLIB) || defined (ENABLE_SD_FAT16)
	return file.write (c);
#else
	return Serial.write (c);
#endif
}

boolean CSVWriter::closeFile () {
	boolean ret = true;

#if defined (ENABLE_SD_FATLIB) || defined (ENABLE_SD_FAT16)
	if (file.isOpen ()) {
#endif
		ret = file.close ();
	}

	return ret;
}

void CSVWriter::endRecord () {
	println ();
}

void CSVWriter::newField () {
	print (',');
}
