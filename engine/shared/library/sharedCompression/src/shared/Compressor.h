// =====================================================================
//
// Compressor.h
//
// Portionc Copyright 1999 Bootprint Entertainment
// Portions Copyright 2003 Sony Online Entertainment
//
// =====================================================================

#ifndef INCLUDED_Compressor_H
#define INCLUDED_Compressor_H

// =====================================================================

class Compressor
{
public:

	Compressor();
	virtual ~Compressor();

	virtual int  compress(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize) = 0;
	virtual int  expand  (const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize) = 0;

	virtual void compress(const char *inputFile, const char *outputFile) = 0;
	virtual void expand  (const char *inputFile, const char *outputFile) = 0;

private:

	Compressor(const Compressor &);
	Compressor &operator =(const Compressor &);
};

// =====================================================================

#endif
