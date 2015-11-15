// ======================================================================
//
// ZlibCompressor.h
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ZlibCompressor_H
#define INCLUDED_ZlibCompressor_H

// ======================================================================

#include "sharedCompression/Compressor.h"

// ======================================================================

class ZlibCompressor : public Compressor
{
public:

	static void install(int numberOfParallelThreads);

public:

	ZlibCompressor();
	virtual ~ZlibCompressor();

	virtual int  compress(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize);
	virtual int  expand  (const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize);

	virtual void compress(const char *inputFile, const char *outputFile);
	virtual void expand  (const char *inputFile, const char *outputFile);

private:

	ZlibCompressor(const ZlibCompressor &);
	ZlibCompressor &operator =(const ZlibCompressor &);
};

// ======================================================================

#endif
