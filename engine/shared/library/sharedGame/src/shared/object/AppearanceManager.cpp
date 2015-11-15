// ======================================================================
// 
// AppearanceManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AppearanceManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

// ======================================================================

namespace AppearanceManagerNamespace
{
	typedef std::set<CrcString const *, LessPointerComparator> AppearanceTemplateNameSet;
	AppearanceTemplateNameSet ms_appearanceTemplateNameSet;

	typedef std::vector<CrcString const *> CrcStringVector;
	typedef std::map<CrcString const *, CrcStringVector *, LessPointerComparator> ObjectTemplateAppearanceTemplateMap;
	ObjectTemplateAppearanceTemplateMap ms_objectTemplateAppearanceTemplateMap;

	bool ms_installed;
	bool ms_verboseWarnings;

	void remove();

#ifdef _DEBUG
	void regressionTest();
#endif

	class PointerDeleterPair
	{
	public:

		template<typename FirstType, typename SecondType>
		void operator()(std::pair<FirstType, SecondType> &pairArgument) const
		{
			delete pairArgument.first;
			delete pairArgument.second;
		}
	};
}

using namespace AppearanceManagerNamespace;	

// ======================================================================

void AppearanceManager::install()
{
	InstallTimer const installTimer("AppearanceManager::install");

	DEBUG_FATAL(ms_installed, ("AppearanceManagerNamespace::install: already installed"));
	ms_installed = true;

	ms_verboseWarnings = ConfigFile::getKeyBool("SharedGame/AppearanceManager", "verboseWarnings", false);

	char const * const appearanceTableFileName = "datatables/appearance/appearance_table.iff";

	Iff iff;
	if (iff.open(appearanceTableFileName))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int const numberOfColumns = dataTable.getNumColumns();

		int const numberOfRows = dataTable.getNumRows();
		for (int row = 0; row < numberOfRows; ++row)
		{
			std::string const & sourceName = dataTable.getStringValue(0, row);
			TemporaryCrcString const crcSourceName(sourceName.c_str(), true);

			//-- Look up the source name
			CrcStringVector * crcStringVector = 0;
			{
				ObjectTemplateAppearanceTemplateMap::iterator iter = ms_objectTemplateAppearanceTemplateMap.find((const CrcString*)&crcSourceName);
				if (iter != ms_objectTemplateAppearanceTemplateMap.end())
				{
					DEBUG_WARNING(true, ("AppearanceManager::install(%s): duplicate entry found for %s", appearanceTableFileName, crcSourceName.getString()));
					continue;
				}
				else
				{
					crcStringVector = new CrcStringVector();
					crcStringVector->reserve(static_cast<size_t>(numberOfColumns));

					ms_objectTemplateAppearanceTemplateMap.insert(std::make_pair((const CrcString*)new PersistentCrcString(crcSourceName), crcStringVector));
				}
			}

			//-- Read in the column data
			for (int column = 1; column < numberOfColumns; ++column)
			{
				std::string const & fileName = dataTable.getStringValue(column, row);

#ifdef _DEBUG
				if (strstr(fileName.c_str(), ".sat") != 0 || strstr(fileName.c_str(), ".apt") != 0)
					DEBUG_WARNING(ms_verboseWarnings && !TreeFile::exists(fileName.c_str()), ("AppearanceManager::install(%s): [%s] is not a valid entry for row %d column %s because the file does not exist", appearanceTableFileName, fileName.c_str(), row, dataTable.getColumnName(column).c_str()));
				else
					DEBUG_WARNING(!(fileName.empty() || fileName == ":block" || fileName == ":default" || fileName == ":hide" ), ("AppearanceManager::install(%s): [%s] is not a valid entry for row %d column %s", appearanceTableFileName, fileName.c_str(), row, dataTable.getColumnName(column).c_str()));
#endif

				TemporaryCrcString const crcFileName(fileName.c_str(), true);
				
				CrcString const * appearanceTemplateName = 0;
				AppearanceTemplateNameSet::iterator iter = ms_appearanceTemplateNameSet.find(static_cast<CrcString const *>(&crcFileName));
				if (iter != ms_appearanceTemplateNameSet.end())
					appearanceTemplateName = *iter;
				else
				{
					appearanceTemplateName = new PersistentCrcString(crcFileName);

					ms_appearanceTemplateNameSet.insert(appearanceTemplateName);
				}

				crcStringVector->push_back(appearanceTemplateName);
			}
		}
	}

#ifdef _DEBUG
	regressionTest();
#endif

	ExitChain::add(AppearanceManagerNamespace::remove, "AppearanceManagerNamespace::remove");
}

// ----------------------------------------------------------------------

void AppearanceManagerNamespace::remove()
{
	DEBUG_FATAL(!ms_installed, ("AppearanceManagerNamespace::remove: not installed"));
	ms_installed = false;

	std::for_each(ms_appearanceTemplateNameSet.begin(), ms_appearanceTemplateNameSet.end(), PointerDeleter());
	ms_appearanceTemplateNameSet.clear();

	std::for_each(ms_objectTemplateAppearanceTemplateMap.begin(), ms_objectTemplateAppearanceTemplateMap.end(), PointerDeleterPair());
	ms_objectTemplateAppearanceTemplateMap.clear();
}

// ----------------------------------------------------------------------

//bool AppearanceManager::exists(CrcString const & sourceName)
bool AppearanceManager::isAppearanceManaged(std::string const &fileName)
{
	TemporaryCrcString const crcFileName(fileName.c_str(), true);
	return ms_objectTemplateAppearanceTemplateMap.find((const CrcString*)&crcFileName) != ms_objectTemplateAppearanceTemplateMap.end();
}

// ----------------------------------------------------------------------

//bool AppearanceManager::getAppearanceName(CrcString const & sourceName, int const column, CrcString const * & fileName)
bool AppearanceManager::getAppearanceName(std::string &targetName, std::string const &sourceName, int const sourceColumn)
{
	TemporaryCrcString const crcSourceName(sourceName.c_str(), true);

	//-- Look up the source name
	ObjectTemplateAppearanceTemplateMap::iterator iter = ms_objectTemplateAppearanceTemplateMap.find((const CrcString*)&crcSourceName);
	if (iter == ms_objectTemplateAppearanceTemplateMap.end())
		return false;

	//-- Source name exists, so the column must exist
	CrcStringVector const & crcStringVector = *iter->second;
	int const column = sourceColumn - 1;
	int const numberOfColumns = static_cast<int>(crcStringVector.size());
	if (column < 0 || column >= numberOfColumns)
	{
		DEBUG_FATAL(true, ("AppearanceManager::getAppearanceName(%s): invalid column %d/%d", sourceName.c_str(), column, numberOfColumns));
		return false;
	}

	targetName = crcStringVector[static_cast<size_t>(column)]->getString();

	return true;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void AppearanceManagerNamespace::regressionTest()
{
	DEBUG_FATAL(!AppearanceManager::isAppearanceManaged("shared_aakuan_belt"), ("AppearanceManagerNamespace::regressionTest: FAILED - shared_aakuan_belt not found"));

	std::string targetName;
	DEBUG_FATAL(!AppearanceManager::getAppearanceName(targetName, "shared_aakuan_belt", 1) || targetName != std::string("appearance/belt_s05_m.sat"), ("AppearanceManagerNamespace::regressionTest: shared_aakuan_belt Male Human appearance is supposed to be appearance/belt_s05_m.sat, but is %s", targetName.c_str()));
}

#endif

// ======================================================================
