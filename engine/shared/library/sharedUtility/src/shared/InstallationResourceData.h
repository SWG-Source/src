//======================================================================
//
// InstallationResourceData.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_InstallationResourceData_H
#define INCLUDED_InstallationResourceData_H

//======================================================================

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

//----------------------------------------------------------------------

/**
* InstallationResource contains information of interest to the client, concerning
* a single resource pool
*/

struct InstallationResourceData
{
	std::string     m_name;
	NetworkId       m_id;
	std::string     m_parentName;
	uint8           m_efficiency;

	InstallationResourceData (const std::string & name, const NetworkId & id, const std::string& parentName, uint8 efficiency);
	InstallationResourceData ();

	bool operator== (const InstallationResourceData & rhs) const;
	bool operator!= (const InstallationResourceData & rhs) const;
};

//----------------------------------------------------------------------

inline InstallationResourceData::InstallationResourceData (const std::string & name, const NetworkId & id, const std::string& parentName, uint8 efficiency) :
m_name (name),
m_id (id),
m_parentName(parentName),
m_efficiency (efficiency)
{
}

//----------------------------------------------------------------------

inline InstallationResourceData::InstallationResourceData () :
m_name (),
m_id (),
m_parentName(),
m_efficiency (0)
{
}

//----------------------------------------------------------------------

inline bool InstallationResourceData::operator== (const InstallationResourceData & rhs) const
{
	return m_id == rhs.m_id && m_name == rhs.m_name && m_efficiency == rhs.m_efficiency && m_parentName == rhs.m_parentName;
}

//----------------------------------------------------------------------

inline bool InstallationResourceData::operator!= (const InstallationResourceData & rhs) const
{
	return !(*this == rhs);
}

//======================================================================

#endif
