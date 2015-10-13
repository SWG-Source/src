// ======================================================================
//
// CityInfo.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _CityInfo_H_
#define _CityInfo_H_

// ======================================================================

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/VectorArchive.h"

class CityInfo;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, CityInfo & target);
	void put(ByteStream & target, const CityInfo & source);
}

// ======================================================================

class CityInfo
{
	friend void Archive::get(Archive::ReadIterator & source, CityInfo & target);
	friend void Archive::put(Archive::ByteStream & target, const CityInfo & source);

public:
	CityInfo();
	CityInfo(CityInfo const &);
	~CityInfo();

	bool operator== (const CityInfo& rhs) const;
	bool operator!= (const CityInfo& rhs) const;

	std::string const &getCityName() const;
	NetworkId const &getCityHallId() const;
	std::string const &getPlanet() const;
	int getX() const;
	int getZ() const;
	int getRadius() const;
	uint32 getFaction() const;
	std::string const &getGcwDefenderRegion() const;
	int getTimeJoinedGcwDefenderRegion() const;
	int getCreationTime() const;
	NetworkId const &getLeaderId() const;
	int getCitizenCount() const;
	int getStructureCount() const;
	int getIncomeTax() const;
	int getPropertyTax() const;
	int getSalesTax() const;
	Vector const &getTravelLoc() const;
	int getTravelCost() const;
	bool getTravelInterplanetary() const;
	Vector const &getCloneLoc() const;
	Vector const &getCloneRespawn() const;
	NetworkId const &getCloneRespawnCell() const;
	NetworkId const &getCloneId() const;

	void setCityInfo(std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId);
	void setCityName(std::string const &cityName);
	void setCityHall(NetworkId const &cityHallId);
	void setCityLocation(std::string const &cityPlanet, int x, int z);
	void setCityRadius(int radius);
	void setCityFaction(uint32 faction);
	void setCityGcwDefenderRegion(std::string const &gcwDefenderRegion);
	void setCityTimeJoinedGcwDefenderRegion(int timeJoinedGcwDefenderRegion);
	void setCityCreationTime(int creationTime);
	void setCityLeader(NetworkId const &leaderId);
	void setCitizenCount(int citizenCount);
	void setStructureCount(int structureCount);
	void setCityIncomeTax(int incomeTax);
	void setCityPropertyTax(int propertyTax);
	void setCitySalesTax(int salesTax);
	void setCityTravelInfo(Vector const &travelLoc, int travelCost, bool travelInterplanetary);
	void setCityCloneInfo(Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId);

private:
	std::string m_cityName;
	NetworkId m_cityHallId;
	std::string m_planet;
	int m_x, m_z, m_radius;
	uint32 m_faction;
	std::string m_gcwDefenderRegion;
	int m_timeJoinedGcwDefenderRegion;
	int m_creationTime;
	NetworkId m_leaderId;
	int m_citizenCount;
	int m_structureCount;
	int m_incomeTax, m_propertyTax, m_salesTax;
	Vector m_travelLoc;
	int m_travelCost;
	bool m_travelInterplanetary;
	Vector m_cloneLoc;
	Vector m_cloneRespawn;
	NetworkId m_cloneRespawnCell;
	NetworkId m_cloneId;
};

// ----------------------------------------------------------------------

inline CityInfo::CityInfo() :
	m_cityName(),
	m_cityHallId(NetworkId::cms_invalid),
	m_planet(),
	m_x(0),
	m_z(0),
	m_radius(0),
	m_faction(0),
	m_gcwDefenderRegion(),
	m_timeJoinedGcwDefenderRegion(0),
	m_creationTime(0),
	m_leaderId(NetworkId::cms_invalid),
	m_citizenCount(0),
	m_structureCount(0),
	m_incomeTax(0),
	m_propertyTax(0),
	m_salesTax(0),
	m_travelLoc(Vector::zero),
	m_travelCost(0),
	m_travelInterplanetary(false),
	m_cloneLoc(Vector::zero),
	m_cloneRespawn(Vector::zero),
	m_cloneRespawnCell(NetworkId::cms_invalid),
	m_cloneId(NetworkId::cms_invalid)
{
}

// ----------------------------------------------------------------------

