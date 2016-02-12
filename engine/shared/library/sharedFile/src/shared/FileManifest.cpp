// ======================================================================
//
// FileManifest.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/FileManifest.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/FileNameUtils.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

namespace FileManifestNamespace
{
	struct FileManifestEntry
	{
		std::string    name;
		int            size;
		int            accesses;
		std::string    scene;
	};

	typedef std::map<const uint32, FileManifestEntry*>  ManifestMap;
	typedef std::pair<std::string, int>                               TransitionVectorEntry;
	typedef std::vector<TransitionVectorEntry>                        TransitionVector;

	static ManifestMap s_manifest;
	static TransitionVector s_transitionVector;
	static bool s_installed                               = false;
	static bool s_updateManifest                          = false;
	static int s_accessThreshold                          = -1;
	static std::string s_currentSceneId                   = "none"; 
	static bool s_isValidScene                            = false;
	static bool s_isTransitionScene                       = true;

	const static std::string s_manifestDataTable          = "datatables/manifest/skufree.iff";

	const static std::string s_validScenes []             = {"tutorial", "space_npe_falcon", "dungeon1:npe_shared_station", "dungeon1:npe_dungeon", "space_ord_mantell"};
	const static int s_numValidScenes                     = sizeof (s_validScenes) / sizeof (s_validScenes[0]);

	const static std::string s_transitionScenes []        = {"dungeon1", "none", "unknown"};
	const static int s_numTransitionScenes                = sizeof (s_transitionScenes) / sizeof (s_transitionScenes[0]);

	const int fileSizeBufferSize = 512;
	const int fileNameBufferSize = 512;
	const int sceneIdBufferSize  = 128;
}
using namespace FileManifestNamespace;

// ======================================================================

void FileManifest::install()
{
	DEBUG_FATAL(s_installed, ("FileManifest::install(): already installed"));
	s_installed = true;

	ExitChain::add(FileManifest::remove, "FileManifest::remove", 0, true);

#if PRODUCTION == 0
	// update the access threshold if one was specified
	s_accessThreshold = ConfigFile::getKeyInt("SharedFile", "fileManifestAccessThreshold", -1, s_accessThreshold);

	// if we are developing, and want to update the manifest, read in the tab file as well
	const char * manifestFile = ConfigFile::getKeyString("SharedFile", "updateFileManifest", 0, nullptr);
	if (manifestFile != nullptr)
	{
		s_updateManifest = true;

		if (FileNameUtils::isReadable(manifestFile))
		{
			// read in the table version of the manifest
			StdioFile inputFile(manifestFile,"r");
			DEBUG_FATAL(!inputFile.isOpen(), ("FileManifet::install(): Could not open %s.", manifestFile));

			int fileLength = inputFile.length();
			char* fileBuffer = new char[fileLength];
			memset(fileBuffer, 0, fileLength);

			int bytes_read = inputFile.read(fileBuffer, fileLength);
			memset(fileBuffer + bytes_read, 0, fileLength - bytes_read);

			char* currentPos = fileBuffer;
			char* delimeter;

			// skip past the two header lines of the datatable
			currentPos = strstr(currentPos, "\n");
			if (currentPos)
			{
				++currentPos;
				currentPos = strstr(currentPos, "\n");
			}
			if (currentPos)
				++currentPos;

			char* fileSizeBuffer = new char[fileSizeBufferSize];
			int fileSize = 0;
			char* fileNameBuffer = new char[fileNameBufferSize];
			char* sceneIdBuffer = new char[sceneIdBufferSize];

			// iterate through the fileBuffer
			while (currentPos && currentPos < fileBuffer + fileLength && strstr(currentPos, "\n"))
			{
				// clear out all values
				fileSize = 0;
				memset(fileSizeBuffer, 0, fileSizeBufferSize);
				memset(fileNameBuffer, 0, fileNameBufferSize);
				memset(sceneIdBuffer, 0, sceneIdBufferSize);

				// get filename
				delimeter = strstr(currentPos, "\t");
				if (!delimeter)
				{
					DEBUG_WARNING(true, ("FileManifest::install(): Couldn't find a tab at position %i in file manfiest table, stopped iterating on elements!\n", currentPos - fileBuffer));
					break;
				}
				int numCharacters = delimeter - currentPos;
				FATAL(numCharacters > fileNameBufferSize, ("Copying too much data!"));
				strncpy(fileNameBuffer, currentPos, numCharacters);
				currentPos = delimeter + 1;

				// get sceneId
				delimeter = strstr(currentPos, "\t");
				if (!delimeter)
				{
					DEBUG_WARNING(true, ("FileManifest::install(): Couldn't find a tab at position %i in file manfiest table, stopped iterating on elements!\n", currentPos - fileBuffer));
					break;
				}

				numCharacters = delimeter - currentPos;
				FATAL(numCharacters > sceneIdBufferSize, ("Copying too much data!"));
				strncpy(sceneIdBuffer, currentPos, numCharacters);
				currentPos = delimeter + 1;

				// get filesize
				delimeter = strstr(currentPos, "\n");
				if (!delimeter)
				{
					DEBUG_WARNING(true, ("FileManifest::install(): Couldn't find a newline at position %i in file manfiest table, stopped iterating on elements!\n", currentPos - fileBuffer));
					break;
				}
				numCharacters = delimeter - currentPos;
				FATAL(numCharacters > fileSizeBufferSize, ("Copying too much data!"));
				strncpy(fileSizeBuffer, currentPos, numCharacters);
				fileSize = atoi(fileSizeBuffer);
				currentPos = delimeter + 1;

				if (isValidScene(sceneIdBuffer))
					addStoredManifestEntry(fileNameBuffer, sceneIdBuffer, fileSize);
			}

			delete [] fileBuffer;
			delete [] fileSizeBuffer;
			delete [] fileNameBuffer;
			delete [] sceneIdBuffer;
			inputFile.close();
		}
		else
		{
			DEBUG_WARNING(true, ("FileManifest::install(): Couldn't access %s for reading", manifestFile));
		}
	}
#endif
}

