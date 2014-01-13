// MessageRegionListCircleResponse.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_MessageRegionListCircleResponse_H
#define	_INCLUDED_MessageRegionListCircleResponse_H

//-----------------------------------------------------------------------

//#include "sharedNetworkMessages/MessageRegionListResponse.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class MessageRegionListCircleResponse : public GameNetworkMessage
{
public:
	MessageRegionListCircleResponse(float worldX, float worldZ, float radius, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String & name, const std::string & planet);
	MessageRegionListCircleResponse(Archive::ReadIterator & source);
	~MessageRegionListCircleResponse();

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
	float                   getRadius       () const;

private:
	MessageRegionListCircleResponse & operator = (const MessageRegionListCircleResponse & rhs);
	MessageRegionListCircleResponse(const MessageRegionListCircleResponse & source);
	
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
};

//-----------------------------------------------------------------------

inline float MessageRegionListCircleResponse::getRadius() const
{
	return m_radius.get();
}


//-----------------------------------------------------------------------

inline const Unicode::String & MessageRegionListCircleResponse::getName() const
{
	return m_name.get();
}

//-----------------------------------------------------------------------

inline const std::string & MessageRegionListCircleResponse::getPlanet() const
{
	return m_planet.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListCircleResponse::getWorldX() const
{
	return m_worldX.get();
}

//-----------------------------------------------------------------------

inline float MessageRegionListCircleResponse::getWorldZ() const
{
	return m_worldZ.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getPvP() const
{
	return m_pvp.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getBuildable() const
{
	return m_buildable.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getSpawnable() const
{
	return m_spawnable.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getMission() const
{
	return m_mission.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getMunicipal() const
{
	return m_municipal.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getGeographical() const
{
	return m_geographical.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getMinDifficulty() const
{
	return m_minDifficulty.get();
}

//-----------------------------------------------------------------------

inline int MessageRegionListCircleResponse::getMaxDifficulty() const
{
	return m_maxDifficulty.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageRegionListCircleResponse_H