inline CityInfo::CityInfo(CityInfo const &rhs) :
	m_cityName(rhs.m_cityName),
	m_cityHallId(rhs.m_cityHallId),
	m_planet(rhs.m_planet),
	m_x(rhs.m_x),
	m_z(rhs.m_z),
	m_radius(rhs.m_radius),
	m_faction(rhs.m_faction),
	m_gcwDefenderRegion(rhs.m_gcwDefenderRegion),
	m_timeJoinedGcwDefenderRegion(rhs.m_timeJoinedGcwDefenderRegion),
	m_creationTime(rhs.m_creationTime),
	m_leaderId(rhs.m_leaderId),
	m_citizenCount(rhs.m_citizenCount),
	m_structureCount(rhs.m_structureCount),
	m_incomeTax(rhs.m_incomeTax),
	m_propertyTax(rhs.m_propertyTax),
	m_salesTax(rhs.m_salesTax),
	m_travelLoc(rhs.m_travelLoc),
	m_travelCost(rhs.m_travelCost),
	m_travelInterplanetary(rhs.m_travelInterplanetary),
	m_cloneLoc(rhs.m_cloneLoc),
	m_cloneRespawn(rhs.m_cloneRespawn),
	m_cloneRespawnCell(rhs.m_cloneRespawnCell),
	m_cloneId(rhs.m_cloneId)
{
}

// ----------------------------------------------------------------------

inline CityInfo::~CityInfo()
{
}

// ----------------------------------------------------------------------

inline std::string const &CityInfo::getCityName() const
{
	return m_cityName;
}

// ----------------------------------------------------------------------

inline NetworkId const &CityInfo::getCityHallId() const
{
	return m_cityHallId;
}

// ----------------------------------------------------------------------

inline std::string const &CityInfo::getPlanet() const
{
	return m_planet;
}

// ----------------------------------------------------------------------

inline int CityInfo::getX() const
{
	return m_x;
}

// ----------------------------------------------------------------------

inline int CityInfo::getZ() const
{
	return m_z;
}

// ----------------------------------------------------------------------

inline int CityInfo::getRadius() const
{
	return m_radius;
}

// ----------------------------------------------------------------------

inline uint32 CityInfo::getFaction() const
{
	return m_faction;
}

// ----------------------------------------------------------------------

inline std::string const &CityInfo::getGcwDefenderRegion() const
{
	return m_gcwDefenderRegion;
}

// ----------------------------------------------------------------------

inline int CityInfo::getTimeJoinedGcwDefenderRegion() const
{
	return m_timeJoinedGcwDefenderRegion;
}

// ----------------------------------------------------------------------

inline int CityInfo::getCreationTime() const
{
	return m_creationTime;
}

// ----------------------------------------------------------------------

inline NetworkId const &CityInfo::getLeaderId() const
{
	return m_leaderId;
}

// ----------------------------------------------------------------------

inline int CityInfo::getCitizenCount() const
{
	return m_citizenCount;
}

// ----------------------------------------------------------------------

inline int CityInfo::getStructureCount() const
{
	return m_structureCount;
}

// ----------------------------------------------------------------------

inline int CityInfo::getIncomeTax() const
{
	return m_incomeTax;
}

// ----------------------------------------------------------------------

inline int CityInfo::getPropertyTax() const
{
	return m_propertyTax;
}

// ----------------------------------------------------------------------

inline int CityInfo::getSalesTax() const
{
	return m_salesTax;
}

// ----------------------------------------------------------------------

inline Vector const &CityInfo::getTravelLoc() const
{
	return m_travelLoc;
}

// ----------------------------------------------------------------------

inline int CityInfo::getTravelCost() const
{
	return m_travelCost;
}

// ----------------------------------------------------------------------

inline bool CityInfo::getTravelInterplanetary() const
{
	return m_travelInterplanetary;
}

// ----------------------------------------------------------------------

inline Vector const &CityInfo::getCloneLoc() const
{
	return m_cloneLoc;
}

// ----------------------------------------------------------------------

inline Vector const &CityInfo::getCloneRespawn() const
{
	return m_cloneRespawn;
}

// ----------------------------------------------------------------------

inline NetworkId const &CityInfo::getCloneRespawnCell() const
{
	return m_cloneRespawnCell;
}

// ----------------------------------------------------------------------

