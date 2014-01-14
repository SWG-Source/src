//===================================================================
//
// ServerProceduralTerrainAppearanceTemplate.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ServerProceduralTerrainAppearanceTemplate_h
#define INCLUDED_ServerProceduralTerrainAppearanceTemplate_h

//===================================================================

#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"

class Iff;

//===================================================================

class ServerProceduralTerrainAppearanceTemplate : public ProceduralTerrainAppearanceTemplate
{
public:

	static void install ();

	static AppearanceTemplate* create (const char* filename, Iff* iff);

public:

	ServerProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff);
	virtual ~ServerProceduralTerrainAppearanceTemplate ();

	virtual Appearance* createAppearance () const;

private:

	static void remove ();

private:

	ServerProceduralTerrainAppearanceTemplate ();
	ServerProceduralTerrainAppearanceTemplate (const ServerProceduralTerrainAppearanceTemplate&);
	ServerProceduralTerrainAppearanceTemplate& operator= (const ServerProceduralTerrainAppearanceTemplate&);
}; 

//===================================================================

#endif
 
