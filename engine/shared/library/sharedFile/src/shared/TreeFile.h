// ======================================================================
//
// TreeFile.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TreeFile_H
#define INCLUDED_TreeFile_H

// ======================================================================

class Compressor;

// @todo codereorg remove dependency on mutex
class Mutex;
#include "fileInterface/AbstractFile.h"

/*NOTE: TreeFile_SearchNode.h is included AFTER TreeFile is defined to allow
  the subclasses to be broken out into a separate file module without creating
	additional include dependencies on the modules that use TreeFile.
*/

// ======================================================================

/**
 * Provide access to files, whether in a treefile or on actual disk.
 *
 * This class is multi-thread safe for modifying the search order, opening
 * files, and closing files.  Applications should not attempt to read a
 * single opened file from multiple threads at the same time, but it is
 * acceptable to read from different files in different threads.
 */
class TreeFile
{
private:

	// The following are defined in a seperate file (TreeFile_SearchNode.*)
	class SearchNode;
	class SearchPath;
	class SearchAbsolute;
	class SearchTree;
	class SearchTOC;
	class SearchCache;

	friend class SearchNode;
	friend class TreeFileBuilder;
	friend class TreeFileBuilderHelper;
	friend class TreeFileExtractor;

public:

	class TreeFileFactory : public AbstractFileFactory
	{
		public:
			virtual AbstractFile* createFile(const char *filename, const char *open_type);
	};
	friend class TreeFileFactory;

public:

	static void install(uint32 skuBits);
	static void remove();

	static bool isLoggingFiles();

#ifdef _DEBUG
	static void setLogTreeFileOpens(bool logTreeFileOpens);
#endif

	static void debugReportPaths();
	static void debugReportMetrics();

	static void addSearchPath(const char *path, int priority);
	static void addSearchAbsolute(int priority);
	static void addSearchTree(const char *fileName, int priority);
	static void addSearchTOC(const char *fileName, int priority);
	static bool validateSearchTree(const char *fileName);
	static void removeAllSearches();

	static bool                    exists(const char *fileName);
	static int                     getFileSize(const char *fileName);
	static DLLEXPORT AbstractFile *open(const char *filename, AbstractFile::PriorityType, bool allowFail);

	static void          fixUpFileName(const char *fileName, char *outName);
	static bool          getPathName(const char *fileName, char *pathName, int pathNameLength);
	static const char   *getShortestExistingPath(const char *fileName);
	static bool          stripTreeFileSearchPathFromFile(const std::string &inputPath, std::string &outputPath);
	static bool          stripTreeFileSearchPathFromFile(const char *inputPath, char *outputPath, int outputPathBufferLength);


	static int           getNumberOfSearchPaths();
	static const char   *getSearchPath(int index);

	static void          addCachedFile(const char *fileName, AbstractFile *file);
	static void          clearCachedFiles();

	static int           getNumberOfFilesOpenedTotal();
	static int           getSizeOfFilesOpenedTotal();

	static int           cacheFile(char const * fileName);

private:

	typedef std::vector<SearchNode *>  SearchNodes;

private:

	/// disabled
	TreeFile();
	/// disabled
	TreeFile(const TreeFile &);
	/// disabled
	TreeFile &operator =(const TreeFile &);

	static void        addSearchCache(int priority);
	static bool        searchNodePriorityOrder(const SearchNode *a, const SearchNode *b);
	static void        addSearchNode(SearchNode *newNode);
	static SearchNode *find(const char *fileName);

	static void        fixUpFileName(char *output, const char *filename, bool warning);

private:

	static bool          ms_installed;
	static bool          ms_haveCachedFiles;
	static Mutex         ms_criticalSection;
	static SearchNodes   ms_searchNodes;
	static SearchCache * ms_searchCache;

	static bool          ms_debugReportFlagShowMetrics;
	static bool          ms_debugReportFlagShowSearchPaths;
	static bool          ms_debugLogFlag;
	static int           ms_numberOfFilesOpenedTotal;
	static int           ms_sizeOfFilesOpenedTotal;
	static int           ms_unexpectedCacheMisses;
};

// ======================================================================

#endif
