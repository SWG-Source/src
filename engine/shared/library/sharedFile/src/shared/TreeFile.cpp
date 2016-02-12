// ======================================================================
//
// TreeFile.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFile/TreeFile_SearchNode.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PixCounter.h"
#include "sharedFile/ConfigSharedFile.h"
#include "sharedFile/FileManifest.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedSynchronization/Mutex.h"

#include <cstdlib>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <vector>
#include <map>

// ======================================================================

bool                   TreeFile::ms_installed;
bool                   TreeFile::ms_haveCachedFiles;
Mutex                  TreeFile::ms_criticalSection;
TreeFile::SearchNodes  TreeFile::ms_searchNodes;
TreeFile::SearchCache * TreeFile::ms_searchCache;

int                    TreeFile::ms_numberOfFilesOpenedTotal;
int                    TreeFile::ms_sizeOfFilesOpenedTotal;
#if PRODUCTION == 0
bool                   TreeFile::ms_debugReportFlagShowMetrics;
bool                   TreeFile::ms_debugReportFlagShowSearchPaths;
bool                   TreeFile::ms_debugLogFlag;
int                    TreeFile::ms_unexpectedCacheMisses;
#endif

namespace TreeFileNamespace
{
	static char const * const cms_priorityStrings[] =
	{
		"L",
		"M",
		"H"
	};

	class CachedFilesComparator
	{
	public:
		bool operator ()(const char *lhs, const char *rhs) const
		{
			return strcmp(lhs, rhs) < 0;
		}
	};

	typedef std::map<const char *, AbstractFile *, CachedFilesComparator> CachedFilesMap;
	static CachedFilesMap cachedFilesMap;

#if PRODUCTION == 0
	bool ms_debugLogSynchronousOnly;
	bool ms_warnTreeFileOpens;

	PixCounter::ResetInteger ms_numberOfFilesOpened[3];
	PixCounter::ResetInteger ms_sizeOfFilesOpened[3];
	PixCounter::ResetString  ms_treeFilesOpened;
#endif
}
using namespace TreeFileNamespace;

// ======================================================================
// Install the TreeFile system

