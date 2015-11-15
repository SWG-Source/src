//========================================================================
//
// ToggleAvatarLoginStatus.h - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ToggleAvatarLoginStatus_H
#define	_INCLUDED_ToggleAvatarLoginStatus_H

//-----------------------------------------------------------------------

#include <string>
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------

class ToggleAvatarLoginStatus : public GameNetworkMessage
{
public:
	ToggleAvatarLoginStatus  (std::string const & clusterName, unsigned int stationId, const NetworkId & toCharacterId, bool enabled);
	ToggleAvatarLoginStatus  (Archive::ReadIterator & source);
	~ToggleAvatarLoginStatus ();

	const std::string & getClusterName () const;
	const NetworkId &  getCharacterId  () const;
	unsigned int       getStationId    () const;
	bool               getEnabled      () const;

private:
	Archive::AutoVariable<std::string>   m_clusterName;
	Archive::AutoVariable<NetworkId>     m_characterId;
	Archive::AutoVariable<unsigned int>  m_stationId;
	Archive::AutoVariable<bool>          m_enabled;

	ToggleAvatarLoginStatus();
	ToggleAvatarLoginStatus(const ToggleAvatarLoginStatus&);
	ToggleAvatarLoginStatus& operator= (const ToggleAvatarLoginStatus&);
};

//-----------------------------------------------------------------------

inline const std::string & ToggleAvatarLoginStatus::getClusterName() const
{
	return m_clusterName.get();
}

//-----------------------------------------------------------------------

inline unsigned int ToggleAvatarLoginStatus::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & ToggleAvatarLoginStatus::getCharacterId() const
{
	return m_characterId.get();
}

//-----------------------------------------------------------------------

inline bool ToggleAvatarLoginStatus::getEnabled() const
{
	return m_enabled.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ToggleAvatarLoginStatus_H
