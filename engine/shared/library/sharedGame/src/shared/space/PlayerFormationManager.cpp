//======================================================================
//
// PlayerFormationManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/PlayerFormationManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>
#include <vector>

//======================================================================

namespace PlayerFormationManagerNamespace
{
	typedef std::vector<PlayerFormationManager::FormationLocation> FormationData;

	namespace FileListColumnNames
	{
		char const * const filename = "FILENAME";
	}

	namespace FormationTableColumnNames
	{
		char const * const x   = "X";
		char const * const y   = "Y";
		char const * const z   = "Z";
		char const * const x2D = "X2D";
		char const * const z2D = "Z2D";
	}

	char const * const cms_filelistTableName = "datatables/space/formation/filelist.iff";
	char const * const cms_formationTableNamePre = "datatables/space/formation/";
	char const * const cms_formationTableNamePost = ".iff";
	int const cms_maxGroupMembers = 20;

	std::map<std::string, FormationData> ms_formations;
	std::map<uint32, std::string> ms_crcToFormationName;
}

//======================================================================

struct PlayerFormationManager::FormationLocation
{
	bool operator==(FormationLocation const & rhs) const;
	bool operator!=(FormationLocation const & rhs) const;

	Vector m_3d;
	Vector2d m_2d;
};

using namespace PlayerFormationManagerNamespace;

//======================================================================

bool PlayerFormationManager::FormationLocation::operator==(PlayerFormationManager::FormationLocation const & rhs) const
{
	return (m_3d == rhs.m_3d) && (m_2d == rhs.m_2d);
}

bool PlayerFormationManager::FormationLocation::operator!=(PlayerFormationManager::FormationLocation const & rhs) const
{
	return !operator== (rhs);
}

//----------------------------------------------------------------------

void PlayerFormationManager::install()
{
	InstallTimer const installTimer("PlayerFormationManager::install");

	ExitChain::add(PlayerFormationManager::remove, "PlayerFormationManager::remove");
	loadTables();
}

//----------------------------------------------------------------------

void PlayerFormationManager::remove()
{
	clear();
}

//----------------------------------------------------------------------

void PlayerFormationManager::loadTables()
{
	clear();

	if(!TreeFile::exists(cms_filelistTableName))
	{
		DEBUG_FATAL(true, ("Datatable [%s] not found to build the formation file list", cms_filelistTableName));
		return;
	}

	DataTable const * const filelistTableName = DataTableManager::getTable(cms_filelistTableName, true);
	std::string formationFileName;
	if(filelistTableName)
	{
		for(int i = 0; i < filelistTableName->getNumRows(); ++i)
		{
			formationFileName = filelistTableName->getStringValue(FileListColumnNames::filename, i);
			loadFormationFile(formationFileName);
		}
	}
}

//----------------------------------------------------------------------

void PlayerFormationManager::clear()
{
	ms_formations.clear();
	ms_crcToFormationName.clear();
}

//----------------------------------------------------------------------