void TreeFile::install(uint32 skuBits)
{
	DEBUG_FATAL(ms_installed, ("already installed"));
	ms_installed            = true;

#if PRODUCTION == 0
	ms_numberOfFilesOpened[0].bindToCounter("TreeFileLowOpenCount");
	ms_numberOfFilesOpened[1].bindToCounter("TreeFileMediumOpenCount");
	ms_numberOfFilesOpened[2].bindToCounter("TreeFileHighOpenCount");
	ms_sizeOfFilesOpened[0].bindToCounter("TreeFileLowOpenBytes");
	ms_sizeOfFilesOpened[1].bindToCounter("TreeFileMediumOpenBytes");
	ms_sizeOfFilesOpened[2].bindToCounter("TreeFileHighOpenBytes");
	ms_treeFilesOpened.bindToCounter("TreeFilesOpened");
#endif

	ExitChain::add(TreeFile::remove, "TreeFile::remove", 0, true);

	// the value 20 is used here for legacy support
	int const maxPriority = ConfigFile::getKeyInt("SharedFile", "maxSearchPriority", 20);

	// search all the specified skus
	bool first = true;
	for (uint32 sku = 0; first || skuBits; first = false, skuBits &= ~(1 << sku), ++sku)
	{
		// skip adding and verifying path, tree, and TOC for skus not flagged on the account
		if (!first && (skuBits & (1 << sku)) == 0)
			continue;

		// figure out what sku to be loading from.  handle 0 to mean no sku numbers are specificed (legacy support)
		char skuText[8] = { '\0' };
		if (skuBits)
			sprintf(skuText, "_%02d_", static_cast<int>(sku));

		// add all the search paths
		for (int priority = 0; priority <= maxPriority; ++priority)
		{
			// add all search paths
			{
				char buffer[32];
				sprintf(buffer, "searchPath%s%d", skuText, priority);

				char const * result;
				for (int index = 0; (result = ConfigFile::getKeyString("SharedFile", buffer, index, nullptr)) != nullptr; ++index)
					TreeFile::addSearchPath(result, priority);
			}

			// add all search trees
			{
				char buffer[32];
				sprintf(buffer, "searchTree%s%d", skuText, priority);

				char const * result;
				for (int index = 0; (result = ConfigFile::getKeyString("SharedFile", buffer, index, nullptr)) != nullptr; ++index)
					TreeFile::addSearchTree(result, priority);
			}

			// add in any TOCs
			{
				char buffer[32];
				sprintf(buffer, "searchTOC%s%d", skuText, priority);

				char const * result;
				for (int index = 0; (result = ConfigFile::getKeyString("SharedFile", buffer, index, nullptr)) != nullptr; ++index)
					TreeFile::addSearchTOC(result, priority);
			}
		}

		// add cached files
		{
			int const numberPreloads = ConfigSharedFile::getNumberOfTreeFilePreloads();
			for (int i = 0; i < numberPreloads; ++i)
			{
				char const * const path = ConfigSharedFile::getTreeFilePreload(i);
				if (path != 0)
				{
					AbstractFile * const file = open(path, AbstractFile::PriorityData, true);
					if (file)
					{
						TreeFile::addCachedFile(path, file);
						DEBUG_REPORT_LOG(true, ("preloaded %s\n", path));
					}
					else
					{
						DEBUG_REPORT_LOG(true, ("FAILED preloading %s\n", path));
					}
				}
			}
		}
	}

	//-- add an absolute search path after all search nodes
	{
		TreeFile::addSearchAbsolute(ConfigFile::getKeyInt("SharedFile", "searchAbsolute", 0, !ms_searchNodes.empty() ? ms_searchNodes.front()->getPriority() + 1 : 0));
	}

	//-- add search cache after all search nodes
	{
		TreeFile::addSearchCache(ConfigFile::getKeyInt("SharedFile", "searchCache", 0, !ms_searchNodes.empty() ? ms_searchNodes.front()->getPriority() + 1 : 0));
	}

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_debugReportFlagShowMetrics,     "SharedFile", "reportTreeFileMetrics",   debugReportMetrics);
	DebugFlags::registerFlag(ms_debugReportFlagShowSearchPaths, "SharedFile", "reportTreeFilePaths",     debugReportPaths);
	DebugFlags::registerFlag(ms_debugLogFlag,                   "SharedFile", "logTreeFileOpens");
	DebugFlags::registerFlag(ms_debugLogSynchronousOnly,        "SharedFile", "logTreeFileOpensSynchronousOnly");
	DebugFlags::registerFlag(ms_warnTreeFileOpens, "SharedFile", "warnTreeFileOpens");
#endif
}

// ----------------------------------------------------------------------
/**
 * Remove the TreeFile system.
 */

void TreeFile::remove(void)
{
	clearCachedFiles();

	ms_criticalSection.enter();

		DEBUG_FATAL(!ms_installed, ("not installed"));
		ms_installed = false;

		// remove all the search nodes
		const SearchNodes::iterator iEnd = ms_searchNodes.end();
		for (SearchNodes::iterator i = ms_searchNodes.begin(); i != iEnd; ++i)
			delete *i;
		ms_searchNodes.clear();

		ms_searchCache = 0;

	ms_criticalSection.leave();

#if PRODUCTION == 0
	DEBUG_WARNING(ms_unexpectedCacheMisses, ("%d unexpected asynchronous loader cache misses occurred", ms_unexpectedCacheMisses));

	DebugFlags::unregisterFlag(ms_debugReportFlagShowMetrics);
	DebugFlags::unregisterFlag(ms_debugReportFlagShowSearchPaths);
	DebugFlags::unregisterFlag(ms_debugLogFlag);
	DebugFlags::unregisterFlag(ms_debugLogSynchronousOnly);
	DebugFlags::unregisterFlag(ms_warnTreeFileOpens);
#endif
}

// ----------------------------------------------------------------------

