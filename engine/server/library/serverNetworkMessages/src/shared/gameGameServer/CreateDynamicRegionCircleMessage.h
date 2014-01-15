// ======================================================================
//
// CreateDynamicRegionCircleMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_CreateDynamicRegionCircleMessage_H
#define	_INCLUDED_CreateDynamicRegionCircleMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class CreateDynamicRegionCircleMessage: public GameNetworkMessage
{
public:
	CreateDynamicRegionCircleMessage(float centerX, float centerZ, float radius, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify);
	CreateDynamicRegionCircleMessage(Archive::ReadIterator &source);

	float getCenterX() const;
	float getCenterZ() const;
	float getRadius() const;
	Unicode::String const &getName() const;
	std::string const &getPlanet() const;
	int getPvp() const;
	int getBuildable() const;
	int getMunicipal() const;
	int getGeography() const;
	int getMinDifficulty() const;
	int getMaxDifficulty() const;
	int getSpawnable() const;
	int getMission() const;
	bool getVisible() const;
	bool getNotify() const;

private:
	CreateDynamicRegionCircleMessage(CreateDynamicRegionCircleMessage const &);
	CreateDynamicRegionCircleMessage &operator=(CreateDynamicRegionCircleMessage const &);

	Archive::AutoVariable<float> m_centerX;
	Archive::AutoVariable<float> m_centerZ;
	Archive::AutoVariable<float> m_radius;
	Archive::AutoVariable<Unicode::String> m_name;
	Archive::AutoVariable<std::string> m_planet;
	Archive::AutoVariable<int> m_pvp;
	Archive::AutoVariable<int> m_buildable;
	Archive::AutoVariable<int> m_municipal;
	Archive::AutoVariable<int> m_geography;
	Archive::AutoVariable<int> m_minDifficulty;
	Archive::AutoVariable<int> m_maxDifficulty;
	Archive::AutoVariable<int> m_spawnable;
	Archive::AutoVariable<int> m_mission;
	Archive::AutoVariable<bool> m_visible;
	Archive::AutoVariable<bool> m_notify;
};

// ----------------------------------------------------------------------

inline float CreateDynamicRegionCircleMessage::getCenterX() const
{
	return m_centerX.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicRegionCircleMessage::getCenterZ() const
{
	return m_centerZ.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicRegionCircleMessage::getRadius() const
{
	return m_radius.get();
}

// ----------------------------------------------------------------------

inline Unicode::String const &CreateDynamicRegionCircleMessage::getName() const
{
	return m_name.get();
}

// ----------------------------------------------------------------------

inline std::string const &CreateDynamicRegionCircleMessage::getPlanet() const
{
	return m_planet.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getPvp() const
{
	return m_pvp.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getBuildable() const
{
	return m_buildable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getMunicipal() const
{
	return m_municipal.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getGeography() const
{
	return m_geography.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getMinDifficulty() const
{
	return m_minDifficulty.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getMaxDifficulty() const
{
	return m_maxDifficulty.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getSpawnable() const
{
	return m_spawnable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionCircleMessage::getMission() const
{
	return m_mission.get();
}

// ----------------------------------------------------------------------

inline bool CreateDynamicRegionCircleMessage::getVisible() const
{
	return m_visible.get();
}

// ----------------------------------------------------------------------

inline bool CreateDynamicRegionCircleMessage::getNotify() const
{
	return m_notify.get();
}

// ======================================================================

#endif	// _INCLUDED_CreateDynamicRegionCircleMessage_H

