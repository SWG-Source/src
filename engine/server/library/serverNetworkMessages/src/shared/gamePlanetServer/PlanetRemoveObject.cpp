// ======================================================================
//
// PlanetRemoveObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PlanetRemoveObject.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

PlanetRemoveObject::PlanetRemoveObject(NetworkId objectId) :
		GameNetworkMessage("PlanetRemoveObject"),
		m_objectId(objectId)
{
	addVariable(m_objectId);
}

// ----------------------------------------------------------------------

PlanetRemoveObject::PlanetRemoveObject(Archive::ReadIterator & source) :
		GameNetworkMessage("PlanetRemoveObject"),
		m_objectId()
{
	addVariable(m_objectId);

	unpack(source);
}

//-----------------------------------------------------------------------

PlanetRemoveObject::~PlanetRemoveObject()
{
}

// ======================================================================
