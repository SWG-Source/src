// ======================================================================
//
// CreateDynamicSpawnRegionCircleMessage.h
//
// Copyright 2009 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_CreateDynamicSpawnRegionCircleMessage_H
#define	_INCLUDED_CreateDynamicSpawnRegionCircleMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class CreateDynamicSpawnRegionCircleMessage: public GameNetworkMessage
{
public:
	CreateDynamicSpawnRegionCircleMessage(float centerX, float centerY, float centerZ, float radius, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify, std::string spawntable, int duration);
	CreateDynamicSpawnRegionCircleMessage(Archive::ReadIterator &source);

	float getCenterX() const;
	float getCenterY() const;
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
	std::string const &getSpawntable() const;
	int getDuration() const;

private:
	CreateDynamicSpawnRegionCircleMessage(CreateDynamicSpawnRegionCircleMessage const &);
	CreateDynamicSpawnRegionCircleMessage &operator=(CreateDynamicSpawnRegionCircleMessage const &);

	Archive::AutoVariable<float> m_centerX;
	Archive::AutoVariable<float> m_centerY;
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
	Archive::AutoVariable<std::string> m_spawnTable;
	Archive::AutoVariable<int>  m_duration;
};

// ----------------------------------------------------------------------

inline float CreateDynamicSpawnRegionCircleMessage::getCenterX() const
{
	return m_centerX.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicSpawnRegionCircleMessage::getCenterY() const
{
	return m_centerY.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicSpawnRegionCircleMessage::getCenterZ() const
{
	return m_centerZ.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicSpawnRegionCircleMessage::getRadius() const
{
	return m_radius.get();
}

// ----------------------------------------------------------------------

inline Unicode::String const &CreateDynamicSpawnRegionCircleMessage::getName() const
{
	return m_name.get();
}

// ----------------------------------------------------------------------

inline std::string const &CreateDynamicSpawnRegionCircleMessage::getPlanet() const
{
	return m_planet.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getPvp() const
{
	return m_pvp.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getBuildable() const
{
	return m_buildable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getMunicipal() const
{
	return m_municipal.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getGeography() const
{
	return m_geography.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getMinDifficulty() const
{
	return m_minDifficulty.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getMaxDifficulty() const
{
	return m_maxDifficulty.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getSpawnable() const
{
	return m_spawnable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getMission() const
{
	return m_mission.get();
}

// ----------------------------------------------------------------------

inline bool CreateDynamicSpawnRegionCircleMessage::getVisible() const
{
	return m_visible.get();
}

// ----------------------------------------------------------------------

inline bool CreateDynamicSpawnRegionCircleMessage::getNotify() const
{
	return m_notify.get();
}
// ----------------------------------------------------------------------

inline std::string const &CreateDynamicSpawnRegionCircleMessage::getSpawntable() const
{
	return m_spawnTable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicSpawnRegionCircleMessage::getDuration() const
{
	return m_duration.get();
}

// ======================================================================

#endif	// _INCLUDED_CreateDynamicSpawnRegionCircleMessage_H

