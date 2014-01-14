// ======================================================================
//
// AsynchronousLoader.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AsynchronousLoader_H
#define INCLUDED_AsynchronousLoader_H

// ======================================================================

class AsynchronousLoader
{
public:

	typedef void        (*Callback)(void *data);
	typedef const void *(*FetchFunction)(const char *fileName);
	typedef void        (*ReleaseFunction)(const void *resource);

public:

	static void install(const char *fileName);
	static bool isInstalled();

	static void disable();
	static void enable();
	static bool isEnabled();
	static bool isIdle();

	static void bindFetchReleaseFunctions(const char *extension, FetchFunction fetchFunction, ReleaseFunction releaseFunction);

	static void add(const char *fileName, Callback callback, void *data);
	static void remove(Callback callback, void *data);
	static void processCallbacks();
};

// ======================================================================

#endif
