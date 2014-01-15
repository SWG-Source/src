// ======================================================================
//
// CreateDynamicRegionRectangleMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_CreateDynamicRegionRectangleMessage_H
#define	_INCLUDED_CreateDynamicRegionRectangleMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class CreateDynamicRegionRectangleMessage: public GameNetworkMessage
{
public:
	CreateDynamicRegionRectangleMessage(float minX, float minZ, float maxX, float maxZ, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify);
	CreateDynamicRegionRectangleMessage(Archive::ReadIterator &source);

	float getMinX() const;
	float getMinZ() const;
	float getMaxX() const;
	float getMaxZ() const;
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
	CreateDynamicRegionRectangleMessage(CreateDynamicRegionRectangleMessage const &);
	CreateDynamicRegionRectangleMessage &operator=(CreateDynamicRegionRectangleMessage const &);

	Archive::AutoVariable<float> m_minX;
	Archive::AutoVariable<float> m_minZ;
	Archive::AutoVariable<float> m_maxX;
	Archive::AutoVariable<float> m_maxZ;
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

inline float CreateDynamicRegionRectangleMessage::getMinX() const
{
	return m_minX.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicRegionRectangleMessage::getMinZ() const
{
	return m_minZ.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicRegionRectangleMessage::getMaxX() const
{
	return m_maxX.get();
}

// ----------------------------------------------------------------------

inline float CreateDynamicRegionRectangleMessage::getMaxZ() const
{
	return m_maxZ.get();
}

// ----------------------------------------------------------------------

inline Unicode::String const &CreateDynamicRegionRectangleMessage::getName() const
{
	return m_name.get();
}

// ----------------------------------------------------------------------

inline std::string const &CreateDynamicRegionRectangleMessage::getPlanet() const
{
	return m_planet.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getPvp() const
{
	return m_pvp.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getBuildable() const
{
	return m_buildable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getMunicipal() const
{
	return m_municipal.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getGeography() const
{
	return m_geography.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getMinDifficulty() const
{
	return m_minDifficulty.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getMaxDifficulty() const
{
	return m_maxDifficulty.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getSpawnable() const
{
	return m_spawnable.get();
}

// ----------------------------------------------------------------------

inline int CreateDynamicRegionRectangleMessage::getMission() const
{
	return m_mission.get();
}

// ----------------------------------------------------------------------

inline bool CreateDynamicRegionRectangleMessage::getVisible() const
{
	return m_visible.get();
}

// ----------------------------------------------------------------------

inline bool CreateDynamicRegionRectangleMessage::getNotify() const
{
	return m_notify.get();
}

// ======================================================================

#endif	// _INCLUDED_CreateDynamicRegionRectangleMessage_H

