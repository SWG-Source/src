// ======================================================================
//
// FreeCtsDataTable.cpp
// Copyright 2008 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/FreeCtsDataTable.h"

#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

// ======================================================================

namespace FreeCtsDataTableNamespace
{
	std::multimap<std::string, FreeCtsDataTable::FreeCtsInfo> s_freeCtsList;

	bool s_dataLoaded = false;
	std::string s_freeCtsFileName;
	void loadData();

	time_t convertToEpoch(int year, int month, int day, int hour, int minute, int second);
}

using namespace FreeCtsDataTableNamespace;

// ======================================================================

void FreeCtsDataTableNamespace::loadData()
{
	if (s_dataLoaded)
		return;

	s_dataLoaded = true;

	// set up data to determine free CTS
	s_freeCtsFileName = ConfigFile::getKeyString("FreeCts", "freeCtsFilename", "");
	if (!s_freeCtsFileName.empty())
	{
		DataTable * table = DataTableManager::getTable(s_freeCtsFileName, true);
		if (table)
		{
			int const columnRuleName = table->findColumnNumber("ruleName");
			int const columnRuleDescription = table->findColumnNumber("ruleDescription");

			int const columnSourceClusterList = table->findColumnNumber("sourceClusterList");
			int const columnTargetClusterList = table->findColumnNumber("targetClusterList");

			int const columnStartDateYr = table->findColumnNumber("startDateYr");
			int const columnStartDateMo = table->findColumnNumber("startDateMo");
			int const columnStartDateDy = table->findColumnNumber("startDateDy");
			int const columnStartDateHr = table->findColumnNumber("startDateHr");
			int const columnStartDateMin = table->findColumnNumber("startDateMin");
			int const columnStartDateSec = table->findColumnNumber("startDateSec");

			int const columnEndDateYr = table->findColumnNumber("endDateYr");
			int const columnEndDateMo = table->findColumnNumber("endDateMo");
			int const columnEndDateDy = table->findColumnNumber("endDateDy");
			int const columnEndDateHr = table->findColumnNumber("endDateHr");
			int const columnEndDateMin = table->findColumnNumber("endDateMin");
			int const columnEndDateSec = table->findColumnNumber("endDateSec");

			int const columnCharacterCreateDateRangeLowerYr = table->findColumnNumber("characterCreateDateRangeLowerYr");
			int const columnCharacterCreateDateRangeLowerMo = table->findColumnNumber("characterCreateDateRangeLowerMo");
			int const columnCharacterCreateDateRangeLowerDy = table->findColumnNumber("characterCreateDateRangeLowerDy");
			int const columnCharacterCreateDateRangeLowerHr = table->findColumnNumber("characterCreateDateRangeLowerHr");
			int const columnCharacterCreateDateRangeLowerMin = table->findColumnNumber("characterCreateDateRangeLowerMin");
			int const columnCharacterCreateDateRangeLowerSec = table->findColumnNumber("characterCreateDateRangeLowerSec");

			int const columnCharacterCreateDateRangeUpperYr = table->findColumnNumber("characterCreateDateRangeUpperYr");
			int const columnCharacterCreateDateRangeUpperMo = table->findColumnNumber("characterCreateDateRangeUpperMo");
			int const columnCharacterCreateDateRangeUpperDy = table->findColumnNumber("characterCreateDateRangeUpperDy");
			int const columnCharacterCreateDateRangeUpperHr = table->findColumnNumber("characterCreateDateRangeUpperHr");
			int const columnCharacterCreateDateRangeUpperMin = table->findColumnNumber("characterCreateDateRangeUpperMin");
			int const columnCharacterCreateDateRangeUpperSec = table->findColumnNumber("characterCreateDateRangeUpperSec");

			FATAL((columnRuleName < 0), ("column \"ruleName\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnRuleDescription < 0), ("column \"ruleDescription\" not found in %s", s_freeCtsFileName.c_str()));

			FATAL((columnSourceClusterList < 0), ("column \"sourceClusterList\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnTargetClusterList < 0), ("column \"targetClusterList\" not found in %s", s_freeCtsFileName.c_str()));

			FATAL((columnStartDateYr < 0), ("column \"startDateYr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnStartDateMo < 0), ("column \"startDateMo\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnStartDateDy < 0), ("column \"startDateDy\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnStartDateHr < 0), ("column \"startDateHr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnStartDateMin < 0), ("column \"startDateMin\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnStartDateSec < 0), ("column \"startDateSec\" not found in %s", s_freeCtsFileName.c_str()));

			FATAL((columnEndDateYr < 0), ("column \"endDateYr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnEndDateMo < 0), ("column \"endDateMo\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnEndDateDy < 0), ("column \"endDateDy\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnEndDateHr < 0), ("column \"endDateHr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnEndDateMin < 0), ("column \"endDateMin\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnEndDateSec < 0), ("column \"endDateSec\" not found in %s", s_freeCtsFileName.c_str()));

			FATAL((columnCharacterCreateDateRangeLowerYr < 0), ("column \"characterCreateDateRangeLowerYr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeLowerMo < 0), ("column \"characterCreateDateRangeLowerMo\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeLowerDy < 0), ("column \"characterCreateDateRangeLowerDy\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeLowerHr < 0), ("column \"characterCreateDateRangeLowerHr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeLowerMin < 0), ("column \"characterCreateDateRangeLowerMin\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeLowerSec < 0), ("column \"characterCreateDateRangeLowerSec\" not found in %s", s_freeCtsFileName.c_str()));

			FATAL((columnCharacterCreateDateRangeUpperYr < 0), ("column \"characterCreateDateRangeUpperYr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeUpperMo < 0), ("column \"characterCreateDateRangeUpperMo\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeUpperDy < 0), ("column \"characterCreateDateRangeUpperDy\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeUpperHr < 0), ("column \"characterCreateDateRangeUpperHr\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeUpperMin < 0), ("column \"characterCreateDateRangeUpperMin\" not found in %s", s_freeCtsFileName.c_str()));
			FATAL((columnCharacterCreateDateRangeUpperSec < 0), ("column \"characterCreateDateRangeUpperSec\" not found in %s", s_freeCtsFileName.c_str()));

			std::string sourceClusterList, targetClusterList;
			int startDateYr, startDateMo, startDateDy, startDateHr, startDateMin, startDateSec;
			int endDateYr, endDateMo, endDateDy, endDateHr, endDateMin, endDateSec;
			int characterCreateDateRangeLowerYr, characterCreateDateRangeLowerMo, characterCreateDateRangeLowerDy, characterCreateDateRangeLowerHr, characterCreateDateRangeLowerMin, characterCreateDateRangeLowerSec;
			int characterCreateDateRangeUpperYr, characterCreateDateRangeUpperMo, characterCreateDateRangeUpperDy, characterCreateDateRangeUpperHr, characterCreateDateRangeUpperMin, characterCreateDateRangeUpperSec;

			FreeCtsDataTable::FreeCtsInfo freeCtsInfo;

			Unicode::UnicodeStringVector tokensSourceClusterList, tokensTargetClusterList;
			Unicode::UnicodeStringVector::const_iterator tokensIter;
			static Unicode::String const clusterListDelimiters(Unicode::narrowToWide(", "));

			int const numRows = table->getNumRows();
			for (int i = 0; i < numRows; ++i)
			{
				freeCtsInfo.ruleName = table->getStringValue(columnRuleName, i);
				freeCtsInfo.ruleDescription = table->getStringValue(columnRuleDescription, i);

				sourceClusterList = table->getStringValue(columnSourceClusterList, i);
				targetClusterList = table->getStringValue(columnTargetClusterList, i);

				startDateYr = table->getIntValue(columnStartDateYr, i);
				startDateMo = table->getIntValue(columnStartDateMo, i);
				startDateDy = table->getIntValue(columnStartDateDy, i);
				startDateHr = table->getIntValue(columnStartDateHr, i);
				startDateMin = table->getIntValue(columnStartDateMin, i);
				startDateSec = table->getIntValue(columnStartDateSec, i);

				endDateYr = table->getIntValue(columnEndDateYr, i);
				endDateMo = table->getIntValue(columnEndDateMo, i);
				endDateDy = table->getIntValue(columnEndDateDy, i);
				endDateHr = table->getIntValue(columnEndDateHr, i);
				endDateMin = table->getIntValue(columnEndDateMin, i);
				endDateSec = table->getIntValue(columnEndDateSec, i);

				characterCreateDateRangeLowerYr = table->getIntValue(columnCharacterCreateDateRangeLowerYr, i);
				characterCreateDateRangeLowerMo = table->getIntValue(columnCharacterCreateDateRangeLowerMo, i);
				characterCreateDateRangeLowerDy = table->getIntValue(columnCharacterCreateDateRangeLowerDy, i);
				characterCreateDateRangeLowerHr = table->getIntValue(columnCharacterCreateDateRangeLowerHr, i);
				characterCreateDateRangeLowerMin = table->getIntValue(columnCharacterCreateDateRangeLowerMin, i);
				characterCreateDateRangeLowerSec = table->getIntValue(columnCharacterCreateDateRangeLowerSec, i);

				characterCreateDateRangeUpperYr = table->getIntValue(columnCharacterCreateDateRangeUpperYr, i);
				characterCreateDateRangeUpperMo = table->getIntValue(columnCharacterCreateDateRangeUpperMo, i);
				characterCreateDateRangeUpperDy = table->getIntValue(columnCharacterCreateDateRangeUpperDy, i);
				characterCreateDateRangeUpperHr = table->getIntValue(columnCharacterCreateDateRangeUpperHr, i);
				characterCreateDateRangeUpperMin = table->getIntValue(columnCharacterCreateDateRangeUpperMin, i);
				characterCreateDateRangeUpperSec = table->getIntValue(columnCharacterCreateDateRangeUpperSec, i);

				freeCtsInfo.targetCluster.clear();
				freeCtsInfo.startTime = -1;
				freeCtsInfo.endTime = -1;
				freeCtsInfo.sourceCharacterCreateTimeLower = -1;
				freeCtsInfo.sourceCharacterCreateTimeUpper = -1;

				if ((startDateYr > 0) && (startDateMo > 0) && (startDateDy > 0))
				{
					freeCtsInfo.startTime = convertToEpoch(startDateYr, startDateMo, startDateDy, startDateHr, startDateMin, startDateSec);
					FATAL((freeCtsInfo.startTime <= 0), ("date %d/%d/%d %d:%d:%d on row %d is invalid", startDateMo, startDateDy, startDateYr, startDateHr, startDateMin, startDateSec, (i+3)));
				}

				if ((endDateYr > 0) && (endDateMo > 0) && (endDateDy > 0))
				{
					freeCtsInfo.endTime = convertToEpoch(endDateYr, endDateMo, endDateDy, endDateHr, endDateMin, endDateSec);
					FATAL((freeCtsInfo.endTime <= 0), ("date %d/%d/%d %d:%d:%d on row %d is invalid", endDateMo, endDateDy, endDateYr, endDateHr, endDateMin, endDateSec, (i+3)));
				}

				if ((freeCtsInfo.startTime > 0) && (freeCtsInfo.endTime > 0))
				{
					FATAL((freeCtsInfo.startTime >= freeCtsInfo.endTime), ("start date (%s) must be before end date (%s) on row %d", CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo.startTime).c_str(), CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo.endTime).c_str(), (i+3)));
				}

				if ((characterCreateDateRangeLowerYr > 0) && (characterCreateDateRangeLowerMo > 0) && (characterCreateDateRangeLowerDy > 0))
				{
					freeCtsInfo.sourceCharacterCreateTimeLower = convertToEpoch(characterCreateDateRangeLowerYr, characterCreateDateRangeLowerMo, characterCreateDateRangeLowerDy, characterCreateDateRangeLowerHr, characterCreateDateRangeLowerMin, characterCreateDateRangeLowerSec);
					FATAL((freeCtsInfo.sourceCharacterCreateTimeLower <= 0), ("date %d/%d/%d %d:%d:%d on row %d is invalid", characterCreateDateRangeLowerMo, characterCreateDateRangeLowerDy, characterCreateDateRangeLowerYr, characterCreateDateRangeLowerHr, characterCreateDateRangeLowerMin, characterCreateDateRangeLowerSec, (i+3)));
				}

				if ((characterCreateDateRangeUpperYr > 0) && (characterCreateDateRangeUpperMo > 0) && (characterCreateDateRangeUpperDy > 0))
				{
					freeCtsInfo.sourceCharacterCreateTimeUpper = convertToEpoch(characterCreateDateRangeUpperYr, characterCreateDateRangeUpperMo, characterCreateDateRangeUpperDy, characterCreateDateRangeUpperHr, characterCreateDateRangeUpperMin, characterCreateDateRangeUpperSec);
					FATAL((freeCtsInfo.sourceCharacterCreateTimeUpper <= 0), ("date %d/%d/%d %d:%d:%d on row %d is invalid", characterCreateDateRangeUpperMo, characterCreateDateRangeUpperDy, characterCreateDateRangeUpperYr, characterCreateDateRangeUpperHr, characterCreateDateRangeUpperMin, characterCreateDateRangeUpperSec, (i+3)));
				}

				if ((freeCtsInfo.sourceCharacterCreateTimeLower > 0) && (freeCtsInfo.sourceCharacterCreateTimeUpper > 0))
				{
					FATAL((freeCtsInfo.sourceCharacterCreateTimeLower >= freeCtsInfo.sourceCharacterCreateTimeUpper), ("lower date (%s) must be before upper date (%s) on row %d", CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo.sourceCharacterCreateTimeLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo.sourceCharacterCreateTimeUpper).c_str(), (i+3)));
				}

				tokensSourceClusterList.clear();
				tokensTargetClusterList.clear();
				if (Unicode::tokenize(Unicode::narrowToWide(sourceClusterList), tokensSourceClusterList, &clusterListDelimiters, nullptr) && (tokensSourceClusterList.size() > 0) && Unicode::tokenize(Unicode::narrowToWide(targetClusterList), tokensTargetClusterList, &clusterListDelimiters, nullptr) && (tokensTargetClusterList.size() > 0))
				{
					for (tokensIter = tokensTargetClusterList.begin(); tokensIter != tokensTargetClusterList.end(); ++tokensIter)
						freeCtsInfo.targetCluster[Unicode::wideToNarrow(Unicode::toLower(*tokensIter))] = Unicode::wideToNarrow(*tokensIter);

					for (tokensIter = tokensSourceClusterList.begin(); tokensIter != tokensSourceClusterList.end(); ++tokensIter)
						IGNORE_RETURN(s_freeCtsList.insert(std::make_pair(Unicode::wideToNarrow(Unicode::toLower(*tokensIter)), freeCtsInfo)));
				}
			}

			DataTableManager::close(s_freeCtsFileName);

