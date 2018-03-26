#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <Arduino.h>
#include "config.h"

#if defined (ENABLE_SD_FATLIB)
#include <SdFat.h>
#elif defined (ENABLE_SD_FAT16)
#include <Fat16.h>
#endif

class CSVWriter: public Print {
public:
	boolean begin (byte ssPin);

	void end ();

	boolean openFile (const char* _path, byte _ncols, const char* const cols[]);

	boolean closeFile ();

	void endRecord ();

	void newField ();

protected:
	size_t write (uint8_t c);

private:
#if defined (ENABLE_SD_FATLIB)
	static SdFat SD;
	File file;
#elif defined (ENABLE_SD_FAT16)
	static SdCard SD;
	Fat16 file;
#endif
};

#endif
