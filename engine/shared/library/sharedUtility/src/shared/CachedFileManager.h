// ======================================================================
// 
// CachedFileManager.h
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_CachedFileManager_H
#define INCLUDED_CachedFileManager_H

// ======================================================================

class CachedFileManager
{
public:

	static void install(bool allowFileCaching);

	static void preloadSomeAssets ();
	static bool donePreloading ();
	static int  getLoadingPercent ();
};

// ======================================================================

#endif