#ifdef _DEBUG
			for (std::multimap<std::string, FreeCtsDataTable::FreeCtsInfo>::const_iterator iter = s_freeCtsList.begin(); iter != s_freeCtsList.end(); ++iter)
			{
				std::string targetClusters;
				for (std::map<std::string, std::string>::const_iterator iter2 = iter->second.targetCluster.begin(); iter2 != iter->second.targetCluster.end(); ++iter2)
				{
					if (!targetClusters.empty())
						targetClusters += ", ";

					targetClusters += iter2->first;
				}

				std::string startDate("NA");
				std::string endDate("NA");

				if (iter->second.startTime > 0)
					startDate = CalendarTime::convertEpochToTimeStringLocal(iter->second.startTime);

				if (iter->second.endTime > 0)
					endDate = CalendarTime::convertEpochToTimeStringLocal(iter->second.endTime);

				std::string characterCreateDateLower("NA");
				std::string characterCreateDateUpper("NA");

				if (iter->second.sourceCharacterCreateTimeLower > 0)
					characterCreateDateLower = CalendarTime::convertEpochToTimeStringLocal(iter->second.sourceCharacterCreateTimeLower);

				if (iter->second.sourceCharacterCreateTimeUpper > 0)
					characterCreateDateUpper = CalendarTime::convertEpochToTimeStringLocal(iter->second.sourceCharacterCreateTimeUpper);

				DEBUG_REPORT_LOG(true, ("FreeCtsInfo rule=(%s, %s), source cluster=(%s) target clusters=(%s) start/end=(%ld, %s) -> (%ld, %s) character create date=(%ld, %s) -> (%ld, %s)\n", iter->second.ruleName.c_str(), iter->second.ruleDescription.c_str(), iter->first.c_str(), targetClusters.c_str(), iter->second.startTime, startDate.c_str(), iter->second.endTime, endDate.c_str(), iter->second.sourceCharacterCreateTimeLower, characterCreateDateLower.c_str(), iter->second.sourceCharacterCreateTimeUpper, characterCreateDateUpper.c_str()));
			}
