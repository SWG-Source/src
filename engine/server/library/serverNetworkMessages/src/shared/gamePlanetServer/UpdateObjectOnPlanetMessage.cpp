// ======================================================================
//
// UpdateObjectOnPlanetMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

UpdateObjectOnPlanetMessage::UpdateObjectOnPlanetMessage(const NetworkId &objectId, const NetworkId &topmostContainer, int x, int y, int z, int interestRadius, int locationReservationRadius, bool watched, bool requiresSimulation, const int objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType) :
		GameNetworkMessage("UpdateObjectOnPlanetMessage"),
		m_objectId(objectId),
		m_topmostContainer(topmostContainer),
		m_x(x),
		m_y(y),
		m_z(z),
		m_interestRadius(interestRadius),
		m_locationReservationRadius(locationReservationRadius),
		m_objectTypeTag(objectTypeTag),
		m_watched(watched),
		m_requiresSimulation(requiresSimulation),
		m_level(level),
		m_hibernating(hibernating),
		m_templateCrc(templateCrc),
		m_aiActivity(aiActivity),
		m_creationType(creationType)
{
	addVariable(m_objectId);
	addVariable(m_topmostContainer);
	addVariable(m_x);
	addVariable(m_y);
	addVariable(m_z);
	addVariable(m_interestRadius);
	addVariable(m_locationReservationRadius);
	addVariable(m_objectTypeTag);
	addVariable(m_watched);
	addVariable(m_requiresSimulation);
	addVariable(m_level);
	addVariable(m_hibernating);
	addVariable(m_templateCrc);
	addVariable(m_aiActivity);
	addVariable(m_creationType);
}

// ----------------------------------------------------------------------

UpdateObjectOnPlanetMessage::UpdateObjectOnPlanetMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("UpdateObjectOnPlanetMessage"),
		m_objectId(),
		m_topmostContainer(),
		m_x(),
		m_y(),
		m_z(),
		m_interestRadius(),
		m_locationReservationRadius(),
		m_objectTypeTag(),
		m_watched(),
		m_requiresSimulation(),
		m_level(0),
		m_hibernating(false),
		m_templateCrc(0),
		m_aiActivity(-1),
		m_creationType(-1)
{
	addVariable(m_objectId);
	addVariable(m_topmostContainer);
	addVariable(m_x);
	addVariable(m_y);
	addVariable(m_z);
	addVariable(m_interestRadius);
	addVariable(m_locationReservationRadius);
	addVariable(m_objectTypeTag);
	addVariable(m_watched);
	addVariable(m_requiresSimulation);
	addVariable(m_level);
	addVariable(m_hibernating);
	addVariable(m_templateCrc);
	addVariable(m_aiActivity);
	addVariable(m_creationType);

	unpack(source);
}

//-----------------------------------------------------------------------

UpdateObjectOnPlanetMessage::~UpdateObjectOnPlanetMessage()
{
}

// ======================================================================
