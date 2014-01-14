// ======================================================================
//
// ServerSpaceTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ServerSpaceTerrainAppearanceTemplate.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/ServerSpaceTerrainAppearance.h"

// ======================================================================
// STATIC PUBLIC ServerSpaceTerrainAppearanceTemplate
// ======================================================================

void ServerSpaceTerrainAppearanceTemplate::install()
{
	AppearanceTemplateList::assignBinding(cms_spaceTerrainAppearanceTemplateTag, ServerSpaceTerrainAppearanceTemplate::create);

	ExitChain::add(ServerSpaceTerrainAppearanceTemplate::remove, "ServerSpaceTerrainAppearanceTemplate::remove");
}

// ----------------------------------------------------------------------

AppearanceTemplate * ServerSpaceTerrainAppearanceTemplate::create(char const * const filename, Iff * const iff)
{
	return new ServerSpaceTerrainAppearanceTemplate(filename, iff);
}

// ======================================================================
// PUBLIC ServerSpaceTerrainAppearanceTemplate
// ======================================================================

ServerSpaceTerrainAppearanceTemplate::ServerSpaceTerrainAppearanceTemplate(char const * const filename, Iff * const iff) :
	SpaceTerrainAppearanceTemplate(filename, iff)
{
}

// ----------------------------------------------------------------------

ServerSpaceTerrainAppearanceTemplate::~ServerSpaceTerrainAppearanceTemplate()
{
}

// ----------------------------------------------------------------------

Appearance * ServerSpaceTerrainAppearanceTemplate::createAppearance() const
{
	return new ServerSpaceTerrainAppearance(this);
}

// ======================================================================
// STATIC PRIVATE ServerSpaceTerrainAppearanceTemplate
// ======================================================================

void ServerSpaceTerrainAppearanceTemplate::remove()
{
	AppearanceTemplateList::removeBinding(cms_spaceTerrainAppearanceTemplateTag);
}

// ======================================================================