#endif
		}
	}
}

// ----------------------------------------------------------------------

time_t FreeCtsDataTableNamespace::convertToEpoch(int const year, int const month, int const day, int const hour, int const minute, int const second)
{
	time_t const timeNow = ::time(nullptr);
	struct tm * timeinfo = ::localtime(&timeNow);
	FATAL(!timeinfo, ("::localtime() returns nullptr"));

	// greater than zero if Daylight Saving Time is in effect,
	// zero if Daylight Saving Time is not in effect,
	// and less than zero if the information is not available.
	int const currentTimeIsDst = timeinfo->tm_isdst;

	timeinfo->tm_year = year - 1900;
	timeinfo->tm_mon = month - 1;
	timeinfo->tm_mday = day;
	timeinfo->tm_hour = hour;
	timeinfo->tm_min = minute;
	timeinfo->tm_sec = second;

	time_t convertedTime = ::mktime(timeinfo);

	if (convertedTime <= 0)
	{
		return convertedTime;
	}

	// if the converted time converted back to y/m/d h:m:s doesn't
	// match the y/m/d h:m:s that is passed in, it probably means
	// the passed in time is in a different standard/daylight period
	// than the current time, so try the conversion again with the
	// "opposite" standard/daylight period than the current time,
	// and it should be OK
	timeinfo = ::localtime(&convertedTime);
	FATAL(!timeinfo, ("::localtime() returns nullptr"));

	if ((timeinfo->tm_year != (year - 1900)) ||
		(timeinfo->tm_mon != (month - 1)) ||
		(timeinfo->tm_mday != day) ||
		(timeinfo->tm_hour != hour) ||
		(timeinfo->tm_min != minute) ||
		(timeinfo->tm_sec != second))
	{
		if (currentTimeIsDst >= 0)
		{
			timeinfo->tm_year = year - 1900;
			timeinfo->tm_mon = month - 1;
			timeinfo->tm_mday = day;
			timeinfo->tm_hour = hour;
			timeinfo->tm_min = minute;
			timeinfo->tm_sec = second;
			timeinfo->tm_isdst = ((currentTimeIsDst == 0) ? 1 : 0);

			convertedTime = ::mktime(timeinfo);
		}
	}

	return convertedTime;
}

