// ======================================================================
//
// ServerSpaceTerrainAppearanceTemplate.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerSpaceTerrainAppearanceTemplate_H
#define INCLUDED_ServerSpaceTerrainAppearanceTemplate_H

// ======================================================================

#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"

class Iff;

// ======================================================================

class ServerSpaceTerrainAppearanceTemplate : public SpaceTerrainAppearanceTemplate
{
public:

	static void install();

	static AppearanceTemplate * create(char const * filename, Iff * iff);

public:

	ServerSpaceTerrainAppearanceTemplate(char const * filename, Iff * iff);
	virtual ~ServerSpaceTerrainAppearanceTemplate();

	virtual Appearance * createAppearance() const;

private:

	static void remove();

private:

	ServerSpaceTerrainAppearanceTemplate();
	ServerSpaceTerrainAppearanceTemplate(ServerSpaceTerrainAppearanceTemplate const &);
	ServerSpaceTerrainAppearanceTemplate & operator=(ServerSpaceTerrainAppearanceTemplate const &);
}; 

// ======================================================================

#endif
 
