// ======================================================================
//
// ZlibCompressor.cpp
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedCompression/FirstSharedCompression.h"
#include "sharedCompression/ZlibCompressor.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedSynchronization/Mutex.h"

#include "zlib.h"

#include <vector>

// ======================================================================

namespace ZlibCompressorNamespace
{
	void   remove();
	voidpf allocateWrapper OF((voidpf opaque, uInt items, uInt size));
	void   freeWrapper OF((voidpf opaque, voidpf address));

	int const           cms_poolElementThreshold = 65536;
	int                 ms_poolElementCount;

	Mutex               ms_mutex;
	std::vector<void *> ms_memoryPool;
	void *              ms_memoryBottom;
	void *              ms_memoryTop;
}
using namespace ZlibCompressorNamespace;

// ======================================================================

voidpf ZlibCompressorNamespace::allocateWrapper(voidpf opaque, uInt items, uInt size)
{
	UNREF(opaque);
		void *result = 0;

	int totalSize = items * size;
	if (totalSize > cms_poolElementThreshold)
	{
		result = operator new(totalSize);
	}
	else
	{
		ms_mutex.enter();

			if (ms_memoryPool.empty())
			{
				result = operator new(totalSize);
			}
			else
			{
				result = ms_memoryPool.back();
				ms_memoryPool.pop_back();
			}

		ms_mutex.leave();
	}

	return result;
}

// ----------------------------------------------------------------------

void ZlibCompressorNamespace::freeWrapper(voidpf opaque, voidpf address)
{
	UNREF(opaque);

	if (address < ms_memoryBottom || address >= ms_memoryTop)
		operator delete(address);
	else
	{
		ms_mutex.enter();
			ms_memoryPool.push_back(address);
		ms_mutex.leave();
	}
}

// ======================================================================

void ZlibCompressor::install(int numberOfParallelThreads)
{
	if (numberOfParallelThreads <= 1)
		ms_poolElementCount = 5;
	else
		if (numberOfParallelThreads <= 2)
			ms_poolElementCount = 10;
		else
			ms_poolElementCount = 15;

	int const size = cms_poolElementThreshold * ms_poolElementCount;
	ms_memoryBottom = operator new(size);
	ms_memoryTop = reinterpret_cast<byte*>(ms_memoryBottom) + size;
	for (int i = 0; i < ms_poolElementCount; ++i)
		ms_memoryPool.push_back(reinterpret_cast<byte*>(ms_memoryBottom) + (i * cms_poolElementThreshold));

	ExitChain::add(ZlibCompressorNamespace::remove, "ZlibCompressorNamespace::remove");
}

// ----------------------------------------------------------------------

void ZlibCompressorNamespace::remove()
{
	ms_mutex.enter();

		DEBUG_FATAL(static_cast<int>(ms_memoryPool.size()) != ms_poolElementCount, ("ZLibCompressor memory pool entries not all released"));
		operator delete(ms_memoryBottom);
		ms_memoryBottom = nullptr;
		ms_memoryTop = nullptr;
		ms_memoryPool.clear();

	ms_mutex.leave();
}

// ======================================================================

ZlibCompressor::ZlibCompressor()
: Compressor()
{
}

// ----------------------------------------------------------------------

ZlibCompressor::~ZlibCompressor()
{
}

// ----------------------------------------------------------------------

int ZlibCompressor::compress(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize)
{
	z_stream z;

	z.next_in = reinterpret_cast<Bytef *>(const_cast<void *>(inputBuffer));
	z.avail_in = inputSize;
	z.total_in = 0;

	z.next_out = reinterpret_cast<Bytef *>(outputBuffer);
	z.avail_out = outputSize;
	z.total_out = 0;

	z.msg = nullptr;
	z.state = nullptr;

	z.zalloc = allocateWrapper;
	z.zfree = freeWrapper;
	z.opaque = nullptr;

	z.data_type = Z_BINARY;
	z.adler = 0;
	z.reserved = 0;

  if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK)
		return -1;

	if (deflate(&z, Z_FINISH) != Z_STREAM_END)
	{
		deflateEnd(&z);
		return -1;
	}

	int const size = z.total_out;
	if (deflateEnd(&z) != Z_OK)
		return -1;

	return size;
}

// ----------------------------------------------------------------------

int ZlibCompressor::expand(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize)
{
	z_stream z;

	z.next_in = reinterpret_cast<Bytef *>(const_cast<void *>(inputBuffer));
	z.avail_in = inputSize;
	z.total_in = 0;

	z.next_out = reinterpret_cast<Bytef *>(outputBuffer);
	z.avail_out = outputSize;
	z.total_out = 0;

	z.msg = nullptr;
	z.state = nullptr;

	z.zalloc = allocateWrapper;
	z.zfree = freeWrapper;
	z.opaque = nullptr;

	z.data_type = Z_BINARY;
	z.adler = 0;
	z.reserved = 0;

  if (inflateInit(&z) != Z_OK)
		return -1;

	if (inflate(&z, Z_FINISH) != Z_STREAM_END)
	{
		inflateEnd(&z);
		return -1;
	}

	int const size = z.total_out;
	if (inflateEnd(&z) != Z_OK)
		return -1;

	return size;
}

// ----------------------------------------------------------------------

void ZlibCompressor::compress(const char *inputFile, const char *outputFile)
{
	UNREF(inputFile);
	UNREF(outputFile);
}

// ----------------------------------------------------------------------

void ZlibCompressor::expand(const char *inputFile, const char *outputFile)
{
	UNREF(inputFile);
	UNREF(outputFile);
}

// ======================================================================
