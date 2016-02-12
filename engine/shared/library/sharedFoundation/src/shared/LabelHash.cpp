// ======================================================================
//
// LabelHash.cpp
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/LabelHash.h"

#include <map>
#include <string>

#include "sharedFoundation/Crc.h"

typedef std::map< uint32, std::string >	THashToStringMap;

typedef THashToStringMap::value_type THashEntry;

typedef std::map< std::string, THashToStringMap >	TDomainMap;

typedef TDomainMap::value_type TDomainEntry;

TDomainMap	gs_allDomains;

void LabelHash::addLabel ( char const * const domain, char const * const label )
{
	uint32 actualHash = Crc::calculate(label);

	checkLabel(domain,label,actualHash);
}

uint32	LabelHash::hashLabel ( char const * const domain, char const * const label )
{
	uint32 actualHash = Crc::calculate(label);

	checkLabel(domain,label,actualHash);

	return actualHash;
}

uint32	LabelHash::checkLabel( char const * const domain, char const * const label, uint32 hashValue )
{
	// First things first - make sure the label actually hashes to the given value

	uint32 actualHash = Crc::calculate(label);

	FATAL(actualHash != hashValue, ("LabelHash::checkLabel - Label [%s] hashes to [%d] and not [%d]\n",label,actualHash,hashValue));

	// ----------

	std::string domainString(domain);
	std::string labelString(label);

	// Find the hash-to-label map for the given domain, or 
	// create a new one if there isn't one already.
	
	TDomainMap::iterator domainIt = gs_allDomains.find(domainString);

	if(domainIt == gs_allDomains.end())
	{
		// Domain not found, create entry for it containing the label

		THashToStringMap newDomainMap;

		newDomainMap.insert( THashEntry(hashValue,labelString) );

		gs_allDomains.insert( TDomainEntry(domainString,newDomainMap) );
	}
	else
	{
		// Domain found, check for existing hash value

		THashToStringMap & domainLabels = (*domainIt).second;

		THashToStringMap::iterator labelIt = domainLabels.find(hashValue);

		if(labelIt == domainLabels.end())
		{
			// Hash value not found, add it to the domain

			domainLabels.insert( THashEntry(hashValue,labelString) );
		}
		else
		{
			// Hash value found, fatal if the string mapped to it doesn't match 
			// the given string

			std::string & currentLabel = (*labelIt).second;

			FATAL( currentLabel != labelString, ("LabelHash::checkLabel - Found a hash collision for domain [%s], label [%s] - two labels hashed to the same value\n",domain,label));
		}
	}

	// All is good, hash is OK.

	return hashValue;
};


const char * LabelHash::findLabel( char const * const domain, uint32 hashValue )
{
	std::string domainString(domain);

	TDomainMap::iterator domainIt = gs_allDomains.find(domainString);

	if(domainIt == gs_allDomains.end())
	{
		// Domain not found

		return nullptr;
	}

	// ----------
	// Domain found, check for existing hash value

	THashToStringMap & domainLabels = (*domainIt).second;

	THashToStringMap::iterator hashIt = domainLabels.find(hashValue);

	if(hashIt != domainLabels.end())
	{
		return (*hashIt).second.c_str();
	}

	// ----------
	// String not found

	return nullptr;
}

