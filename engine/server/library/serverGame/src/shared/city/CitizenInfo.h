// ======================================================================
//
// CitizenInfo.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _CitizenInfo_H_
#define _CitizenInfo_H_

// ======================================================================

#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class CitizenInfo
{
public:
	bool operator== (const CitizenInfo& rhs) const;
	bool operator!= (const CitizenInfo& rhs) const;

public:
	std::string m_citizenName;
	std::string m_citizenProfessionSkillTemplate;
	int m_citizenLevel;
	int m_citizenPermissions;
	BitArray m_citizenRank;
	std::string m_citizenTitle;
	NetworkId m_citizenAllegiance;
};

// ----------------------------------------------------------------------

namespace CitizenPermissions
{
	enum
	{
		Citizen = 0, // base citizen without any additional permission
		Militia = (1<<0),
		AbsentWeek1 = (1<<1),
		AbsentWeek2 = (1<<2),
		AbsentWeek3 = (1<<3),
		AbsentWeek4 = (1<<4),
		AbsentWeek5 = (1<<5),
		InactiveProtected = (1<<6), // protected from being removed as a citizen and structures marked packable due to inactivity
	};
}

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, CitizenInfo & target);
	void put(ByteStream & target, const CitizenInfo & source);
}

// ======================================================================

#endif // _CitizenInfo_H_
