// MessageRegionListResponse.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_MessageRegionListResponse_H
#define	_INCLUDED_MessageRegionListResponse_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class MessageRegionListResponse : public GameNetworkMessage
{
public:
	MessageRegionListResponse(const NetworkId& networkId, int gameServerId, float worldX, float worldZ, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String& label);
	MessageRegionListResponse(const std::string& messageName, const NetworkId& networkId, int gameServerId, float worldX, float worldZ, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String& label);
	MessageRegionListResponse(Archive::ReadIterator & source);
	~MessageRegionListResponse();

	const NetworkId &        getNetworkId     () const;
	const Unicode::String &  getLabel         () const;
	float                    getWorldX        () const;
	float                    getWorldZ        () const;
	int                      getPvP           () const;
	int                      getBuildable     () const;
	int                      getSpawnable     () const;
	int                      getMission       () const;
	int                      getMunicipal     () const;
	int                      getGeographical  () const;
	int                      getMinDifficulty () const;
	int                      getMaxDifficulty () const;
	int                      getGameServerId  () const;

private:
	MessageRegionListResponse & operator = (const MessageRegionListResponse & rhs);
	MessageRegionListResponse(const MessageRegionListResponse & source);
	
	Archive::AutoVariable<NetworkId>        m_objectId;
	Archive::AutoVariable<Unicode::String>  m_label;
	Archive::AutoVariable<float>            m_worldX;
	Archive::AutoVariable<float>            m_worldZ;
	Archive::AutoVariable<int>              m_pvp;
	Archive::AutoVariable<int>              m_buildable;
	Archive::AutoVariable<int>              m_spawnable;
	Archive::AutoVariable<int>              m_mission;
	Archive::AutoVariable<int>              m_municipal;
	Archive::AutoVariable<int>              m_geographical;
	Archive::AutoVariable<int>              m_minDifficulty;
	Archive::AutoVariable<int>              m_maxDifficulty;
	Archive::AutoVariable<int>              m_gameServerId;
};

//-----------------------------------------------------------------------

inline const NetworkId & MessageRegionListResponse::getNetworkId() const
{
	return m_objectId.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & MessageRegionListResponse::getLabel() const
{
	return m_label.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListResponse::getWorldX() const
{
	return m_worldX.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListResponse::getWorldZ() const
{
	return m_worldZ.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getPvP() const
{
	return m_pvp.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getBuildable() const
{
	return m_buildable.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getSpawnable() const
{
	return m_spawnable.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getMission() const
{
	return m_mission.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getMunicipal() const
{
	return m_municipal.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getGeographical() const
{
	return m_geographical.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getGameServerId() const
{
	return m_gameServerId.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getMinDifficulty() const
{
	return m_minDifficulty.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListResponse::getMaxDifficulty() const
{
	return m_maxDifficulty.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageRegionListResponse_H
