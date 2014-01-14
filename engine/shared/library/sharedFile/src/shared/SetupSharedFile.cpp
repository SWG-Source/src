// ======================================================================
//
// SetupSharedFile.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/SetupSharedFile.h"

#include "sharedFile/ConfigSharedFile.h"
#include "sharedFile/FileManifest.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFile/FileStreamerFile.h"
#include "sharedFile/Iff.h"
#include "sharedFile/MemoryFile.h"
#include "sharedFile/OsFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFile/ZlibFile.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

void SetupSharedFile::install(bool useFileStreamer, uint32 skuBits)
{
	InstallTimer const installTimer("SetupSharedFile::install");

	ConfigSharedFile::install();
	FileStreamerFile::install();
	FileStreamer::install(useFileStreamer);
	OsFile::install();
	TreeFile::install(skuBits);
	FileManifest::install();
	MemoryFile::install();
	ZlibFile::install();
	Iff::install();
}

// ======================================================================