// ----------------------------------------------------------------------

std::string const & FreeCtsDataTable::getFreeCtsFileName()
{
	if (!s_dataLoaded)
		loadData();

	return s_freeCtsFileName;
}

// ----------------------------------------------------------------------

std::multimap<std::string, FreeCtsDataTable::FreeCtsInfo> const & FreeCtsDataTable::getFreeCtsInfo()
{
	if (!s_dataLoaded)
		loadData();

	return s_freeCtsList;
}

// ----------------------------------------------------------------------

FreeCtsDataTable::FreeCtsInfo const * FreeCtsDataTable::isFreeCtsSourceCluster(std::string const & sourceCluster)
{
	if (!s_dataLoaded)
		loadData();

	if (s_freeCtsList.empty())
		return nullptr;

	time_t const timeNow = ::time(nullptr);
	std::pair<std::multimap<std::string, FreeCtsInfo>::const_iterator, std::multimap<std::string, FreeCtsInfo>::const_iterator> range = s_freeCtsList.equal_range(Unicode::toLower(sourceCluster));
	for (std::multimap<std::string, FreeCtsInfo>::const_iterator iter = range.first; iter != range.second; ++iter)
	{
		bool const satisfyStartTime = ((iter->second.startTime <= 0) || (timeNow >= iter->second.startTime));
		bool const satisfyEndTime = ((iter->second.endTime <= 0) || (timeNow <= iter->second.endTime));

		if (satisfyStartTime && satisfyEndTime)
			return &(iter->second);
	}

	return nullptr;
}

