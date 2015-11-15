// ======================================================================
//
// PopulationList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/PopulationList.h"

#include "Archive/AutoByteStream.h"

// ======================================================================

PopulationList::PopulationList()
{
}

// ----------------------------------------------------------------------

PopulationList::~PopulationList()
{
}

// ----------------------------------------------------------------------

void PopulationList::setPopulation(const std::string &scene, int x, int z, int population)
{
	m_populationMap[Location(scene,x,z)]=population;
	m_populationCache.clear();
}

// ----------------------------------------------------------------------

int PopulationList::getNearestPopulation(const std::string &scene, int x, int z) const
{
	Location where(scene,x,z);
	PopulationMapType::const_iterator i=m_populationMap.find(where);
	if (i!=m_populationMap.end())
		return i->second;
	else
	{
		i=m_populationCache.find(where);
		if (i!=m_populationCache.end())
			return i->second;
		else
		{
			int closestSoFar=-1;
			int population=0;
			for (i=m_populationMap.begin(); i!=m_populationMap.end(); ++i)
			{
				if (i->first.getScene()==scene)
				{
					int distX=x - i->first.getX();
					int distZ=z - i->first.getZ();
					int distSquared = (distX*distX)+(distZ*distZ);
					if (closestSoFar==-1 || distSquared < closestSoFar)
					{
						closestSoFar = distSquared;
						population = i->second;
					}
				}
			}
			m_populationCache.insert(std::make_pair(where,population));
			return population;
		}
	}
}

// ----------------------------------------------------------------------

void PopulationList::update(const PopulationList &newData)
{
	for (PopulationMapType::const_iterator i=newData.m_populationMap.begin(); i!=newData.m_populationMap.end(); ++i)
	{
		setPopulation(i->first.getScene(), i->first.getX(), i->first.getZ(), i->second);
	}   
}

// ======================================================================

PopulationList::Location::Location(const std::string &scene, int x, int z) :
		m_scene(scene),
		m_x(x),
		m_z(z)
{
}

// ----------------------------------------------------------------------

bool PopulationList::Location::operator<(const Location &rhs) const
{
	if (m_z < rhs.m_z)
		return true;
	else
	{
		if (m_z == rhs.m_z)
		{
			if (m_x < rhs.m_x)
				return true;
			else
			{
				if (m_x == rhs.m_x)
					return (m_scene < rhs.m_scene);
			}
		}
	}
	
	return false;   
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PopulationList & target)
	{
		size_t numEntries;
		get(source,numEntries);
		for (size_t count=0; count < numEntries; ++count)
		{
			std::string scene;
			int x;
			int z;
			int population;
			get(source,scene);
			get(source,x);
			get(source,z);
			get(source,population);
			target.setPopulation(scene,x,z,population);
		}
	}
	
	void put(ByteStream & target, const PopulationList & source)
	{
		put(target,source.m_populationMap.size());
		for (PopulationList::PopulationMapType::const_iterator i=source.m_populationMap.begin(); i!=source.m_populationMap.end(); ++i)
		{
			put(target,i->first.getScene());
			put(target,i->first.getX());
			put(target,i->first.getZ());
			put(target,i->second);
		}
	}
}

// ======================================================================
