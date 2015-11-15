// ======================================================================
//
// FileManifest.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FileManifest_H
#define INCLUDED_FileManifest_H

#include<string>

// ======================================================================

/**
 * This class keeps information about what files have been opened via this client
 * in order to track what assets are necessary for this client to run on the
 * server it is connecting to. It will read in and dump out it's info to a file if
 * the config setting is set.
 */
class FileManifest
{
public:

	static void install();
	static void remove();

	static void addNewManifestEntry(const char *fileName, int fileSize);
	static void addStoredManifestEntry(const char *fileName, const char *sceneId, int fileSize);
	static void addToTransitionVector(const char *fileName, int fileSize);

	static void addTransitionElementsToManifest();
	static void clearAllTransitionElements();

	static void setSceneId(const char *newScene);
	static bool isValidScene(const char *);
	static bool isTransitionScene(const char *);

	static bool contains(const char *fileName);
	static bool contains(uint32 crc);

	static std::string getDatatableName();
	static bool shouldUpdateManifest();

private:

	/// disabled
	FileManifest();
	/// disabled
	FileManifest(const FileManifest &);
	/// disabled
	FileManifest &operator =(const FileManifest &);
};

// ======================================================================

#endif
