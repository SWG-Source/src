//===================================================================
//
// ServerProceduralTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ServerProceduralTerrainAppearanceTemplate.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ServerProceduralTerrainAppearance.h"

//===================================================================

void ServerProceduralTerrainAppearanceTemplate::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ServerProceduralTerrainAppearanceTemplate::install\n"));

	AppearanceTemplateList::assignBinding (TAG (M,P,T,A), ServerProceduralTerrainAppearanceTemplate::create);
	AppearanceTemplateList::assignBinding (TAG (P,T,A,T), ServerProceduralTerrainAppearanceTemplate::create);

	ExitChain::add (remove, "ServerProceduralTerrainAppearanceTemplate::remove");
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearanceTemplate::remove ()
{
	AppearanceTemplateList::removeBinding (TAG (M,P,T,A));
	AppearanceTemplateList::removeBinding (TAG (P,T,A,T));
}

//-------------------------------------------------------------------
	
AppearanceTemplate* ServerProceduralTerrainAppearanceTemplate::create (const char* filename, Iff* iff)
{
	return new ServerProceduralTerrainAppearanceTemplate (filename, iff);
}

//===================================================================

ServerProceduralTerrainAppearanceTemplate::ServerProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff) :
	ProceduralTerrainAppearanceTemplate (filename, iff, false, 0, 2)
{
	unsigned mapsToSample = 
		  TGM_vertexPosition
		| TGM_shader
		| TGM_passable
		| TGM_exclude
	;
	
	if (getLegacyMode())
	{
		mapsToSample|=TGM_floraStaticCollidable;
	}

	m_terrainGenerator->setMapsToSample(mapsToSample);
}

//-------------------------------------------------------------------

ServerProceduralTerrainAppearanceTemplate::~ServerProceduralTerrainAppearanceTemplate ()
{
}

//-------------------------------------------------------------------

Appearance* ServerProceduralTerrainAppearanceTemplate::createAppearance () const
{
	return new ServerProceduralTerrainAppearance (this);
}

//===================================================================
