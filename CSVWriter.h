#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_SD
#include <SdFat.h>
#endif

class CSVWriter: public Print {
public:
	boolean begin (byte ssPin);

	boolean openFile (const char* _path, int _ncols, const char* cols[]);

	boolean endFile ();

	boolean newRecord ();

	void newField ();

protected:
	size_t write (uint8_t c);

private:
#ifdef ENABLE_SD
	static SdFat SD;
	File file;
#endif
	const char* path;
	byte ncols;
};

#endif