// -----------------------------------------------------------------------

void FileManifest::remove()
{
	DEBUG_FATAL(!s_installed, ("FileManifest::remove(): not installed"));
	s_installed = false;

#if PRODUCTION == 0
	// dump out the new manifest if requested
	const char * manifestFile = ConfigFile::getKeyString("SharedFile", "updateFileManifest", 0, nullptr);
	if (manifestFile != nullptr)
	{
		StdioFile outputFile(manifestFile,"w");
		DEBUG_FATAL(!outputFile.isOpen(), ("FileManifest::remove(): Could not open %s for writing.", manifestFile));
		char * buffer = new char[1024];
		std::vector<std::string> manifestEntries;

		sprintf(buffer, "fileName\tsceneId\tfileSize\n");
		outputFile.write(strlen(buffer), buffer);
		sprintf(buffer, "s\ts\ti\n");
		outputFile.write(strlen(buffer), buffer);

		DEBUG_REPORT_LOG_PRINT(s_accessThreshold >= 0, ("FileManifestAccessReport:\tFile:\tScene:\tSize:\tAccesses:\n"));
		for (ManifestMap::iterator i = s_manifest.begin(); i != s_manifest.end(); ++i)
		{
			if (!i->second)
			{
				DEBUG_WARNING(true, ("FileManifest::remove(): Found a nullptr pointer in the fileManifest map!\n"));
				continue;
			}

			// output the entry in the file
			sprintf(buffer, "%s\t%s\t%i\n", ((i->second)->name).c_str(), ((i->second)->scene).c_str(), (i->second)->size);
			manifestEntries.push_back(buffer);

			// print and log the entry if it is in the access threshold
			DEBUG_REPORT_LOG_PRINT((i->second)->accesses <= s_accessThreshold, ("FileManifestAccessReport:\t%s\t%s\t%i\t%i\n", ((i->second)->name).c_str(), ((i->second)->scene).c_str(), (i->second)->size, (i->second)->accesses));

			// delete the actual entry
			delete i->second;
			i->second = 0;
		}

		std::sort(manifestEntries.begin(), manifestEntries.end());
		for (std::vector<std::string>::const_iterator j = manifestEntries.begin(); j != manifestEntries.end(); ++j)
			outputFile.write(j->length(), j->c_str());

		delete [] buffer;
		outputFile.close();

		return;
	}
#endif

	for (ManifestMap::iterator i = s_manifest.begin(); i != s_manifest.end(); ++i)
	{
		// delete the actual entry
		delete i->second;
		i->second = 0;
	}

	s_transitionVector.clear();
}

// -----------------------------------------------------------------------

void FileManifest::addNewManifestEntry(const char *fileName, int fileSize)
{
#if PRODUCTION == 0
	// check to see if the config option to update the manifest was set
	if (!s_updateManifest)
		return;

	// we don't know if the next scene will be valid, so we hang onto these elements just in case
	if (s_isTransitionScene)
		addToTransitionVector(fileName, fileSize);

	// if we are not in a valid scene, return
	if (!s_isValidScene)
		return;

	// throw away useless data
	if (!fileName || !strlen(fileName) || !strchr(fileName, '/'))
		return;

	const uint32 crc = Crc::calculate(fileName);
	FileManifestEntry * entry = new FileManifestEntry();
	entry->name     = fileName;
	entry->scene    = s_currentSceneId;
	entry->size     = fileSize;
	entry->accesses = 0;

	std::pair<ManifestMap::iterator, bool> insertReturn = s_manifest.insert(std::pair<const uint32, FileManifestEntry*>(crc, entry));

	// increment the accesses 
	++(((insertReturn.first)->second)->accesses);

	// if the insert failed
	if (!insertReturn.second)
	{
		// make sure the file is using the new fileSize - fileSize is 0 if this is entered due to a TreeFile::exists call
		if (fileSize)
			((insertReturn.first)->second)->size = fileSize;

	}
	delete entry;
#else
	return;
#endif
}

