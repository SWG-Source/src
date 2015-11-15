// ======================================================================
//
// CityStringParser.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CityStringParser.h"

#include "serverGame/PgcRatingInfo.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"

// ======================================================================

static void skipField(std::string const &source, unsigned int &pos)
{
	while (source[pos] && source[pos] != ':')
		++pos;
	if (source[pos] == ':')
		++pos;
}

// ----------------------------------------------------------------------

static void nextInt(std::string const &source, unsigned int &pos, int &ret)
{
	ret = atoi(source.c_str()+pos);
	skipField(source, pos);
}

// ----------------------------------------------------------------------

static void nextBool(std::string const &source, unsigned int &pos, bool &ret)
{
	ret = atoi(source.c_str()+pos) ? true : false;
	skipField(source, pos);
}

// ----------------------------------------------------------------------

static void nextFloat(std::string const &source, unsigned int &pos, float &ret)
{
	ret = static_cast<float>(atof(source.c_str()+pos));
	skipField(source, pos);
}

// ----------------------------------------------------------------------

static void nextString(std::string const &source, unsigned int &pos, std::string &ret)
{
	unsigned int oldPos = pos;
	while (source[pos] && source[pos] != ':')
		++pos;
	ret = source.substr(oldPos, pos-oldPos);
	if (source[pos] == ':')
		++pos;
}

// ----------------------------------------------------------------------

static void nextOid(std::string const &source, unsigned int &pos, NetworkId &ret)
{
	unsigned int oldPos = pos;
	skipField(source, pos);
	ret = NetworkId(source.substr(oldPos, pos-oldPos));
}

// ----------------------------------------------------------------------

static void nextVector(std::string const &source, unsigned int &pos, Vector &ret)
{
	nextFloat(source, pos, ret.x);
	nextFloat(source, pos, ret.y);
	nextFloat(source, pos, ret.z);
}

// ======================================================================