// ----------------------------------------------------------------------

FreeCtsDataTable::FreeCtsInfo const * FreeCtsDataTable::wouldCharacterTransferBeFree(time_t const sourceCharacterCreateTime, uint32 const sourceStationId, std::string const & sourceCluster, uint32 const targetStationId, std::string const & targetCluster, bool ignoreTimeRestriction)
{
	if (!s_dataLoaded)
		loadData();

	if (s_freeCtsList.empty())
		return nullptr;

	if ((sourceCharacterCreateTime <= 0) && !ignoreTimeRestriction)
		return nullptr;

	if (sourceStationId != targetStationId)
		return nullptr;

	time_t const timeNow = ::time(nullptr);

	std::string const lowerTargetCluster(Unicode::toLower(targetCluster));
	std::pair<std::multimap<std::string, FreeCtsInfo>::const_iterator, std::multimap<std::string, FreeCtsInfo>::const_iterator> range = s_freeCtsList.equal_range(Unicode::toLower(sourceCluster));
	for (std::multimap<std::string, FreeCtsInfo>::const_iterator iter = range.first; iter != range.second; ++iter)
	{
		if (iter->second.targetCluster.count(lowerTargetCluster) > 0)
		{
			if (ignoreTimeRestriction)
				return &(iter->second);

			bool const satisfyStartTime = ((iter->second.startTime <= 0) || (timeNow >= iter->second.startTime));
			bool const satisfyEndTime = ((iter->second.endTime <= 0) || (timeNow <= iter->second.endTime));

			bool const satisfyCreateTimeLowerBound = ((iter->second.sourceCharacterCreateTimeLower <= 0) || (sourceCharacterCreateTime >= iter->second.sourceCharacterCreateTimeLower));
			bool const satisfyCreateTimeUpperBound = ((iter->second.sourceCharacterCreateTimeUpper <= 0) || (sourceCharacterCreateTime <= iter->second.sourceCharacterCreateTimeUpper));

			if (satisfyStartTime && satisfyEndTime && satisfyCreateTimeLowerBound && satisfyCreateTimeUpperBound)
				return &(iter->second);
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

FreeCtsDataTable::FreeCtsInfo const * FreeCtsDataTable::getFreeCtsInfoForCharacter(time_t const sourceCharacterCreateTime, std::string const & sourceCluster, bool ignoreTimeRestriction)
{
	if (!s_dataLoaded)
		loadData();

	if (s_freeCtsList.empty())
		return nullptr;

	if ((sourceCharacterCreateTime <= 0) && !ignoreTimeRestriction)
		return nullptr;

	time_t const timeNow = ::time(nullptr);
	std::pair<std::multimap<std::string, FreeCtsInfo>::const_iterator, std::multimap<std::string, FreeCtsInfo>::const_iterator> range = s_freeCtsList.equal_range(Unicode::toLower(sourceCluster));
	for (std::multimap<std::string, FreeCtsInfo>::const_iterator iter = range.first; iter != range.second; ++iter)
	{
		if (ignoreTimeRestriction)
			return &(iter->second);

		bool const satisfyStartTime = ((iter->second.startTime <= 0) || (timeNow >= iter->second.startTime));
		bool const satisfyEndTime = ((iter->second.endTime <= 0) || (timeNow <= iter->second.endTime));

		bool const satisfyCreateTimeLowerBound = ((iter->second.sourceCharacterCreateTimeLower <= 0) || (sourceCharacterCreateTime >= iter->second.sourceCharacterCreateTimeLower));
		bool const satisfyCreateTimeUpperBound = ((iter->second.sourceCharacterCreateTimeUpper <= 0) || (sourceCharacterCreateTime <= iter->second.sourceCharacterCreateTimeUpper));

		if (satisfyStartTime && satisfyEndTime && satisfyCreateTimeLowerBound && satisfyCreateTimeUpperBound)
			return &(iter->second);
	}

	return nullptr;
}
