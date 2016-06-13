#include "CSVWriter.h"
#include "debug.h"

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

boolean CSVWriter::openFile (const char* _path, int _ncols, const char* cols[]) {
	path = _path;
	ncols = _ncols;

#ifdef ENABLE_SD
	if (!SD.exists (path)) {
#endif
		// File does not exist, create it and write header line
		if (cols != NULL) {
			byte i;

			newRecord ();

			for (i = 0; i < ncols - 1; i++) {
				print (cols[i]);
				newField ();
			}

			print (cols[i]);
			//~ endFile ();
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
		// File is already open, prepare for new record
		println ();
	}
#endif

	return ret;
}

void CSVWriter::newField () {
	print (',');
}
