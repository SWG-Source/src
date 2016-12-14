// ======================================================================
//
// PaletteArgbList.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PaletteArgbList_H
#define INCLUDED_PaletteArgbList_H

// ======================================================================

class CrcString;
class LessPointerComparator;
class PaletteArgb;

// ======================================================================
/**
 * Manages PaletteArgb assets loaded via the TreeFile system.
 *
 * The PaletteArgbList coordinates handing out reference-counted
 * PaletteArgb instances loaded from the TreeFile system.  It ensures
 * only a single instance of a Palette file is loaded at any given time.
 */

class PaletteArgbList
{
friend class PaletteArgb;

public:

	static void install();

	static const PaletteArgb *fetch(const CrcString &pathName);

	static void assignAsynchronousLoaderFunctions();

private:

	typedef std::map<const CrcString *, PaletteArgb*, LessPointerComparator>  ResourceMap;

private:

	static void remove();

	static void               stopTracking(const PaletteArgb &palette);
	static const void        *asynchronousLoaderFetchNoCreate(char const *fileName);
	static void               asynchronousLoaderRelease(void const *palette);

	static const PaletteArgb *fetch(const CrcString &pathName, bool create);


private:

	static bool         ms_installed;
	static ResourceMap *ms_resourceMap;

};

// ======================================================================

#endif
