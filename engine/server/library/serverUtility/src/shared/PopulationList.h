// ======================================================================
//
// PopulationList.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PopulationList_H
#define INCLUDED_PopulationList_H

// ======================================================================

#include <string>
#include <map>

// ======================================================================

class PopulationList;
namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PopulationList & target);
	void put(ByteStream & target, const PopulationList & source);
}

// ======================================================================

/**
 * A class that manages a list of locations and the number of players
 * in each location.
 */

class PopulationList
{
  public:
	PopulationList();
	~PopulationList();
	
  public:
	void setPopulation(const std::string &scene, int x, int z, int population);
	int getNearestPopulation(const std::string &scene, int x, int z) const;
	void update(const PopulationList &newData);

  private:
	class Location
	{
	  public:
		Location(const std::string &scene, int x, int z);
		bool operator< (const Location &rhs) const;

	  public:
		const std::string &getScene() const;
		int getX() const;
		int getZ() const;
		
	  private:
		std::string m_scene;
		int m_x;
		int m_z;
	};

	typedef std::map<Location, int> PopulationMapType;
	PopulationMapType m_populationMap;
	mutable PopulationMapType m_populationCache;

	friend void Archive::get(Archive::ReadIterator & source, PopulationList & target);
	friend void Archive::put(Archive::ByteStream & target, const PopulationList & source);
};

// ======================================================================

inline const std::string &PopulationList::Location::getScene() const
{
	return m_scene;
}

// ----------------------------------------------------------------------

inline int PopulationList::Location::getX() const
{
	return m_x;
}

// ----------------------------------------------------------------------

inline int PopulationList::Location::getZ() const
{
	return m_z;
}

// ======================================================================

#endif