bool CityStringParser::parseCityInfo(std::string const &source, int &cityId, std::string &cityName, NetworkId &cityHallId, std::string &cityPlanet, int &x, int &z, int &radius, int &creationTime, NetworkId &leaderId, int &incomeTax, int &propertyTax, int &salesTax, Vector &travelLoc, int &travelCost, bool &travelInterplanetary, Vector &cloneLoc, Vector &cloneRespawn, NetworkId &cloneRespawnCell, NetworkId &cloneId)
{
	// need to parse different versions
	if (source.find("v2:") == 0) // version 2
	{
		unsigned int pos = 3;
		nextInt(source, pos, cityId);
		nextString(source, pos, cityName);
		nextOid(source, pos, cityHallId);
		nextString(source, pos, cityPlanet);
		nextInt(source, pos, x);
		nextInt(source, pos, z);
		nextInt(source, pos, radius);
		nextInt(source, pos, creationTime);
		nextOid(source, pos, leaderId);
		nextInt(source, pos, incomeTax);
		nextInt(source, pos, propertyTax);
		nextInt(source, pos, salesTax);
		nextVector(source, pos, travelLoc);
		nextInt(source, pos, travelCost);
		nextBool(source, pos, travelInterplanetary);
		nextVector(source, pos, cloneLoc);
		nextVector(source, pos, cloneRespawn);
		nextOid(source, pos, cloneRespawnCell);
		nextOid(source, pos, cloneId);
	}
	else // version 1
	{
		unsigned int pos = 0;
		nextInt(source, pos, cityId);
		nextString(source, pos, cityName);
		nextOid(source, pos, cityHallId);
		nextString(source, pos, cityPlanet);
		nextInt(source, pos, x);
		nextInt(source, pos, z);
		nextInt(source, pos, radius);
		creationTime = 0;
		nextOid(source, pos, leaderId);
		nextInt(source, pos, incomeTax);
		nextInt(source, pos, propertyTax);
		nextInt(source, pos, salesTax);
		nextVector(source, pos, travelLoc);
		nextInt(source, pos, travelCost);
		nextBool(source, pos, travelInterplanetary);
		nextVector(source, pos, cloneLoc);
		nextVector(source, pos, cloneRespawn);
		nextOid(source, pos, cloneRespawnCell);
		nextOid(source, pos, cloneId);
	}

	bool const ok = cityId && !cityName.empty() && !cityPlanet.empty() && cityHallId != NetworkId::cms_invalid;
	WARNING(!ok, ("CityStringParser::parseCityInfo: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool CityStringParser::parseCitizenInfo(std::string const &source, int &cityId, NetworkId &citizenId, std::string &citizenName, std::string &citizenProfessionSkillTemplate, int &citizenLevel, int &citizenPermissions, BitArray &citizenRank, std::string &citizenTitle, NetworkId &citizenAllegiance)
{
	// need to parse different versions
	if (source.find("v3:") == 0) // version 3 is to store pgc rating info
	{
		return false;
	}

	if (source.find("v2:") == 0) // version 2
	{
		unsigned int pos = 3;
		nextInt(source, pos, cityId);
		nextOid(source, pos, citizenId);
		nextString(source, pos, citizenName);
		nextString(source, pos, citizenProfessionSkillTemplate);
		nextInt(source, pos, citizenLevel);
		nextString(source, pos, citizenTitle);
		nextOid(source, pos, citizenAllegiance);
		nextInt(source, pos, citizenPermissions);

		std::string rankStr;
		nextString(source, pos, rankStr);
		citizenRank.setFromDbTextString(rankStr.c_str());
	}
	else // version 1
	{
		unsigned int pos = 0;
		nextInt(source, pos, cityId);
		nextOid(source, pos, citizenId);
		nextString(source, pos, citizenName);
		citizenProfessionSkillTemplate.clear();
		citizenLevel = 0;
		citizenTitle.clear();
		nextOid(source, pos, citizenAllegiance);
		nextInt(source, pos, citizenPermissions);
		citizenRank.clear();
	}

	bool const ok = cityId && citizenId != NetworkId::cms_invalid && !citizenName.empty();
	WARNING(!ok, ("CityStringParser::parseCitizenInfo: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool CityStringParser::parseCityStructureInfo(std::string const &source, int &cityId, NetworkId &structureId, int &structureType, bool &structureValid)
{
	unsigned int pos = 0;
	nextInt(source, pos, cityId);
	nextOid(source, pos, structureId);
	nextInt(source, pos, structureType);
	nextBool(source, pos, structureValid);
	bool ok = cityId && structureId != NetworkId::cms_invalid;
	WARNING(!ok, ("CityStringParser::parseCityStructureInfo: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool CityStringParser::parsePgcRatingInfo(std::string const &source, NetworkId &chroniclerId, PgcRatingInfo &pgcRatingInfo)
{
	if (source.find("v3:") == 0) // version 3 is to store pgc rating info
	{
		unsigned int pos = 3;
		nextOid(source, pos, chroniclerId);
		nextString(source, pos, pgcRatingInfo.m_chroniclerName);

		NetworkId temp;

		nextOid(source, pos, temp);
		pgcRatingInfo.m_ratingCount = temp.getValue();

		nextOid(source, pos, temp);
		pgcRatingInfo.m_ratingTotal = temp.getValue();

		nextInt(source, pos, pgcRatingInfo.m_lastRatingTime);

		for (size_t i = 0, max = pgcRatingInfo.m_data.size(); ((i < PgcRatingInfo::ms_PgcRatingDataSize) && (i < max)); ++i)
		{
			nextInt(source, pos, pgcRatingInfo.m_data[i]);
		}

		bool const ok = chroniclerId != NetworkId::cms_invalid && !pgcRatingInfo.m_chroniclerName.empty();
		WARNING(!ok, ("CityStringParser::parsePgcRatingInfo: bad string [%s]", source.c_str()));
		return ok;
	}

	return false;
}

// ----------------------------------------------------------------------

void CityStringParser::buildCitySpec(int cityId, std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, int creationTime, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId, std::string &result)
{
	FATAL(cityName.length() > 25, ("CityStringParser::buildNameSpec: bad city name"));

	// DB column to store information is VARCHAR2(500)
	char buffer[501]; // 500 chars + terminating \0

	// only write out newer version if there is actually new version information
	if (creationTime > 0) // version 2
	{
		snprintf(buffer, sizeof(buffer), "v2:%d:%s:%s:%s:%d:%d:%d:%d:%s:%d:%d:%d:%.10g:%.10g:%.10g:%d:%d:%.10g:%.10g:%.10g:%.10g:%.10g:%.10g:%s:%s",
			cityId,
			cityName.c_str(),
			cityHallId.getValueString().c_str(),
			cityPlanet.c_str(),
			x, z, radius,
			creationTime,
			leaderId.getValueString().c_str(),
			incomeTax, propertyTax, salesTax,
			static_cast<double>(travelLoc.x), static_cast<double>(travelLoc.y), static_cast<double>(travelLoc.z),
			travelCost,
			travelInterplanetary ? 1 : 0,
			static_cast<double>(cloneLoc.x), static_cast<double>(cloneLoc.y), static_cast<double>(cloneLoc.z),
			static_cast<double>(cloneRespawn.x), static_cast<double>(cloneRespawn.y), static_cast<double>(cloneRespawn.z),
			cloneRespawnCell.getValueString().c_str(),
			cloneId.getValueString().c_str());
	}
	else // version 1
	{
		snprintf(buffer, sizeof(buffer), "%d:%s:%s:%s:%d:%d:%d:%s:%d:%d:%d:%.10g:%.10g:%.10g:%d:%d:%.10g:%.10g:%.10g:%.10g:%.10g:%.10g:%s:%s",
			cityId,
			cityName.c_str(),
			cityHallId.getValueString().c_str(),
			cityPlanet.c_str(),
			x, z, radius,
			leaderId.getValueString().c_str(),
			incomeTax, propertyTax, salesTax,
			static_cast<double>(travelLoc.x), static_cast<double>(travelLoc.y), static_cast<double>(travelLoc.z),
			travelCost,
			travelInterplanetary ? 1 : 0,
			static_cast<double>(cloneLoc.x), static_cast<double>(cloneLoc.y), static_cast<double>(cloneLoc.z),
			static_cast<double>(cloneRespawn.x), static_cast<double>(cloneRespawn.y), static_cast<double>(cloneRespawn.z),
			cloneRespawnCell.getValueString().c_str(),
			cloneId.getValueString().c_str());
	}

	// absolutely, positively make sure that there is no more than 500
	// characters in buffer (not including the terminating \0) or the DB
	// will ***CRASH*** because the column is defined as VARCHAR2(500)
	buffer[500] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void CityStringParser::buildCitizenSpec(int const cityId, NetworkId const &citizenId, std::string const &citizenName, std::string const &citizenProfessionSkillTemplate, int const citizenLevel, int const citizenPermissions, BitArray const &citizenRank, std::string const &citizenTitle, NetworkId const &citizenAllegiance, std::string &result)
{
	FATAL(citizenTitle.length() > 25, ("CityStringParser::buildMemberSpec: bad title"));

	std::string rankStr;
	citizenRank.getAsDbTextString(rankStr);

	// DB column to store information is VARCHAR2(500)
	char buffer[501]; // 500 chars + terminating \0

	// only write out newer version if there is actually new version information
	if ((citizenLevel > 0) || !rankStr.empty() || !citizenProfessionSkillTemplate.empty() || !citizenTitle.empty()) // version 2
	{
		snprintf(buffer, sizeof(buffer), "v2:%d:%s:%s:%s:%d:%s:%s:%d:%s",
			cityId,
			citizenId.getValueString().c_str(),
			citizenName.c_str(),
			citizenProfessionSkillTemplate.c_str(),
			citizenLevel,
			citizenTitle.c_str(),
			citizenAllegiance.getValueString().c_str(),
			citizenPermissions,
			rankStr.c_str());
	}
	else // version 1
	{
		snprintf(buffer, sizeof(buffer), "%d:%s:%s:%s:%d",
			cityId,
			citizenId.getValueString().c_str(),
			citizenName.c_str(),
			citizenAllegiance.getValueString().c_str(),
			citizenPermissions);
	}

	// absolutely, positively make sure that there is no more than 500
	// characters in buffer (not including the terminating \0) or the DB
	// will ***CRASH*** because the column is defined as VARCHAR2(500)
	buffer[500] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void CityStringParser::buildCityStructureSpec(int cityId, NetworkId const &structureId, int structureType, bool structureValid, std::string &result)
{
	char buffer[256];
	// max field lengths - 11:21:11:1
	snprintf(buffer, sizeof(buffer)-1, "%d:%s:%d:%d",
		cityId,
		structureId.getValueString().c_str(),
		structureType,
		structureValid ? 1 : 0);
	buffer[sizeof(buffer)-1] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void CityStringParser::buildPgcRatingSpec(NetworkId const &chroniclerId, PgcRatingInfo const &pgcRatingInfo, std::string &result)
{
	// DB column to store information is VARCHAR2(500)
	char buffer[501]; // 500 chars + terminating \0

	std::string pgcData;
	for (size_t i = 0, max = pgcRatingInfo.m_data.size(); ((i < PgcRatingInfo::ms_PgcRatingDataSize) && (i < max)); ++i)
	{
		if (!pgcData.empty())
			pgcData += ":";

		snprintf(buffer, sizeof(buffer), "%d", pgcRatingInfo.m_data[i]);
		pgcData += buffer;
	}

	snprintf(buffer, sizeof(buffer), "v3:%s:%s:%s:%s:%d:%s",
		chroniclerId.getValueString().c_str(),
		pgcRatingInfo.m_chroniclerName.c_str(),
		NetworkId(pgcRatingInfo.m_ratingCount).getValueString().c_str(),
		NetworkId(pgcRatingInfo.m_ratingTotal).getValueString().c_str(),
		pgcRatingInfo.m_lastRatingTime,
		pgcData.c_str());

	// absolutely, positively make sure that there is no more than 500
	// characters in buffer (not including the terminating \0) or the DB
	// will ***CRASH*** because the column is defined as VARCHAR2(500)
	buffer[500] = '\0';
	result = buffer;
}

// ======================================================================

