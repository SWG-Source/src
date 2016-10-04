//===================================================================
//
// SetupSharedUtility.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/SetupSharedUtility.h"

#include "sharedFile/FileManifest.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/CachedFileManager.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/ConfigSharedUtility.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "sharedUtility/LocationManager.h"
#include "sharedUtility/PooledString.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"
#include "sharedDebug/InstallTimer.h"

//===================================================================

namespace
{
	bool ms_installed;
}

//===================================================================

SetupSharedUtility::Data::Data () :
	m_allowFileCaching(false)
{
}

//===================================================================

void SetupSharedUtility::install(SetupSharedUtility::Data const & data)
{
	InstallTimer const installTimer("SetupSharedUtility::install");

	DEBUG_FATAL (ms_installed, ("SetupSharedUtility::install already installed"));
	ms_installed = true;

	ConfigSharedUtility::install();

	CurrentUserOptionManager::install ();
	LocalMachineOptionManager::install ();
	DataTableManager::install ();
	WorldSnapshotReaderWriter::Node::install ();
	Callback::install ();
	CachedFileManager::install(data.m_allowFileCaching);
	PooledString::install ();
	LocationManager::install ();
	installFileManifestEntries ();

	ExitChain::add (SetupSharedUtility::remove, "SetupSharedUtility");
}

//-------------------------------------------------------------------

void SetupSharedUtility::remove ()
{
	DEBUG_FATAL (!ms_installed, ("SetupSharedUtility::remove not installed"));
	ms_installed = false;
}

//-------------------------------------------------------------------

void SetupSharedUtility::setupGameData(Data & data)
{
	data.m_allowFileCaching = false;
}

//-------------------------------------------------------------------

void SetupSharedUtility::setupToolData(Data & data)
{
	data.m_allowFileCaching = false;
}

//-------------------------------------------------------------------

void SetupSharedUtility::installFileManifestEntries ()
{
	// read in the datatable entries for sharedFile/FileManifest.cpp
	std::string datatableName = FileManifest::getDatatableName();

	FATAL(!TreeFile::exists(datatableName.c_str()), ("%s could not be found. Are your paths set up correctly?", datatableName.c_str()));

	DataTable * manifestDatatable = DataTableManager::getTable(datatableName, true);

	if (manifestDatatable)
	{
		int numRows = manifestDatatable->getNumRows();

		for (int i = 0; i < numRows; ++i)
		{
			std::string fileName = manifestDatatable->getStringValue("fileName", i);
			std::string sceneId = manifestDatatable->getStringValue("sceneId", i);
			int fileSize = manifestDatatable->getIntValue("fileSize", i);

			if (!fileName.empty())
				FileManifest::addStoredManifestEntry(fileName.c_str(), sceneId.c_str(), fileSize);
			else
				DEBUG_WARNING(true, ("SetupSharedUtility::installFileManifestEntries(): found an entry with a nullptr filename: (row %i)\n", i));
		}
	}
	else
		DEBUG_WARNING(true, ("SetupSharedUtility::installFileManifestEntries(): can't find %s\n", datatableName.c_str()));
	DataTableManager::close(datatableName);
}
//===================================================================
