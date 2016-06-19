#include "CSVWriter.h"
#include "debug.h"

#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)
#define F_TO_PSTR(s) reinterpret_cast<PGM_P> (s)
#define FlashString const __FlashStringHelper *


#ifdef ENABLE_SD
SdFat CSVWriter::SD;
#endif

boolean CSVWriter::begin (byte ssPin) {
#ifdef ENABLE_SD
	return SD.begin (ssPin);
#else
	return true;
#endif
}

boolean CSVWriter::openFile (const char* _path, int _ncols, const char* const cols[]) {
	path = _path;
	ncols = _ncols;

#ifdef ENABLE_SD
	if (!SD.exists (path)) {
#endif
		// File does not exist, create it and write header line
		newRecord ();
		for (byte i = 0; i < ncols; i++) {
			FlashString colName = PSTR_TO_F (pgm_read_word (&(cols[i])));
			print (colName);
			if (i < ncols - 1)
				newField ();
		}
#ifdef ENABLE_SD
	}
#endif

	return true;
}

size_t CSVWriter::write (uint8_t c) {
#ifdef ENABLE_SD
	return file.write (c);
#else
	return Serial.write (c);
#endif
}

boolean CSVWriter::endFile () {
	boolean ret = true;

	println ();

#ifdef ENABLE_SD
	if (file) {
		file.close ();
	} else {
		ret = false;
	}
#endif

	return ret;
}

boolean CSVWriter::newRecord () {
	boolean ret = true;

#ifdef ENABLE_SD
	if (!file) {
		file = SD.open (path, FILE_WRITE);
		if (!file) {
			DPRINTLN (F("Cannot open log file"));
			ret = false;
		}
	} else {
#endif
		// File is already open, prepare for new record
		println ();
#ifdef ENABLE_SD
	}
#endif

	return ret;
}

void CSVWriter::newField () {
	print (',');
}
