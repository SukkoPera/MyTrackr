#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <Arduino.h>
#include "debug.h"

#ifdef ENABLE_SD
#include <SD.h>
#endif

class CSVWriter: public Print {
public:
	boolean begin (const char* _path, int _ncols, const char* cols[]);

	boolean begin (const char* _path, int _ncols);

	boolean end ();

	boolean newRecord ();

	void newField ();

protected:
	size_t write (uint8_t c);

private:
#ifdef ENABLE_SD
	File file;
#endif
	const char* path;
	byte ncols;
};

#endif