inline NetworkId const &CityInfo::getCloneId() const
{
	return m_cloneId;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityInfo(std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId)
{
	m_cityName = cityName;
	m_cityHallId = cityHallId;
	m_planet = cityPlanet;
	m_x = x;
	m_z = z;
	m_radius = radius;
	m_leaderId = leaderId;
	m_incomeTax = incomeTax;
	m_propertyTax = propertyTax;
	m_salesTax = salesTax;
	m_travelLoc = travelLoc;
	m_travelCost = travelCost;
	m_travelInterplanetary = travelInterplanetary;
	m_cloneLoc = cloneLoc;
	m_cloneRespawn = cloneRespawn;
	m_cloneRespawnCell = cloneRespawnCell;
	m_cloneId = cloneId;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityName(std::string const &cityName)
{
	m_cityName = cityName;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityHall(NetworkId const &cityHallId)
{
	m_cityHallId = cityHallId;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityLocation(std::string const &cityPlanet, int x, int z)
{
	m_planet = cityPlanet;
	m_x = x;
	m_z = z;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityRadius(int radius)
{
	m_radius = radius;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityFaction(uint32 faction)
{
	m_faction = faction;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityGcwDefenderRegion(std::string const &gcwDefenderRegion)
{
	m_gcwDefenderRegion = gcwDefenderRegion;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityTimeJoinedGcwDefenderRegion(int timeJoinedGcwDefenderRegion)
{
	m_timeJoinedGcwDefenderRegion = timeJoinedGcwDefenderRegion;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityCreationTime(int creationTime)
{
	m_creationTime = creationTime;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityLeader(NetworkId const &leaderId)
{
	m_leaderId = leaderId;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCitizenCount(int citizenCount)
{
	m_citizenCount = citizenCount;
}

// ----------------------------------------------------------------------

inline void CityInfo::setStructureCount(int structureCount)
{
	m_structureCount = structureCount;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityIncomeTax(int incomeTax)
{
	m_incomeTax = incomeTax;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityPropertyTax(int propertyTax)
{
	m_propertyTax = propertyTax;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCitySalesTax(int salesTax)
{
	m_salesTax = salesTax;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityTravelInfo(Vector const &travelLoc, int travelCost, bool travelInterplanetary)
{
	m_travelLoc = travelLoc;
	m_travelCost = travelCost;
	m_travelInterplanetary = travelInterplanetary;
}

// ----------------------------------------------------------------------

inline void CityInfo::setCityCloneInfo(Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId)
{
	m_cloneLoc = cloneLoc;
	m_cloneRespawn = cloneRespawn;
	m_cloneRespawnCell = cloneRespawnCell;
	m_cloneId = cloneId;
}

// ======================================================================

inline void Archive::get(ReadIterator & source, CityInfo & target)
{
	get(source, target.m_cityName);
	get(source, target.m_cityHallId);
	get(source, target.m_planet);
	get(source, target.m_x);
	get(source, target.m_z);
	get(source, target.m_radius);
	get(source, target.m_faction);
	get(source, target.m_gcwDefenderRegion);
	get(source, target.m_timeJoinedGcwDefenderRegion);
	get(source, target.m_creationTime);
	get(source, target.m_leaderId);
	get(source, target.m_citizenCount);
	get(source, target.m_structureCount);
	get(source, target.m_incomeTax);
	get(source, target.m_propertyTax);
	get(source, target.m_salesTax);
	get(source, target.m_travelLoc);
	get(source, target.m_travelCost);
	get(source, target.m_travelInterplanetary);
	get(source, target.m_cloneLoc);
	get(source, target.m_cloneRespawn);
	get(source, target.m_cloneRespawnCell);
	get(source, target.m_cloneId);
}

// ----------------------------------------------------------------------

inline void Archive::put(ByteStream & target, const CityInfo & source)
{
	put(target, source.m_cityName);
	put(target, source.m_cityHallId);
	put(target, source.m_planet);
	put(target, source.m_x);
	put(target, source.m_z);
	put(target, source.m_radius);
	put(target, source.m_faction);
	put(target, source.m_gcwDefenderRegion);
	put(target, source.m_timeJoinedGcwDefenderRegion);
	put(target, source.m_creationTime);
	put(target, source.m_leaderId);
	put(target, source.m_citizenCount);
	put(target, source.m_structureCount);
	put(target, source.m_incomeTax);
	put(target, source.m_propertyTax);
	put(target, source.m_salesTax);
	put(target, source.m_travelLoc);
	put(target, source.m_travelCost);
	put(target, source.m_travelInterplanetary);
	put(target, source.m_cloneLoc);
	put(target, source.m_cloneRespawn);
	put(target, source.m_cloneRespawnCell);
	put(target, source.m_cloneId);
}

// ======================================================================

#endif // _CityInfo_H_