void PlayerFormationManager::loadFormationFile(std::string const & formationFileName)
{
	std::string const fullformationFilename = cms_formationTableNamePre + formationFileName + cms_formationTableNamePost;

	//ensure the full filename exists
	if(!TreeFile::exists(fullformationFilename.c_str()))
	{
		DEBUG_FATAL(true, ("Datatable [%s] not found to build the formation", fullformationFilename.c_str()));
		return;
	}

	DataTable const * const formationDataTable = DataTableManager::getTable(fullformationFilename, true);
	if(formationDataTable)
	{
		//ensure the table has the right number of rows
		if(formationDataTable->getNumRows() != cms_maxGroupMembers)
		{
			DEBUG_FATAL(true, ("Formation data table [%s] does not have the required number of slots [20] defined", fullformationFilename.c_str()));
			return;
		}

		PlayerFormationManager::FormationLocation formationLocation;
		FormationData formationData;
		for(int i = 0; i < formationDataTable->getNumRows(); ++i)
		{
			formationLocation.m_3d.x = formationDataTable->getFloatValue(FormationTableColumnNames::x, i);
			formationLocation.m_3d.y = formationDataTable->getFloatValue(FormationTableColumnNames::y, i);
			formationLocation.m_3d.z = formationDataTable->getFloatValue(FormationTableColumnNames::z, i);

			formationLocation.m_2d.x = formationDataTable->getFloatValue(FormationTableColumnNames::x2D, i);
			formationLocation.m_2d.y = formationDataTable->getFloatValue(FormationTableColumnNames::z2D, i);

			//ensure point wasn't already defined
			for(FormationData::const_iterator j = formationData.begin(); j != formationData.end(); ++j)
			{
				PlayerFormationManager::FormationLocation const & test = *j;
				if(test == formationLocation)
				{
					DEBUG_FATAL(true, ("When building formation data for [%s], 3d position [%.2f, %.2f, %.2f] and 2d position [%.2f, %.2f] defined twice, each position must be unique", fullformationFilename.c_str(), formationLocation.m_3d.x, formationLocation.m_3d.y, formationLocation.m_3d.z, formationLocation.m_2d.x, formationLocation.m_2d.y));
					return;
				}
			}

			formationData.push_back(formationLocation);
		}

		//ensure we could build all the slots (so we can always assume the vector is the right size)
		if(formationData.size() != static_cast<unsigned int>(cms_maxGroupMembers))
		{
			DEBUG_FATAL(true, ("When building formation data for [%s], could not build the required number of slots [%d]", fullformationFilename.c_str(), cms_maxGroupMembers));
			return;
		}

		//ensure this formation wasn't already loaded
		std::map<std::string, FormationData>::const_iterator k = ms_formations.find(formationFileName);
		if(k != ms_formations.end())
		{
			DEBUG_FATAL(true, ("Formation data table [%s], defined twice", formationFileName.c_str()));
			return;
		}

		ms_formations[formationFileName] = formationData;
		ms_crcToFormationName[Crc::normalizeAndCalculate(formationFileName.c_str())] = formationFileName;
	}
}

//----------------------------------------------------------------------

void PlayerFormationManager::getFormationNames(PlayerFormationManager::FormationNameList & formationNames)
{
	formationNames.reserve(ms_formations.size());

	std::map<std::string, FormationData>::const_iterator ii = ms_formations.begin();
	std::map<std::string, FormationData>::const_iterator iiEnd = ms_formations.end();

	for (; ii != iiEnd; ++ii)
	{
		formationNames.push_back(ii->first);
	}
}

//----------------------------------------------------------------------

bool PlayerFormationManager::isValidFormationName(std::string const & formationName)
{
	std::map<std::string, FormationData>::const_iterator const & i = ms_formations.find(formationName);
	return (i != ms_formations.end());
}

//----------------------------------------------------------------------

int PlayerFormationManager::getMaximumFormationGroupMembers()
{
	return cms_maxGroupMembers;
}

//----------------------------------------------------------------------

bool PlayerFormationManager::getPositionOffsetFormationLocation(uint32 const formationCrc, int const index, FormationLocation & formationLocation)
{
	if(index < 0 || index > cms_maxGroupMembers)
	{
		DEBUG_WARNING(true, ("getPositionOffsetFormationLocation, index is [%d], out of range of [0, %d]", index, cms_maxGroupMembers));
		return false;
	}

	std::map<uint32, std::string>::const_iterator const & it = ms_crcToFormationName.find(formationCrc);
	if(it != ms_crcToFormationName.end())
	{
		std::map<std::string, FormationData>::const_iterator const & it2 = ms_formations.find(it->second);
		if(it2 != ms_formations.end())
		{
			if(it2->second.size() <= static_cast<unsigned int>(index))
			{
				DEBUG_WARNING(true, ("getPositionOffsetFormationLocation, index is [%d], out of range of [0, %d)", index, it2->second.size()));
				return false;
			}
			formationLocation = it2->second[static_cast<unsigned int>(index)];
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

bool PlayerFormationManager::getPositionOffset(uint32 const formationCrc, int const index, Vector & /*OUT*/ offset)
{
	offset.makeZero();

	FormationLocation formationLocation;
	if (getPositionOffsetFormationLocation(formationCrc, index, formationLocation))
	{
		offset = formationLocation.m_3d;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool PlayerFormationManager::getPositionOffset(uint32 formationCrc, int index, Vector2d & /*OUT*/ offset)
{
	offset.makeZero();

	FormationLocation formationLocation;
	if (getPositionOffsetFormationLocation(formationCrc, index, formationLocation))
	{
		offset = formationLocation.m_2d;
		return true;
	}
	return false;
}


//======================================================================
