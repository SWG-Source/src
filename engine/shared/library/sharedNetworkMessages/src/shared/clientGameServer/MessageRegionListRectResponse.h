// MessageRegionListRectResponse.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_MessageRegionListRectResponse_H
#define	_INCLUDED_MessageRegionListRectResponse_H

//-----------------------------------------------------------------------

//#include "sharedNetworkMessages/MessageRegionListResponse.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class MessageRegionListRectResponse : public GameNetworkMessage
{
public:
	MessageRegionListRectResponse(float ll_worldX, float ll_worldZ, float ur_worldX, float ur_worldZ, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String & name, const std::string & planet);
	MessageRegionListRectResponse(Archive::ReadIterator & source);
	~MessageRegionListRectResponse();

	const Unicode::String & getName         () const;
	const std::string &     getPlanet       () const;
	float                   getWorldX       () const;
	float                   getWorldZ       () const;
	int                     getPvP          () const;
	int                     getBuildable    () const;
	int                     getSpawnable    () const;
	int                     getMission      () const;
	int                     getMunicipal    () const;
	int                     getGeographical () const;
	int                     getMinDifficulty() const;
	int                     getMaxDifficulty() const;
	float                   getURWorldX     () const;
	float                   getURWorldZ     () const;

private:
	MessageRegionListRectResponse & operator = (const MessageRegionListRectResponse & rhs);
	MessageRegionListRectResponse(const MessageRegionListRectResponse & source);
	
private:
	Archive::AutoVariable<Unicode::String>  m_name;
	Archive::AutoVariable<std::string>      m_planet;
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
	Archive::AutoVariable<float>            m_radius;
	Archive::AutoVariable<float>            m_ur_worldX;
	Archive::AutoVariable<float>            m_ur_worldZ;
};

//-----------------------------------------------------------------------

inline float MessageRegionListRectResponse::getURWorldX() const
{
	return m_ur_worldX.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListRectResponse::getURWorldZ() const
{
	return m_ur_worldZ.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & MessageRegionListRectResponse::getName() const
{
	return m_name.get();
}

//-----------------------------------------------------------------------

inline const std::string & MessageRegionListRectResponse::getPlanet() const
{
	return m_planet.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListRectResponse::getWorldX() const
{
	return m_worldX.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListRectResponse::getWorldZ() const
{
	return m_worldZ.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getPvP() const
{
	return m_pvp.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getBuildable() const
{
	return m_buildable.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getSpawnable() const
{
	return m_spawnable.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getMission() const
{
	return m_mission.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getMunicipal() const
{
	return m_municipal.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getGeographical() const
{
	return m_geographical.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getMinDifficulty() const
{
	return m_minDifficulty.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListRectResponse::getMaxDifficulty() const
{
	return m_maxDifficulty.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageRegionListRectResponse_H