bool TreeFile::isLoggingFiles()
{
#if PRODUCTION == 0
	return ms_debugLogFlag;
#else
	return false;
#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void TreeFile::setLogTreeFileOpens(bool const logTreeFileOpens)
{
	ms_debugLogFlag = logTreeFileOpens;
}
#endif

// ----------------------------------------------------------------------

#if PRODUCTION == 0

void TreeFile::debugReportMetrics()
{
	REPORT_LOG_PRINT(true, ("TreeFile: %d,%d,%d=opened %d,%d,%d=bytes %d=oTotal %d=bTotal %d=cacheMiss\n", ms_numberOfFilesOpened[0].getLastFrameValue(), ms_numberOfFilesOpened[1].getLastFrameValue(), ms_numberOfFilesOpened[2].getLastFrameValue(), ms_sizeOfFilesOpened[0].getLastFrameValue(), ms_sizeOfFilesOpened[1].getLastFrameValue(), ms_sizeOfFilesOpened[2].getLastFrameValue(), ms_numberOfFilesOpenedTotal, ms_sizeOfFilesOpenedTotal, ms_unexpectedCacheMisses));
}

#endif

// ----------------------------------------------------------------------

#if PRODUCTION == 0

void TreeFile::debugReportPaths()
{
	ms_criticalSection.enter();

		DEBUG_REPORT_PRINT(true, ("TreeFile search paths\n"));
		DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME("Foundation\\Treefile");
		DEBUG_OUTPUT_STATIC_VIEW("Foundation\\Treefile", ("TreeFile search paths\n"));

		// Print information about the search paths
		const SearchNodes::iterator iEnd = ms_searchNodes.end();
		for (SearchNodes::iterator i = ms_searchNodes.begin(); i != iEnd; ++i)
			(*i)->debugPrint();

		DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME("Foundation\\Treefile");

	ms_criticalSection.leave();
}

#endif

// ----------------------------------------------------------------------

bool TreeFile::searchNodePriorityOrder(const SearchNode *a, const SearchNode *b)
{
	return a->getPriority() > b->getPriority();
}

// ----------------------------------------------------------------------
/**
 * Add a SearchNode to the node list.
 *
 * The node list is prioritized from highest to lowest priority.  New
 * nodes with the same priority as old nodes will be inserted after the
 * last priority match.
 */

void TreeFile::addSearchNode(SearchNode *newNode)
{
	NOT_NULL(newNode);
	ms_criticalSection.enter();

		SearchNodes::iterator insertionPoint = std::lower_bound(ms_searchNodes.begin(), ms_searchNodes.end(), newNode, searchNodePriorityOrder);
		IGNORE_RETURN(ms_searchNodes.insert(insertionPoint, newNode));

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------
/**
 * Add a SearchAbsolute to the node list.
 *
 * This type of search entry will attempt to try to load the file from
 * the specified path, by prefixing the path before every file name.  The
 * path may be absolute or relative.
 *
 * @param path  Path to the search directory to add
 * @param priority  Priority for this search entry
 */

void TreeFile::addSearchPath(const char *path, int priority)
{
	addSearchNode(new SearchPath(priority, path));
}

// ----------------------------------------------------------------------
/**
 * Add a SearchAbsolute to the node list.
 *
 * This type of search entry will attempt to try to load the file exactly
 * as named from the disk subsystem, allowing absolute or relative file
 * name paths.
 *
 * @param priority  Search priority for the absolute path
 */

void TreeFile::addSearchAbsolute(int priority)
{
	addSearchNode(new SearchAbsolute(priority));
}

// ----------------------------------------------------------------------

void TreeFile::addSearchCache(int priority)
{
	IS_NULL(ms_searchCache);
	ms_searchCache = new SearchCache(priority);
	addSearchNode(ms_searchCache);
}

// ----------------------------------------------------------------------
/**
 * Add a SearchTree to the node list.
 *
 * @param fileName  File name of the search tree
 * @param priority  Search priority for the search tree
 */

void TreeFile::addSearchTree(const char *fileName, int priority)
{
	if (FileStreamer::exists (fileName))
		addSearchNode(new SearchTree(priority, fileName));
	else
	{
#ifdef _DEBUG
		DEBUG_FATAL(true, ("TreeFile::addSearchTree - [%s] not found", fileName));
#else
		WARNING(true, ("TreeFile::addSearchTree - [%s] not found", fileName));
#endif
	}
}

// ----------------------------------------------------------------------
/**
 * Add a SearchTOC to the node list.
 *
 * @param fileName  File name of the search tree
 */

void TreeFile::addSearchTOC(const char *fileName, int priority)
{
	if (FileStreamer::exists (fileName))
		addSearchNode(new SearchTOC(priority, fileName));
	else
	{
#ifdef _DEBUG
		DEBUG_FATAL(true, ("TreeFile::addSearchTOC - [%s] not found", fileName));
#else
		WARNING(true, ("TreeFile::addSearchTOC - [%s] not found", fileName));
#endif
	}
}

// ----------------------------------------------------------------------
/**
 * Validate a SearchTree.
 *
 * @param fileName  File name of the search tree to validate
 */

bool TreeFile::validateSearchTree(const char *fileName)
{
	return SearchTree::validate(fileName);
}

// ----------------------------------------------------------------------
/**
 * Remove all the search settings.
 *
 * This routine will remove all the search trees, paths, and absolute
 * settings from the TreeFile class.
 */

void TreeFile::removeAllSearches(void)
{
	ms_criticalSection.enter();

		// remove all the search nodes
		const SearchNodes::iterator iEnd = ms_searchNodes.end();
		for (SearchNodes::iterator i = ms_searchNodes.begin(); i != iEnd; ++i)
			delete *i;
		ms_searchNodes.clear();

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------
/**
 * Find the node (if any) the requested file is in.
 *
 * @return Pointer to the highest priority node containing the file.  If the
 * file is not found, nullptr is returned.  If checking for filename collisions is
 * set on, then this function DEBUG_FATALS if multiple files match
 */

TreeFile::SearchNode *TreeFile::find(const char *fileName)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (!fileName)
	{
		DEBUG_WARNING(true, ("TreeFile::find() Cannot find a nullptr filename"));
		return nullptr;
	}

	if (fileName[0] == '\0')
	{
		DEBUG_WARNING(true, ("TreeFile::find() Cannot find an empty filename"));
		return nullptr;
	}

	// search the list of nodes looking to see if the specified file exists
	bool deleted = false;
	const SearchNodes::iterator iEnd = ms_searchNodes.end();
	for (SearchNodes::iterator i = ms_searchNodes.begin(); !deleted && i != iEnd; ++i)
		if ((*i)->exists(fileName, deleted))
			return *i;

	return nullptr;
}

// ----------------------------------------------------------------------
/**
 * Check if a specified file can be found.
 *
 * @param fileName  File name to check for existance
 * @return True if the file exists, otherwise false
 */

bool TreeFile::exists(const char *fileName)
{
	char fixedFileName[Os::MAX_PATH_LENGTH];
	fixUpFileName(fixedFileName, fileName, true);

	DEBUG_FATAL(!ms_installed, ("TreeFile::exists not installed"));

#if PRODUCTION == 0
	FileManifest::addNewManifestEntry(fixedFileName, 0);
#endif

	return (find(fixedFileName) != nullptr);
}

// ----------------------------------------------------------------------

int TreeFile::getFileSize(const char *fileName)
{
	char fixedFileName[Os::MAX_PATH_LENGTH];
	fixUpFileName(fixedFileName, fileName, true);

	// search the list of nodes looking to see if the specified file exists
	bool deleted = false;
	const SearchNodes::iterator iEnd = ms_searchNodes.end();
	for (SearchNodes::iterator i = ms_searchNodes.begin(); !deleted && i != iEnd; ++i)
	{
		int size = (*i)->getFileSize(fixedFileName, deleted);
		if (size >= 0)
			return size;
	}

	return -1;
}

// ----------------------------------------------------------------------
/**
 * This function assumes the output buffer is large enough.
 * The output path will be the same length or shorter than the input path.
 */

void TreeFile::fixUpFileName(char *output, const char *fileName, bool warning)
{
	bool        currentIsSlash  = false;
	bool        previousIsSlash = false;

	UNREF(warning);

#ifdef _DEBUG
	// since we only emit a warning the first time we see the malformed file name, if we invert the warning-desired flag
	// it will behave exactly as we want
	warning = !warning;
#endif

	// skip this warnings for paths
	const char *f = fileName;

	// skip leading "\" or "/"
	while (f[0] == '\\' || f[0] == '/')
	{
#ifdef _DEBUG
		if (!warning)
		{
			WARNING(true, ("Malformed file name: %s'", fileName));
			warning = true;
		}
#endif

		++f;
	}

	// skip leading ".\" or "./"
	while (f[0] == '.' && (f[1] == '\\' || f[1] == '/'))
	{
#ifdef _DEBUG
		if (!warning)
		{
			WARNING(true, ("Malformed file name: %s'", fileName));
			warning = true;
		}
#endif

		f += 2;
	}

	// skip leading "..\" or "../"
	while (f[0] == '.' && f[1] == '.' && (f[2] == '\\' || f[2] == '/'))
	{
#ifdef _DEBUG
		if (!warning)
		{
			WARNING(true, ("Malformed file name: %s'", fileName));
			warning = true;
		}
#endif

		f += 3;
	}

	for (; *f; ++f)
	{
		// convert all backslashes to forward slashes, and lowercase all the characters
		const char c = *f;
		if (c == '\\' || c == '/')
		{
			currentIsSlash = true;
			*output = '/';
		}
		else
		{
			currentIsSlash = false;
			*output = static_cast<char>(tolower(c));
		}

		// strip repeated forward slashes
		if (!currentIsSlash || !previousIsSlash)
		{
			++output;
			previousIsSlash = currentIsSlash;
		}
#ifdef _DEBUG
		else
			if (!warning)
			{
				WARNING(true, ("Malformed file name: %s'", fileName));
				warning = true;
			}
#endif
	}

	*output = '\0';
}

// ----------------------------------------------------------------------
/**
 * Fix up the file name.
 *
 * This function will lowercase the string, change all slashes to forward slashes, remove duplicate slashes,
 * and more.  This function will not strip off full paths that are part of the search directories.
 *
 * The output buffer must be at least as large as the input buffer.  The output buffer may be the same as
 * the input buffer, however warning messages may be garbled in that case.
 */

void TreeFile::fixUpFileName(const char *fileName, char *outName)
{
	fixUpFileName(outName, fileName, false);
}

// ----------------------------------------------------------------------
/**
 * Find the node (if any) the requested file is in.
 *
 * @return Pointer to the highest priority node containing the file.  If the
 * file is not found, nullptr is returned.
 */

bool TreeFile::getPathName(const char *fileName, char *pathName, int pathNameLength)
{
	NOT_NULL(fileName);
	NOT_NULL(pathName);
	DEBUG_FATAL(pathNameLength < 1, ("invalid pathNameLength %d", pathNameLength));
	DEBUG_FATAL(!ms_installed, ("not installed"));

	// search the list of nodes looking to see if the specified file exists
	SearchNode *node = find(fileName);
	if (node)
	{
		node->getPathName(fileName, pathName, pathNameLength);
		return true;
	}

	*pathName = '\0';
	return false;
}

// ----------------------------------------------------------------------
/**
 * Open a file.
 *
 * The handle of the file is copied to the handlePointer.  The handle
 * is -1 on failure.
 *
 * If the allowFail parameter is false, this routine will call Fatal
 * if the file could not be opened
 *
 * @param fileName  File name to open
 * @param allowFail  True to return false on failure, otherwise Fatal
 * @return pointer to a newly allocated AbstractFile-derived class (caller must delete), nullptr if failure.
 */

AbstractFile* TreeFile::open(const char *fileName, AbstractFile::PriorityType priority, bool allowFail)
{
	AbstractFile *file = nullptr;

	char fixedFileName[Os::MAX_PATH_LENGTH];
	fixUpFileName(fixedFileName, fileName, true);

	// check the cache to see if the file has been preloaded
	ms_criticalSection.enter();
		const bool haveCachedFiles = ms_haveCachedFiles;
	ms_criticalSection.leave();

	if (haveCachedFiles)
	{
		ms_criticalSection.enter();

			CachedFilesMap::iterator cachedIterator = cachedFilesMap.find(fixedFileName);
			if (cachedIterator != cachedFilesMap.end())
			{
				file = cachedIterator->second;
				cachedIterator->second = nullptr;
			}

		ms_criticalSection.leave();

#if PRODUCTION == 0
		if (!file && Os::isMainThread())
		{
			DEBUG_REPORT_LOG(true, ("unexpected cache miss %d %s\n", ms_unexpectedCacheMisses, fixedFileName));
			++ms_unexpectedCacheMisses;
		}
#endif

		if (file)
		{
#if PRODUCTION == 0
			REPORT_LOG(ms_debugLogFlag && !ms_debugLogSynchronousOnly, ("TF::open(%s) %s @ [cached]\n", cms_priorityStrings[priority], fixedFileName));
			if (PixCounter::connectedToPixProfiler())
				ms_treeFilesOpened.append("C\t%d\t%s\n", file->length(), fixedFileName);

			FileManifest::addNewManifestEntry(fixedFileName, file->length());
#endif
			return file;
		}
	}

#if PRODUCTION == 0
	bool first = true;
#endif

	bool deleted = false;
	const SearchNodes::iterator iEnd = ms_searchNodes.end();
	for (SearchNodes::iterator i = ms_searchNodes.begin(); !file && !deleted && i != iEnd; ++i)
	{
		file = (*i)->open(fixedFileName, priority, deleted);

#if PRODUCTION == 0
		if (file)
		{
			if (PixCounter::connectedToPixProfiler())
				ms_treeFilesOpened.append("%s\t%d\t%s\n", first ? "F" : cms_priorityStrings[priority], file->length(), fixedFileName);

			if (ms_debugLogFlag || ms_warnTreeFileOpens)
			{
				char buffer[Os::MAX_PATH_LENGTH];
				(*i)->getPathName(fixedFileName, buffer, sizeof(buffer));

				if (ms_warnTreeFileOpens)
					WARNING(true, ("TF::open(%s) %s @ %s, [size=%d]\n", cms_priorityStrings[priority], fixedFileName, buffer, file->length()));
				else
				{
					REPORT_LOG(!ms_debugLogSynchronousOnly || (ms_debugLogSynchronousOnly && (priority == AbstractFile::PriorityData) && (*i != ms_searchCache)), ("TF::open(%s) %s @ %s, [size=%d]\n", cms_priorityStrings[priority], fixedFileName, buffer, file->length()));
					DEBUG_OUTPUT_CHANNEL("Foundation\\Treefile", ("TF::open %s -- %s\n", fixedFileName, buffer));
				}
			}
		}

		first = false;
#endif
	}

	if (!file)
	{
		FATAL(!allowFail, ("open '%s' not found", fixedFileName));

#if PRODUCTION == 0
		FileManifest::addNewManifestEntry(fixedFileName, 0);
#endif

		return nullptr;
	}

	const int length = file->length();
	++ms_numberOfFilesOpenedTotal;
	ms_sizeOfFilesOpenedTotal += length;

#if PRODUCTION == 0
	++ms_numberOfFilesOpened[priority];
	ms_sizeOfFilesOpened[priority] += length;

	FileManifest::addNewManifestEntry(fixedFileName, length);
#endif

	return file;
}

// ----------------------------------------------------------------------

int TreeFile::getNumberOfSearchPaths(void)
{
	int count = 0;

	const SearchNodes::iterator iEnd = ms_searchNodes.end();
	for (SearchNodes::iterator i = ms_searchNodes.begin(); i != iEnd; ++i)
		if (dynamic_cast<const SearchPath*>(*i) != 0)
			count++;

	return count;
}

// ----------------------------------------------------------------------

const char *TreeFile::getSearchPath(int index)
{
	int count = 0;
	const SearchNodes::iterator iEnd = ms_searchNodes.end();
	for (SearchNodes::iterator i = ms_searchNodes.begin(); i != iEnd; ++i)
	{
		const SearchPath *searchPath = dynamic_cast<const SearchPath*>(*i);
		if (searchPath != nullptr)
		{
			if (count == index)
				return searchPath->getPathName();
			count++;
		}
	}

	return nullptr;
}

//-----------------------------------------------------------------
/**
 * This function will return the shortest trailing path of its input that
 * can be loaded by the TreeFile.  If no files can be loaded, this routine
 * will return nullptr.
 */

const char *TreeFile::getShortestExistingPath(const char *path)
{
	NOT_NULL(path);

	if (!*path)
		return nullptr;

	const char *result = nullptr;
	do
	{
		// check if this one exists
		if (exists(path))
			result = path;

		// advance past the next path separator
		while (*path && *path != '\\' && *path != '/')
			++path;
		if (*path)
			++path;

	} while (*path);

	return result;
}

// ----------------------------------------------------------------------

static bool pathcompare(const char *lhs, const char *rhs)
{
	for ( ; *lhs && *rhs; ++lhs, ++rhs)
	{
		char l = static_cast<char>(tolower(*lhs));
		if (l == '\\')
			l = '/';

		char r = static_cast<char>(tolower(*rhs));
		if (r == '\\')
			r = '/';

		if (l != r)
			return false;
	}

	return *lhs == '\0' && *rhs == '/';
}

// ----------------------------------------------------------------------

bool TreeFile::stripTreeFileSearchPathFromFile(const char *inputPath, char *outputPath, int outputPathBufferSize)
{
	// convert the input to an absolute patah
	char buffer[Os::MAX_PATH_LENGTH];
	if (!Os::getAbsolutePath(inputPath, buffer, sizeof(buffer)))
		return false;

	// scan all the search nodes
	const SearchNodes::iterator iEnd = ms_searchNodes.end();
	for (SearchNodes::iterator i = ms_searchNodes.begin(); i != iEnd; ++i)
	{
		// make sure it's a relative search path
		const SearchPath *searchPath = dynamic_cast<const SearchPath*>(*i);
		if (searchPath != nullptr)
		{
			// check to see if the the search path is a prefix for the requested path
			const char *path = searchPath->getPathName();

			if (pathcompare(path, buffer))
			{
				// make sure the output buffer has enough room
				int const pathLength = static_cast<int>(strlen(path));
				int const outputLength = static_cast<int>(strlen(buffer)) - pathLength;
				if (outputLength > outputPathBufferSize)
					return false;

				// return the buffer
				strcpy(outputPath, buffer + pathLength + 1);
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool TreeFile::stripTreeFileSearchPathFromFile(const std::string &inputPath, std::string &outputPath)
{
	char buffer[Os::MAX_PATH_LENGTH];

	if (!stripTreeFileSearchPathFromFile(inputPath.c_str(), buffer, sizeof(buffer)))
		return false;

	outputPath = buffer;
	return true;
}

// ----------------------------------------------------------------------

void TreeFile::addCachedFile(const char *fileName, AbstractFile *file)
{
	ms_criticalSection.enter();

		const bool result = cachedFilesMap.insert(CachedFilesMap::value_type(fileName, file)).second;
		UNREF(result);
		DEBUG_FATAL(!result, ("item was already present"));
		ms_haveCachedFiles = true;

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void TreeFile::clearCachedFiles()
{
	ms_criticalSection.enter();

		// delete any remaining files
		CachedFilesMap::iterator iEnd = cachedFilesMap.end();
		for (CachedFilesMap::iterator i = cachedFilesMap.begin(); i != iEnd; ++i)
			if (i->second)
			{
				DEBUG_REPORT_LOG(true, ("Unused cached tree file %s(%d)\n", i->first, i->second->length()));
				delete i->second;
			}

		cachedFilesMap.clear();
		ms_haveCachedFiles = false;

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

int TreeFile::getNumberOfFilesOpenedTotal()
{
	return ms_numberOfFilesOpenedTotal;
}

// ----------------------------------------------------------------------

int TreeFile::getSizeOfFilesOpenedTotal()
{
	return ms_sizeOfFilesOpenedTotal;
}

// ----------------------------------------------------------------------

int TreeFile::cacheFile(char const * const fileName)
{
	NOT_NULL(ms_searchCache);
	return ms_searchCache->addCachedFile(fileName);
}

// ======================================================================

AbstractFile* TreeFile::TreeFileFactory::createFile(const char *filename, const char *open_type)
{
	UNREF(open_type);
	return open(filename, AbstractFile::PriorityData, true);
}

// ======================================================================