// -----------------------------------------------------------------------

void FileManifest::addStoredManifestEntry(const char *fileName, const char * sceneId, int fileSize)
{
	const uint32 crc = Crc::calculate(fileName);
	FileManifestEntry * entry = new FileManifestEntry();
	entry->name     = fileName;
	entry->scene    = sceneId;
	entry->size     = fileSize;
	entry->accesses = 0;

	s_manifest.insert(std::pair<const uint32, FileManifestEntry*>(crc, entry));

	delete entry;
}

// -----------------------------------------------------------------------

void FileManifest::addToTransitionVector(const char *fileName, int fileSize)
{
	TransitionVectorEntry entry(fileName, fileSize);
	s_transitionVector.push_back(entry);
}

// -----------------------------------------------------------------------

void FileManifest::addTransitionElementsToManifest()
{
	TransitionVector::const_iterator end = s_transitionVector.end();
	for (TransitionVector::const_iterator i = s_transitionVector.begin(); i != end; ++i)
		addNewManifestEntry((i->first).c_str(), i->second);
	clearAllTransitionElements();
}

// -----------------------------------------------------------------------

void FileManifest::clearAllTransitionElements()
{
	s_transitionVector.clear();
}

// -----------------------------------------------------------------------

void FileManifest::setSceneId(const char *newScene)
{
#if PRODUCTION == 0
	// check to see if the config option to update the manifest was set
	if (!s_updateManifest)
		return;

	// don't do anything if the scene hasn't changed
	if (s_currentSceneId.compare(newScene) == 0)
		return;

	if (newScene[0] == '\0')
		s_currentSceneId = "unknown";
	else
		s_currentSceneId = newScene;

	// update whether this scene is a valid one that we want to add entries into, or if this is a 
	// scene we want to wait on (we wait until we get a valid or invalid scene)
	// if we get an invalid scene, we throw away the entries we accumulated
	// if we get a valid scene, we add all the entries we accumulated
	// this functionality is necessary because of how the client gets information about its scene, and 
	// when it gets information about the buildout location it is in
	s_isValidScene = isValidScene(s_currentSceneId.c_str());
	s_isTransitionScene = isTransitionScene(s_currentSceneId.c_str());

	// on any scene transition, we either clear out or use all entries in the transition vector
	// unless we are going to another wait scene, in which case we keep accumulating
	if (s_isValidScene)
		addTransitionElementsToManifest();
	else if (!s_isTransitionScene)
		clearAllTransitionElements();
#else
	return;
#endif
}

// -----------------------------------------------------------------------

bool FileManifest::isValidScene(const char *scene)
{
	std::string sceneString(scene);
	for (int i = 0; i < s_numValidScenes; ++i)
	{
		// if the scene matches the name, return true
		if (sceneString.compare(s_validScenes[i]) == 0)
			return true;	
		//otherwise, if the scene name is <scene name>_<num>, we return true (multiple zones)
		else if (sceneString.find(s_validScenes[i] + "_", 0) == 0 && sceneString.length() == (s_validScenes[i].length() + 2))
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------

bool FileManifest::isTransitionScene(const char *scene)
{
	std::string sceneString(scene);
	for (int i = 0; i < s_numTransitionScenes; ++i)
	{
		// if the scene matches the name, return true
		if (sceneString.compare(s_transitionScenes[i]) == 0)
			return true;	
		//otherwise, if the scene name is <scene name>_<num>, we return true (multiple zones)
		else if (sceneString.find(s_transitionScenes[i] + "_", 0) == 0 && sceneString.length() == (s_transitionScenes[i].length() + 2))
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------

bool FileManifest::contains(const char *fileName)
{
	uint32 crc = Crc::calculate(fileName);
	return contains(crc);
}

// -----------------------------------------------------------------------

bool FileManifest::contains(uint32 crc)
{
	ManifestMap::iterator i = s_manifest.find(crc);

	if (i == s_manifest.end())
		return false;

	return true;
}

// -----------------------------------------------------------------------

std::string FileManifest::getDatatableName()
{
	return s_manifestDataTable;
}

// -----------------------------------------------------------------------

bool FileManifest::shouldUpdateManifest()
{
	return s_updateManifest;
}

// ======================================================================
