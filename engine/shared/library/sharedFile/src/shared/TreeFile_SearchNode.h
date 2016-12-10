// ======================================================================
//
// TreeFile_SearchNode.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TreeFile_SearchNode_H
#define INCLUDED_TreeFile_SearchNode_H

// ======================================================================

class CrcString;
class MemoryBlockManager;

#include "sharedFile/TreeFile.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Tag.h"

// ======================================================================

/* This module defines the inner classes of Treefile, to help keep TreeFile.h clean and easy to read.
   Since all of these classes are private, only TreeFile.h/cpp needs to even know these files exist.
*/

class TreeFile::SearchNode
{
public:

	explicit SearchNode(int priority);
	virtual ~SearchNode();

	int                   getPriority() const;

	virtual void          debugPrint() = 0;
	virtual bool          exists(const char *fileName, bool &deleted) const = 0;
	virtual int           getFileSize(const char *fileName, bool &deleted) const = 0;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const = 0;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted) = 0;

private:

	SearchNode();
	SearchNode(const SearchNode &);
	SearchNode &operator =(const SearchNode &);

private:

	const int m_priority;
};

// ======================================================================

inline int TreeFile::SearchNode::getPriority() const
{
	return m_priority;
}

// ======================================================================

class TreeFile::SearchPath : public TreeFile::SearchNode
{
public:

	SearchPath(int priority, const char *path);
	virtual ~SearchPath();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

	const char           *getPathName() const; //lint !e1411  // Warning -- member with different signature hides virtual member (bug in PC-Lint, incorrect warning)

private:

	SearchPath();
	SearchPath(const SearchPath &);
	SearchPath &operator =(const SearchPath &);

	void makeAbsolutePath(const char *fileName, char *buffer) const;

private:

	char  *m_pathName;
	int    m_pathNameLength;
};

// ======================================================================

inline const char *TreeFile::SearchPath::getPathName() const
{
	return m_pathName;
}

// ======================================================================

class TreeFile::SearchAbsolute : public TreeFile::SearchNode
{
public:

	explicit SearchAbsolute(int priority);
	virtual ~SearchAbsolute();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

private:

	SearchAbsolute();
	SearchAbsolute(const SearchAbsolute &);
	SearchAbsolute &operator =(const SearchAbsolute &);
};

// ======================================================================

class TreeFile::SearchTree : public TreeFile::SearchNode
{
	// these friend classes are from the treefile tool
	friend class TOC;
	friend class FileEntry;
	friend class TreeFileBuilder;
	friend class TreeFileExtractor;

public:

	static bool        validate(const char *fileName);
	static bool        isCompressed(int compressorIndex);

public:

	SearchTree(int priority, const char *fileName);
	virtual ~SearchTree();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

private:

	// disabled
	SearchTree();

	// disabled
	SearchTree(const SearchTree &);

	// disabled
	SearchTree &operator =(const SearchTree &);

private:

	bool localExists(const char *fileName, int *index, bool &deleted) const;

private:

	enum CompressorType
	{
		CT_none,
		CT_deprecated,
		CT_zlib,
		CT_max
	};

	struct Header
	{
		Tag    token;
		Tag    version;
		uint32 numberOfFiles;
		uint32 tocOffset;
		uint32 tocCompressor;
		uint32 sizeOfTOC;
		uint32 blockCompressor;
		uint32 sizeOfNameBlock;
		uint32 uncompSizeOfNameBlock;
	};

public:

	struct TableOfContentsEntry
	{
		uint32 crc;
		int    length;
		int    offset;
		int    compressor;
		int    compressedLength;
		int    fileNameOffset;
	};

private:

	char                   *m_treeFileName;
	FileStreamer::File     *m_treeFile;
	uint32                  m_version;
	int                     m_numberOfFiles;
	char                   *m_fileNames;
	TableOfContentsEntry   *m_tableOfContents;
};

// ======================================================================

inline bool TreeFile::SearchTree::isCompressed(int compressorIndex)
{
	DEBUG_FATAL(compressorIndex == CT_deprecated, ("No longer supported compressor"));
	return compressorIndex != static_cast<int>(CT_none);
}

// ======================================================================

class TreeFile::SearchTOC : public TreeFile::SearchNode
{
public:

	static bool        validate(const char *fileName);
	static bool        isCompressed(int compressorIndex);

public:

	SearchTOC(int priority, const char *fileName);
	virtual ~SearchTOC();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

private:

	// disabled
	SearchTOC();

	// disabled
	SearchTOC(const SearchTOC &);

	// disabled
	SearchTOC &operator =(const SearchTOC &);

private:

	bool localExists(const char *fileName, int *index) const;

private:

	enum CompressorType
	{
		CT_none,
		CT_deprecated,
		CT_zlib,
		CT_max
	};

	struct Header
	{
		Tag    token;
		Tag    version;
		uint8  tocCompressor;
		uint8  fileNameBlockCompressor;
		uint8  unusedOne;
		uint8  unusedTwo;
		uint32 numberOfFiles;
		uint32 sizeOfTOC;
		uint32 sizeOfNameBlock;
		uint32 uncompSizeOfNameBlock;
		uint32 numberOfTreeFiles;
		uint32 sizeOfTreeFileNameBlock;
	};

public:

	// unused entry is there to verify 32 bit word alignment
	struct TableOfContentsEntry
	{
		uint8  compressor;
		uint8  unused;
		uint16 treeFileIndex;
		uint32 crc;
		uint32 fileNameOffset;
		uint32 offset;
		uint32 length;
		uint32 compressedLength;
	};

private:

	char                   *m_TOCFileName;
	FileStreamer::File     *m_TOCFile;
	FileStreamer::File     **m_treeFiles;
	uint32                 m_numberOfTreeFiles;
	uint32                 m_numberOfFiles;
	char                   *m_treeFileNames;
	char                   **m_treeFileNamePointers;
	TableOfContentsEntry   *m_tableOfContents;
	char                   *m_fileNames;
};

// ======================================================================

inline bool TreeFile::SearchTOC::isCompressed(int compressorIndex)
{
	DEBUG_FATAL(compressorIndex == CT_deprecated, ("No longer supported compressor"));
	return compressorIndex != static_cast<int>(CT_none);
}

// ======================================================================

class TreeFile::SearchCache : public TreeFile::SearchNode
{
public:

	explicit SearchCache(int priority);
	virtual ~SearchCache();

	int                   addCachedFile(char const * fileName);

	virtual void          debugPrint();
	virtual bool          exists(char const * fileName, bool & deleted) const;
	virtual int           getFileSize(char const * fileName, bool & deleted) const;
	virtual void          getPathName(char const * fileName, char * pathName, int pathNameLength) const;
	virtual AbstractFile *open(char const * fileName, AbstractFile::PriorityType priority, bool & deleted);

private:

	SearchCache();
	SearchCache(const SearchCache &);
	SearchCache &operator =(const SearchCache &);

private:

	class CachedFile;
	typedef std::map<CrcString const *, CachedFile *, LessPointerComparator> CachedFileMap;
	CachedFileMap * const m_cachedFileMap;
};

// ======================================================================

#endif
