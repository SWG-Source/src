// Location.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/Location.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcStringTable.h"
#include <string>

//-----------------------------------------------------------------------

namespace LocationNamespace
{
	CrcStringTable * s_planetCrcStringTable = 0;

	struct TableAllocator
	{
		TableAllocator();
		~TableAllocator();
	};

	TableAllocator::TableAllocator()
	{
		if(! s_planetCrcStringTable)
			s_planetCrcStringTable = new CrcStringTable("misc/planet_crc_string_table.iff");
	}

	TableAllocator::~TableAllocator()
	{
		delete s_planetCrcStringTable;
	}
}
using namespace LocationNamespace;

//-----------------------------------------------------------------------

Location::Location() :
m_coordinates(),
m_cell(),
m_sceneIdCrc()
{
	static TableAllocator t;
}

//-----------------------------------------------------------------------

Location::Location(const Location & source) :
m_coordinates(source.m_coordinates),
m_cell(source.m_cell),
m_sceneIdCrc(source.m_sceneIdCrc)
{
}

//-----------------------------------------------------------------------

Location::Location(const Vector & coordinates, const NetworkId & cell, unsigned int sceneIdCrc) :
m_coordinates(coordinates),
m_cell(cell),
m_sceneIdCrc(sceneIdCrc)
{
}

//-----------------------------------------------------------------------

Location::~Location()
{
}


//-----------------------------------------------------------------------

Location & Location::operator = (const Location & rhs)
{
	if(&rhs != this)
	{
		m_coordinates = rhs.m_coordinates;
		m_cell = rhs.m_cell;
		m_sceneIdCrc = rhs.m_sceneIdCrc;
	}
	return *this;
}

//-----------------------------------------------------------------------

bool Location::operator != (const Location & rhs) const
{
	return (m_coordinates != rhs.m_coordinates || m_cell != rhs.m_cell || m_sceneIdCrc != rhs.m_sceneIdCrc);
}

//-----------------------------------------------------------------------

bool Location::operator < (const Location & rhs) const
{
	if (m_sceneIdCrc != rhs.m_sceneIdCrc)
		return m_sceneIdCrc < rhs.m_sceneIdCrc;
	if (m_cell != rhs.m_cell)
		return m_cell < rhs.m_cell;
	return m_coordinates.x < rhs.m_coordinates.x;
}

//-----------------------------------------------------------------------

const char * const Location::getSceneNameByCrc(unsigned int crc)
{
	return s_planetCrcStringTable->lookUp(crc).getString();
}

//-----------------------------------------------------------------------

unsigned int Location::getCrcBySceneName(const char * const name)
{
	return s_planetCrcStringTable->lookUp(name).getCrc();
}

//-----------------------------------------------------------------------

unsigned int Location::getCrcBySceneName(const std::string & name)
{
	return getCrcBySceneName(name.c_str());
}

//-----------------------------------------------------------------------

const Vector & Location::getCoordinates() const
{
	return m_coordinates;
}

//-----------------------------------------------------------------------

const NetworkId & Location::getCell() const
{
	return m_cell;
}

//-----------------------------------------------------------------------

unsigned int Location::getSceneIdCrc() const
{
	return m_sceneIdCrc;
}

//-----------------------------------------------------------------------

const char * const Location::getSceneId() const
{
	return getSceneNameByCrc(m_sceneIdCrc);
}

//-----------------------------------------------------------------------

void Location::setCoordinates(const Vector & coordinates)
{
	m_coordinates = coordinates;
}

//-----------------------------------------------------------------------

void Location::setCell(const NetworkId & cell)
{
	m_cell = cell;
}

//-----------------------------------------------------------------------

void Location::setSceneIdCrc(unsigned int crc)
{
	m_sceneIdCrc = crc;
}

//-----------------------------------------------------------------------

void Location::setSceneId(const char * const name)
{
	m_sceneIdCrc = getCrcBySceneName(name);
}

//-----------------------------------------------------------------------

