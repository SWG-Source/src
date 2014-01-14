// ======================================================================
//
// BakedTerrainReader.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_BakedTerrainReader_H
#define INCLUDED_BakedTerrainReader_H

// ======================================================================

class BakedTerrain;

// ======================================================================

class BakedTerrainReader
{
public:

	static BakedTerrain * create (char const * fileName);
};

// ======================================================================

#endif

