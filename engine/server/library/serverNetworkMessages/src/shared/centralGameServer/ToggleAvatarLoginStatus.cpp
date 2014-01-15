//========================================================================
//
// ToggleAvatarLoginStatus.cpp - disable a character on a server
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ToggleAvatarLoginStatus.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object that was created
 */
ToggleAvatarLoginStatus::ToggleAvatarLoginStatus(std::string const & clusterName, unsigned int stationId, const NetworkId & characterId, bool enabled) :
GameNetworkMessage("ToggleAvatarLoginStatus"),
m_clusterName(clusterName),
m_characterId(characterId),
m_stationId(stationId),
m_enabled(enabled)
{
	addVariable(m_clusterName);
	addVariable(m_characterId);
	addVariable(m_stationId);
	addVariable(m_enabled);
}	// ToggleAvatarLoginStatus::ToggleAvatarLoginStatus

//-----------------------------------------------------------------------

ToggleAvatarLoginStatus::ToggleAvatarLoginStatus(Archive::ReadIterator & source) :
GameNetworkMessage("ToggleAvatarLoginStatus"),
m_clusterName(),
m_characterId(),
m_stationId(),
m_enabled()
{
	addVariable(m_clusterName);
	addVariable(m_characterId);
	addVariable(m_stationId);
	addVariable(m_enabled);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ToggleAvatarLoginStatus::~ToggleAvatarLoginStatus()
{
}	// ToggleAvatarLoginStatus::~ToggleAvatarLoginStatus

//-----------------------------------------------------------------------
