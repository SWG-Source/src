// ======================================================================
//
// Crc.h
// ala diaz
//
// copyright 1998 Bootprint Entertainment
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Crc_H
#define INCLUDED_Crc_H

// ======================================================================

// Functions that return the CRC value of either strings or blocks of data.

class Crc
{
public:

	// CRC of the empty string ("")
	static const uint32 crcNull;
	static const uint32 crcInit;

public:

	static uint32 calculate(const char *string);
	static uint32 calculate(const void *data, int len, uint32 initCrc = crcInit);
	static uint32 normalizeAndCalculate(const char *string);
};

// ======================================================================

#endif
