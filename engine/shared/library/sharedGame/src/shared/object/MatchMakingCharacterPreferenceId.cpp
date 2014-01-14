// ============================================================================
//
// MatchMakingCharacterPreferenceId.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/MatchMakingCharacterPreferenceId.h"

// ============================================================================
//
// Archive
//
// ============================================================================

//-----------------------------------------------------------------------------

void Archive::get(ReadIterator &source, MatchMakingCharacterPreferenceId &target)
{
	// Search Attribute
	get(source, target.getSearchAttribute());

	// Sub-string
	std::string subString;
	get(source, subString);
	target.setSubString(subString);
}

//-----------------------------------------------------------------------------

void Archive::put(ByteStream &target, const MatchMakingCharacterPreferenceId &source)
{
	// Search Attribute
	put(target, source.getSearchAttribute());

	// Sub-string
	put(target, source.getSubString());
}

// ============================================================================
//
// MatchMakingCharacterPreferenceId
//
// ============================================================================

//-----------------------------------------------------------------------------

MatchMakingCharacterPreferenceId::MatchMakingCharacterPreferenceId()
 : m_searchAttribute()
 , m_subString()
{
}

//-----------------------------------------------------------------------------

bool MatchMakingCharacterPreferenceId::operator ==(MatchMakingCharacterPreferenceId const &rhs) const
{
	return (m_searchAttribute == rhs.m_searchAttribute);
}

//-----------------------------------------------------------------------------

bool MatchMakingCharacterPreferenceId::operator !=(MatchMakingCharacterPreferenceId const &rhs) const
{
	return !(*this == rhs);
}

//-----------------------------------------------------------------------------

MatchMakingCharacterPreferenceId &MatchMakingCharacterPreferenceId::operator =(MatchMakingCharacterPreferenceId const &rhs)
{
	if (this != &rhs)
	{
		m_searchAttribute = rhs.getSearchAttribute();
		m_subString = rhs.getSubString();
	}

	return *this;
}

//-----------------------------------------------------------------------------

void MatchMakingCharacterPreferenceId::packString(std::string &value) const
{
	m_searchAttribute.getAsDbTextString(value);
	if (value.empty())
		value = "NONE";

	if (!m_subString.empty())
	{
		value += " ";
		value += m_subString;
	}
}

//-----------------------------------------------------------------------------

void MatchMakingCharacterPreferenceId::unPackString(std::string const &value)
{
	size_t const separator = value.find(' ');
	if (separator == std::string::npos)
	{
		m_subString.clear();

		if (value.empty() || (value == std::string("NONE")))
			m_searchAttribute.clear();
		else
			m_searchAttribute.setFromDbTextString(value.c_str());
	}
	else
	{
		std::string const ba = value.substr(0, separator);
		m_subString = value.substr(separator + 1);

		if (ba.empty() || (ba == std::string("NONE")))
			m_searchAttribute.clear();
		else
			m_searchAttribute.setFromDbTextString(ba.c_str());
	}
}

// ============================================================================
