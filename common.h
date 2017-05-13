#ifndef _COMMON_H_INCLUDED
#define _COMMON_H_INCLUDED

// We want enums to only use 1 byte
#ifdef __GNUC__
	#define _PACKED_ __attribute__ ((packed))
#else
	#define _PACKED_
#endif

#endif
