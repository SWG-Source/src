// ======================================================================
//
// TreeFile_SearchNode.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/TreeFile_SearchNode.h"

#include "sharedCompression/Compressor.h"
#include "sharedCompression/ZlibCompressor.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/FileStreamerFile.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFile/MemoryFile.h"
#include "sharedFile/ZlibFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedSynchronization/Mutex.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

const Tag TAG_TREE = TAG(T,R,E,E);
const Tag TAG_TOC  = TAG3(T,O,C);

// ======================================================================

TreeFile::SearchNode::SearchNode(int priority)
:
	m_priority(priority)
{
}

// ----------------------------------------------------------------------

TreeFile::SearchNode::~SearchNode(void)
{
}
// ======================================================================

TreeFile::SearchPath::SearchPath(int priority, const char *path)
: SearchNode(priority),
	m_pathName(nullptr),
	m_pathNameLength(0)
{
	NOT_NULL(path);
	DEBUG_FATAL(!path[0], ("empty path"));

	// convert from relative to absolute path
	char absolutePath[Os::MAX_PATH_LENGTH];
	const bool result = Os::getAbsolutePath(path, absolutePath, sizeof(absolutePath));
	FATAL(!result, ("Could not convert to absolute path.  Does it exist?  %s", path));

	// clean the path name up and remove any trailing slash
	// this function will remove leading slashes but we actually want them, so make sure we preserve it
	int const offset = (absolutePath[0] == '/') ? 1 : 0;
	TreeFile::fixUpFileName(absolutePath+offset, absolutePath+offset);
	m_pathNameLength = strlen(absolutePath);
	if (m_pathNameLength && absolutePath[m_pathNameLength-1] == '/')
		absolutePath[--m_pathNameLength] = '\0';

	// copy the path to a safe place
	m_pathName = DuplicateString(absolutePath);
}

// ----------------------------------------------------------------------

TreeFile::SearchPath::~SearchPath(void)
{
	delete [] m_pathName;
}

// ----------------------------------------------------------------------

void TreeFile::SearchPath::debugPrint(void)
{
	DEBUG_REPORT_PRINT(true, ("  %d=priority %s=path\n", getPriority(), m_pathName));
	DEBUG_OUTPUT_STATIC_VIEW("Foundation\\Treefile", ("  %d=priority %s=path\n", getPriority(), m_pathName));
}

// ----------------------------------------------------------------------

void TreeFile::SearchPath::makeAbsolutePath(const char *fileName, char *buffer) const
{
	NOT_NULL(fileName);
	NOT_NULL(buffer);
	DEBUG_FATAL(strlen(m_pathName) + 1 + strlen(fileName) + 1 > Os::MAX_PATH_LENGTH, ("file name to long %d/%d", strlen(m_pathName) + strlen(fileName) + 1, Os::MAX_PATH_LENGTH));

	strcpy(buffer, m_pathName);
	buffer[m_pathNameLength] = '/';
	strcpy(buffer+m_pathNameLength+1, fileName);
}

// ----------------------------------------------------------------------

bool TreeFile::SearchPath::exists(const char *fileName, bool &) const 
{
	char buffer[Os::MAX_PATH_LENGTH];
	makeAbsolutePath(fileName, buffer);
	return FileStreamer::exists(buffer);
}

// ----------------------------------------------------------------------

int TreeFile::SearchPath::getFileSize(const char *fileName, bool &) const 
{
	char buffer[Os::MAX_PATH_LENGTH];
	makeAbsolutePath(fileName, buffer);
	return FileStreamer::getFileSize(buffer);
}

// ----------------------------------------------------------------------

void TreeFile::SearchPath::getPathName(const char *fileName, char *outPathName, int outPathNameLength) const
{
	UNREF(outPathNameLength);
	DEBUG_FATAL(istrlen(m_pathName) + 1 + istrlen(fileName) + 1 > outPathNameLength, ("file name too long %d/%d", strlen(m_pathName) + 1 + strlen(fileName) + 1, outPathNameLength));

#ifdef _DEBUG
	bool deleted = false;
	DEBUG_FATAL(!exists(fileName, deleted), ("file name doesn't exist"));
#endif

	makeAbsolutePath(fileName, outPathName);
}

// ----------------------------------------------------------------------

AbstractFile *TreeFile::SearchPath::open(const char *fileName, AbstractFile::PriorityType priority, bool &)
{
	char buffer[Os::MAX_PATH_LENGTH];
	makeAbsolutePath(fileName, buffer);
	FileStreamer::File *file = FileStreamer::open(buffer);
	if (!file)
		return nullptr;
	return new FileStreamerFile(priority, *file);
}

// ======================================================================

TreeFile::SearchAbsolute::SearchAbsolute(int priority)
: SearchNode(priority)
{
}

// ----------------------------------------------------------------------

TreeFile::SearchAbsolute::~SearchAbsolute(void)
{
}

// ----------------------------------------------------------------------

void TreeFile::SearchAbsolute::debugPrint(void)
{
}

// ----------------------------------------------------------------------

bool TreeFile::SearchAbsolute::exists(const char *fileName, bool &) const
{
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH, ("Filename too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));
	return FileStreamer::exists(fileName);
}

// ----------------------------------------------------------------------

int TreeFile::SearchAbsolute::getFileSize(const char *fileName, bool &) const
{
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH, ("Filename too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));
	return FileStreamer::getFileSize(fileName);
}

// ----------------------------------------------------------------------

void TreeFile::SearchAbsolute::getPathName(const char *fileName, char *pathName, int pathNameLength) const
{
	NOT_NULL(fileName);
	NOT_NULL(pathName);
#ifdef _DEBUG
	bool deleted = false;
	DEBUG_FATAL(!exists(fileName, deleted), ("fileName does not exist"));
#endif

	UNREF(pathNameLength);
	DEBUG_FATAL(istrlen(fileName) + 1 > pathNameLength, ("Filename too long %d/%d", strlen(fileName) + 1, pathNameLength));
	strcpy(pathName, fileName);
}

// ----------------------------------------------------------------------

AbstractFile *TreeFile::SearchAbsolute::open(const char *fileName, AbstractFile::PriorityType priority, bool &)
{
	NOT_NULL(fileName);
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH, ("Filename too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));

	FileStreamer::File *file = FileStreamer::open(fileName);
	if (!file)
		return nullptr;
	return new FileStreamerFile(priority, *file);
}

// ======================================================================

bool TreeFile::SearchTree::validate(const char *fileName)
{
	// open the file
	FileStreamer::File *file = FileStreamer::open(fileName);
	if (!file)
		return false;

	// read the header
	Header header;
	const int readPos = file->read(0, &header, sizeof(header), AbstractFile::PriorityData);

	// close the file
	delete file;

	// make sure all the bytes were read
	if (readPos != isizeof(header))
		return false;

	// validate the token
	if (header.token != TAG_TREE)
		return false;

	// validate the version number
	if (header.version < TAG_0004 || header.version > TAG_0004)
		return false;

	return true;
}

// ----------------------------------------------------------------------

TreeFile::SearchTree::SearchTree(int priority, const char *fileName)
: SearchNode(priority),
	m_treeFileName(nullptr),
	m_treeFile(nullptr),
	m_version(0),
	m_numberOfFiles(0),
	m_fileNames(nullptr),
	m_tableOfContents(nullptr)
{
	NOT_NULL(fileName);

	// set to the the current name of the tree file that is fileName
	m_treeFileName = DuplicateString(fileName);

	m_treeFile = FileStreamer::open(m_treeFileName, true);
	DEBUG_FATAL(!m_treeFile, ("failed to open TreeFile %s", m_treeFileName));

	// read the header (the first 32 bytes of the tree file) 
	Header header;
	m_treeFile->read(0, &header, sizeof(header), AbstractFile::PriorityData);
	DEBUG_FATAL(header.token != TAG_TREE, ("file does not look like a tree file"));

	// set to the number of files that has been compressed within the tree file
	m_numberOfFiles = static_cast<int>(header.numberOfFiles);

	int readPosition = header.tocOffset;
	m_version = header.version;
	switch (m_version)
	{
		case TAG_0004:
		case TAG_0005:
			{
				m_tableOfContents = new TableOfContentsEntry[m_numberOfFiles];
				m_fileNames = new char [header.uncompSizeOfNameBlock];

				// prepare table of contents by zeroing out the total size of data to be stored
				const int tableOfContentsSize = isizeof(TableOfContentsEntry) * m_numberOfFiles;
				memset(m_tableOfContents, 0, tableOfContentsSize);

				if (isCompressed(header.tocCompressor))
				{
					// create temp buffer to store the compressed TOC entry data
					byte *entryBuffer = new byte[static_cast<uint32>(header.sizeOfTOC)];
				
					// read the compressed table of contents data into buffer
					const int bytesRead = m_treeFile->read(readPosition, entryBuffer, header.sizeOfTOC, AbstractFile::PriorityData);					
					DEBUG_FATAL(bytesRead != static_cast<int>(header.sizeOfTOC), ("failed to read tree file TOC entries"));
					readPosition += bytesRead;

					// decompress data into toc 
					static_cast<void>(ZlibCompressor().expand(entryBuffer, header.sizeOfTOC, m_tableOfContents, tableOfContentsSize));

					delete [] entryBuffer;
				}
				else
				{
					// read the uncompressed table of contents data
					const int bytesRead = m_treeFile->read(header.tocOffset, m_tableOfContents, tableOfContentsSize, AbstractFile::PriorityData);
					DEBUG_FATAL(bytesRead != tableOfContentsSize, ("failed to read tree file tableOfContents entries"));
					readPosition += bytesRead;
				}

				if (header.blockCompressor)
				{
					// create temp buffer to store the compressed name block data
					byte *nameBuffer  = new byte[static_cast<uint32>(header.sizeOfNameBlock)];

					// read the compressed table of contents data into buffer
					const int bytesRead = m_treeFile->read(readPosition, nameBuffer, header.sizeOfNameBlock, AbstractFile::PriorityData);					
					UNREF(bytesRead);
					DEBUG_FATAL(bytesRead != static_cast<int>(header.sizeOfNameBlock), ("failed to read tree file name block"));

					// decompress data into tocFileNames 
					static_cast<void>(ZlibCompressor().expand(nameBuffer, header.sizeOfNameBlock, m_fileNames, header.uncompSizeOfNameBlock));
					
					delete [] nameBuffer;
				}
				else
				{
					// read the uncompressed name block data 
					const int bytesRead = m_treeFile->read(readPosition, m_fileNames, header.uncompSizeOfNameBlock, AbstractFile::PriorityData);
					UNREF(bytesRead);
					DEBUG_FATAL(bytesRead != static_cast<int>(header.uncompSizeOfNameBlock), ("failed to read tree file name block"));
				}
			}
			break;

		default:
			{
				delete m_treeFile;

#if PRODUCTION
				FATAL(true, ("TreeFile corruption detected.  Please do a \"Full Scan\" from the launchpad. (%08x %s)", m_version, m_treeFileName));
#else
				FATAL(true, ("unsupported version %d in %s", m_version, m_treeFileName));
#endif

			}
			break;
	}
}

// ----------------------------------------------------------------------
/**
 * Look for a file in the SearchTree.
 * 
 * If index is passed, the table of contents index is assigned to it.
 * 
 * @return True if found, otherwise false.
 */

bool TreeFile::SearchTree::localExists(const char *fileName, int *index, bool &deleted) const
{
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH,("file name too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));

	const uint32 crc = Crc::calculate(fileName);

	// try a binary search through the tree file to find the file
	bool found       = false;
	int  left        = 0;
	int  right       = m_numberOfFiles - 1;
	int  mid         = 0;
	while (!found && (left <= right))
	{
		mid = (left + right) / 2;

		if (m_tableOfContents[mid].crc < crc)
			left = mid + 1;
		else
			if (m_tableOfContents[mid].crc > crc)
				right = mid - 1;
			else
			{
				const int res = _stricmp(m_fileNames + m_tableOfContents[mid].fileNameOffset, fileName);

				if (res < 0)
					left = mid + 1;
				else
					if (res > 0)
						right = mid - 1;
					else
						found = true;
			}
	}

	// return the found index if desired
	if (found)
	{
		if (m_tableOfContents[mid].length == 0)
		{
			deleted = true;
			return false;
		}

		if (index)
			*index = mid;
	}

	return found;
}

// ----------------------------------------------------------------------

TreeFile::SearchTree::~SearchTree(void)
{
	delete [] m_treeFileName;
	delete [] m_tableOfContents;
	delete [] m_fileNames;
	delete m_treeFile;
}

// ----------------------------------------------------------------------

void TreeFile::SearchTree::debugPrint(void)
{
	DEBUG_REPORT_PRINT(true, ("  %d=priority %s=tree\n", getPriority(), m_treeFileName));
	DEBUG_OUTPUT_STATIC_VIEW("Foundation\\Treefile", ("  %d=priority %s=tree\n", getPriority(), m_treeFileName));
}

// ----------------------------------------------------------------------
/**
 * Look for a file in the SearchTree.
 * 
 * @return True if found, otherwise false.
 */

bool TreeFile::SearchTree::exists(const char *fileName, bool &deleted) const
{
	NOT_NULL(fileName);
	return localExists(fileName, nullptr, deleted);
}

// ----------------------------------------------------------------------

int TreeFile::SearchTree::getFileSize(const char *fileName, bool &deleted) const
{
	NOT_NULL(fileName);
	int tableOfContentsIndex = 0;
	if (!localExists(fileName, &tableOfContentsIndex, deleted))
		return -1;

	return m_tableOfContents[tableOfContentsIndex].length;
}

// ----------------------------------------------------------------------

void TreeFile::SearchTree::getPathName(const char *fileName, char *pathName, int pathNameLength)  const
{
	NOT_NULL(fileName);
	NOT_NULL(pathName);
	UNREF(pathNameLength);

#ifdef _DEBUG
	bool deleted = false;
	DEBUG_FATAL(!exists(fileName, deleted), ("fileName does not exist"));

	int stringLength = istrlen(m_treeFileName) + 1 + istrlen(fileName) + 1 + 1;
	DEBUG_FATAL(stringLength > pathNameLength, ("file name to long %d/%d", stringLength, pathNameLength));
#endif

	// make a pseudo-path name for the tree file
	strcpy(pathName, m_treeFileName);
	strcat(pathName, "[");
	strcat(pathName, fileName);
	strcat(pathName, "]");
}

// ----------------------------------------------------------------------

AbstractFile *TreeFile::SearchTree::open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted)
{
	NOT_NULL(fileName);
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH,("file name too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));

	int tableOfContentsIndex = -1;
	if (localExists(fileName, &tableOfContentsIndex, deleted))
	{
		const TableOfContentsEntry &entry = m_tableOfContents[tableOfContentsIndex];

		if (!TreeFile::SearchTree::isCompressed(entry.compressor))
			return new FileStreamerFile(priority, *m_treeFile, entry.offset, entry.length);

		byte * compressedBuffer = new byte[entry.compressedLength];

		const int bytesRead = m_treeFile->read(entry.offset, compressedBuffer, entry.compressedLength, priority);
		DEBUG_FATAL(bytesRead != entry.compressedLength, ("error reading compressed data into buffer"));
		UNREF(bytesRead);

		return new ZlibFile(entry.length, compressedBuffer, entry.compressedLength, true);
	}

	return nullptr;
}

// ======================================================================

bool TreeFile::SearchTOC::validate(const char *fileName)
{
	// open the file
	FileStreamer::File *file = FileStreamer::open(fileName);
	if (!file)
		return false;

	// read the header
	Header header;
	const int readPos = file->read(0, &header, sizeof(header), AbstractFile::PriorityData);

	// close the file
	delete file;

	// make sure all the bytes were read
	if (readPos != isizeof(header))
		return false;

	// validate the token
	if (header.token != TAG_TOC)
		return false;

	// validate the version number
	if (header.version != TAG_0001)
		return false;

	return true;
}

// ----------------------------------------------------------------------

TreeFile::SearchTOC::SearchTOC(int priority, const char *fileName)
: SearchNode(priority),
	m_TOCFileName(nullptr),
	m_TOCFile(nullptr),
	m_treeFiles(nullptr),
	m_numberOfFiles(0),
	m_treeFileNames(nullptr),
	m_treeFileNamePointers(nullptr),
	m_tableOfContents(nullptr),
	m_fileNames(nullptr)
{
	NOT_NULL(fileName);

	// set to the the current name of the TOC file to the fileName
	m_TOCFileName = DuplicateString(fileName);

	m_TOCFile = FileStreamer::open(m_TOCFileName, true);
	DEBUG_FATAL(!m_TOCFile, ("failed to open TOCFile %s", m_TOCFileName));

	// read the header
	Header header;
	m_TOCFile->read(0, &header, sizeof(header), AbstractFile::PriorityData);
	DEBUG_FATAL(header.token != TAG_TOC, ("file does not look like a table of contents file"));

	// grab the number of files
	m_numberOfFiles = header.numberOfFiles;

	// grab the number of tree files
	m_numberOfTreeFiles = header.numberOfTreeFiles;

	// set the read position to after the header (beginning of patch tree names)
	int readPosition = isizeof(Header);
	uint32 version = header.version;
	switch (version)
	{
		case TAG_0001:
			{
				m_tableOfContents = new TableOfContentsEntry [m_numberOfFiles];
				m_fileNames = new char [header.uncompSizeOfNameBlock];
				m_treeFileNames = new char [header.sizeOfTreeFileNameBlock];
				m_treeFiles = new FileStreamer::File* [header.numberOfTreeFiles];
				m_treeFileNamePointers = new char* [header.numberOfTreeFiles];

				{
					// get any paths we need to check to open the tree files
					std::vector<const char *> treePaths;
					char *treePathBuffer = new char[Os::MAX_PATH_LENGTH];
					
					// add on the current path (an empty string) to the list of paths
					char *emptyPath = new char('\0');
					treePaths.push_back(emptyPath);

					// add on all paths in config file
					const char * result;
					for (int index = 0; (result = ConfigFile::getKeyString("SharedFile", "TOCTreePath", index, nullptr)) != nullptr; ++index)
						treePaths.push_back(result);

					// read in the tree file names and open the files
					const int bytesRead = m_TOCFile->read(readPosition, m_treeFileNames, header.sizeOfTreeFileNameBlock, AbstractFile::PriorityData);
					DEBUG_FATAL(bytesRead != static_cast<int>(header.sizeOfTreeFileNameBlock), ("failed to read tree file name entries"));
					readPosition += bytesRead;

					for (int treeFileNameIndex = 0, treeFileNameReadPosition = 0; treeFileNameIndex < static_cast<int>(header.numberOfTreeFiles); treeFileNameIndex++)
					{
						m_treeFileNamePointers[treeFileNameIndex] = (m_treeFileNames + treeFileNameReadPosition);
						m_treeFiles[treeFileNameIndex] = nullptr;						

						// try to open the tree file in each of the relative paths 
						for (std::vector<const char *>::const_iterator pathIter = treePaths.begin(); pathIter != treePaths.end(); ++pathIter)
						{
							strcpy(treePathBuffer, *pathIter);
							strcat(treePathBuffer, (m_treeFileNames + treeFileNameReadPosition));

							if (FileStreamer::exists (treePathBuffer))
							{
								m_treeFiles[treeFileNameIndex] = FileStreamer::open(treePathBuffer, true);
								break;
							}
						}

						FATAL(!m_treeFiles[treeFileNameIndex], ("failed to open tree file index %d, offset %d, name %s", treeFileNameIndex, treeFileNameReadPosition, m_treeFileNames + treeFileNameReadPosition));

						treeFileNameReadPosition += (strlen(m_treeFileNames + treeFileNameReadPosition) + 1);
					}

					delete [] treePathBuffer;
					delete emptyPath;
				}

				// prepare table of contents by zeroing out the total size of data to be stored
				const int tableOfContentsSize = isizeof(TableOfContentsEntry) * m_numberOfFiles;
				if (isCompressed(header.tocCompressor))
				{
					// create temp buffer to store the compressed TOC entry data
					byte *entryBuffer = new byte[header.sizeOfTOC];

					// read the compressed table of contents data into buffer
					const int bytesRead = m_TOCFile->read(readPosition, entryBuffer, header.sizeOfTOC, AbstractFile::PriorityData);
					DEBUG_FATAL(bytesRead != static_cast<int>(header.sizeOfTOC), ("failed to read tableOfContents entries"));
					readPosition += bytesRead;

					// decompress data into toc
					static_cast<void>(ZlibCompressor().expand(entryBuffer, header.sizeOfTOC, m_tableOfContents, tableOfContentsSize));

					delete [] entryBuffer;
				}
				else
				{
					// read the uncompressed table of contents data
					const int bytesRead = m_TOCFile->read(readPosition, m_tableOfContents, tableOfContentsSize, AbstractFile::PriorityData);
					DEBUG_FATAL(bytesRead != tableOfContentsSize, ("failed to read tableOfContents entries"));
					readPosition += bytesRead;
				}

				// After the TableOfContents is read into memory, the fileNameLengths must be changed to fileNameOffsets
				{
					int currentFileNameOffset = 0;
					int currentFileNameLength = 0;

					for (uint32 i = 0; i < m_numberOfFiles; ++i)
					{
						currentFileNameLength = m_tableOfContents[i].fileNameOffset;
						m_tableOfContents[i].fileNameOffset = currentFileNameOffset;
						// + 1 for the nullptr termination
						currentFileNameOffset += (currentFileNameLength + 1);
					}
				}

				if (isCompressed(header.fileNameBlockCompressor))
				{
					// create temp buffer to store the compressed name block data
					byte *nameBuffer  = new byte[header.uncompSizeOfNameBlock];

					// read the compressed table of contents data into buffer
					const int bytesRead = m_TOCFile->read(readPosition, nameBuffer, header.sizeOfNameBlock, AbstractFile::PriorityData);
					UNREF(bytesRead);
					DEBUG_FATAL(bytesRead != static_cast<int>(header.sizeOfNameBlock), ("failed to read file name block"));

					// decompress data into tocFileNames
					static_cast<void>(ZlibCompressor().expand(nameBuffer, header.sizeOfNameBlock, m_fileNames, header.uncompSizeOfNameBlock));

					delete [] nameBuffer;
				}
				else
				{
					// read the uncompressed name block data
					const int bytesRead = m_TOCFile->read(readPosition, m_fileNames, header.sizeOfNameBlock, AbstractFile::PriorityData);
					UNREF(bytesRead);
					DEBUG_FATAL(bytesRead != static_cast<int>(header.sizeOfNameBlock), ("failed to read file name block"));
				}

			}
			break;

		default:
			{
				delete m_TOCFile;

#if PRODUCTION
				FATAL(true, ("Table of Contents File corruption detected.  Please do a \"Full Scan\" from the launchpad. (%08x %s)", version, m_TOCFileName));
#else
				FATAL(true, ("unsupported version %d in %s", version, m_TOCFileName));
#endif

			}
			break;
	}

}

// ----------------------------------------------------------------------

TreeFile::SearchTOC::~SearchTOC(void)
{
	delete [] m_TOCFileName;
	delete [] m_treeFileNames;
	delete [] m_tableOfContents;
	delete [] m_fileNames;
	delete [] m_treeFileNamePointers;

	// clear out FileStreamer::File pointers
	for (uint32 i = 0; i < m_numberOfTreeFiles; i++)
		delete m_treeFiles[i];
	delete [] m_treeFiles;

	delete m_TOCFile;
}

// ----------------------------------------------------------------------

void TreeFile::SearchTOC::debugPrint(void)
{
	DEBUG_REPORT_PRINT(true, ("  %d=priority %s=tree\n", getPriority(), m_TOCFileName));
	DEBUG_OUTPUT_STATIC_VIEW("Foundation\\TOCfile", ("  %d=priority %s=tree\n", getPriority(), m_TOCFileName));
}

// ----------------------------------------------------------------------

bool TreeFile::SearchTOC::localExists(const char *fileName, int *index) const
{
	NOT_NULL(fileName);
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH,("file name too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));

	const uint32 crc = Crc::calculate(fileName);

	// try a binary search through the tree file to find the file
	bool found       = false;
	int  left        = 0;
	int  right       = m_numberOfFiles - 1;
	int  mid         = 0;
	while (!found && (left <= right))
	{
		mid = (left + right) / 2;

		if (m_tableOfContents[mid].crc < crc)
			left = mid + 1;
		else
			if (m_tableOfContents[mid].crc > crc)
				right = mid - 1;
			else
			{
				const int res = _stricmp(m_fileNames + m_tableOfContents[mid].fileNameOffset, fileName);

				if (res < 0)
					left = mid + 1;
				else
					if (res > 0)
						right = mid - 1;
					else
						found = true;
			}
	}

	// return the found index if desired
	if (found)
	{
		if (m_tableOfContents[mid].length == 0)
		{
			return false;
		}
		else if (m_tableOfContents[mid].offset == 0)
		{
			// sanity check - if the file has a length, but the offset is zero, the pointer in the TOC is invalid
			return false;
		}

		if (index)
			*index = mid;
	}

	return found;
}

// ----------------------------------------------------------------------

bool TreeFile::SearchTOC::exists(const char *fileName, bool &deleted) const
{
	NOT_NULL(fileName);
	deleted = false;
	return localExists(fileName, nullptr);
}

// ----------------------------------------------------------------------

int TreeFile::SearchTOC::getFileSize(const char *fileName, bool &deleted) const
{
	NOT_NULL(fileName);
	deleted = false;
	int tableOfContentsIndex = 0;
	if (!localExists(fileName, &tableOfContentsIndex))
		return -1;

	return m_tableOfContents[tableOfContentsIndex].length;
}

// ----------------------------------------------------------------------

void TreeFile::SearchTOC::getPathName(const char *fileName, char *pathName, int pathNameLength)  const
{
	NOT_NULL(fileName);
	NOT_NULL(pathName);
	UNREF(pathNameLength);

	// make a pseudo-path name for the tree file
	int tableOfContentsIndex = 0;
	if(!localExists(fileName, &tableOfContentsIndex))
		return;

	char* treeFileName = m_treeFileNamePointers[m_tableOfContents[tableOfContentsIndex].treeFileIndex];

#ifdef _DEBUG
	bool deleted;
	DEBUG_FATAL(!exists(fileName, deleted), ("fileName does not exist"));

	int stringLength = istrlen(treeFileName) + 1 + istrlen(fileName) + 1 + 1;
	DEBUG_FATAL(stringLength > pathNameLength, ("file name to long %d/%d", stringLength, pathNameLength));
#endif

	strcpy(pathName, treeFileName);
	strcat(pathName, "[");
	strcat(pathName, fileName);
	strcat(pathName, "]");
}

// ----------------------------------------------------------------------

AbstractFile *TreeFile::SearchTOC::open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted)
{
	NOT_NULL(fileName);
	DEBUG_FATAL(strlen(fileName) + 1 > Os::MAX_PATH_LENGTH,("file name too long %d/%d", strlen(fileName) + 1, Os::MAX_PATH_LENGTH));
	deleted = false;

	int tableOfContentsIndex = -1;
	if (localExists(fileName, &tableOfContentsIndex))
	{
		const TableOfContentsEntry &entry = m_tableOfContents[tableOfContentsIndex];

		if (!isCompressed(entry.compressor))
			return new FileStreamerFile(priority, *m_treeFiles[entry.treeFileIndex], entry.offset, entry.length);

		byte * compressedBuffer = new byte[entry.compressedLength];

		const uint32 bytesRead = m_treeFiles[entry.treeFileIndex]->read(entry.offset, compressedBuffer, entry.compressedLength, priority);
		DEBUG_FATAL(bytesRead != entry.compressedLength, ("error reading compressed data into buffer"));
		UNREF(bytesRead);

		return new ZlibFile(entry.length, compressedBuffer, entry.compressedLength, true);
	}

	return nullptr;
}

// ======================================================================

class TreeFile::SearchCache::CachedFile
{
public:

	explicit CachedFile(char const * fileName, AbstractFile * abstractFile);
	~CachedFile();

	CrcString const * getCrcString () const;
	bool              getCompressed() const;
	int               getUncompressedLength() const;
	int               getCompressedLength() const;
	AbstractFile *    createAbstractFile() const;

private:

	CachedFile();
	CachedFile(CachedFile const &);
	CachedFile & operator= (CachedFile const &);

private:

	PersistentCrcString const m_name;

	byte * m_buffer;
	int    m_length;
	bool   m_compressed;
	int    m_uncompressedLength;
};

// ----------------------------------------------------------------------

TreeFile::SearchCache::CachedFile::CachedFile(char const * fileName, AbstractFile * abstractFile) :
	m_name(fileName, true),
	m_buffer(0),
	m_length(0),
	m_compressed(false),
	m_uncompressedLength(abstractFile->length())
{
	if (abstractFile->isZlibCompressed())
	{
		abstractFile->getZlibCompressedDataAndClose(m_buffer, m_length);
		m_compressed = true;
	}
	else
	{
		m_buffer = abstractFile->readEntireFileAndClose();
		m_length = m_uncompressedLength;
	}
}

// ----------------------------------------------------------------------

TreeFile::SearchCache::CachedFile::~CachedFile()
{
	delete [] m_buffer;
	m_buffer = 0;
}

// ----------------------------------------------------------------------

CrcString const * TreeFile::SearchCache::CachedFile::getCrcString () const
{
	return &m_name;
}

// ----------------------------------------------------------------------

bool TreeFile::SearchCache::CachedFile::getCompressed() const
{
	return m_compressed;
}

// ----------------------------------------------------------------------

int TreeFile::SearchCache::CachedFile::getUncompressedLength() const
{
	return m_compressed ? m_uncompressedLength : m_length;
}

// ----------------------------------------------------------------------

int TreeFile::SearchCache::CachedFile::getCompressedLength() const
{
	return m_length;
}

// ----------------------------------------------------------------------

AbstractFile * TreeFile::SearchCache::CachedFile::createAbstractFile() const
{
	if (m_compressed)
	{
		return new ZlibFile(m_uncompressedLength, m_buffer, m_length, false);
	}
	else
	{
		byte * const uncompressedData = new byte[m_length];
		memcpy(uncompressedData, m_buffer, m_length);
		return new MemoryFile(uncompressedData, m_length);
	}
}

// ======================================================================

namespace TreeFileSearchCacheNamespace
{
	bool ms_logSearchCache;
}

using namespace TreeFileSearchCacheNamespace;

// ----------------------------------------------------------------------

TreeFile::SearchCache::SearchCache(int const priority) : 
	SearchNode(priority),
	m_cachedFileMap(new CachedFileMap)
{
#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_logSearchCache, "SharedFile", "logSearchCache");
#endif
}

// ----------------------------------------------------------------------

TreeFile::SearchCache::~SearchCache()
{
	std::for_each (m_cachedFileMap->begin(), m_cachedFileMap->end(), PointerDeleterPairSecond());
	m_cachedFileMap->clear();
	delete m_cachedFileMap;

#if PRODUCTION == 0
	DebugFlags::unregisterFlag(ms_logSearchCache);
#endif
}

// ----------------------------------------------------------------------

int TreeFile::SearchCache::addCachedFile(char const * const fileName)
{
	bool deleted = false;
	int fileSize = 0;
	if (exists(fileName, deleted))
		DEBUG_WARNING(true, ("Skipping existing cached file %s", fileName));
	else
	{
		AbstractFile * const abstractFile = TreeFile::open(fileName, AbstractFile::PriorityData, false);
		CachedFile * const cachedFile = new CachedFile(fileName, abstractFile);
		fileSize = cachedFile->getCompressedLength();
		delete abstractFile;

		bool const result = (m_cachedFileMap->insert(CachedFileMap::value_type (cachedFile->getCrcString(), cachedFile))).second;
		if (result)
		{
#if PRODUCTION == 0
			if (ms_logSearchCache)
			{
				if (cachedFile->getCompressed())
					REPORT_LOG(true, ("Adding cached file %s, compressed, %i/%i bytes\n", fileName, cachedFile->getCompressedLength(), cachedFile->getUncompressedLength()));
				else
					REPORT_LOG(true, ("Adding cached file %s, uncompressed, %i bytes\n", fileName, cachedFile->getUncompressedLength()));
			}
#endif
		}
		else
		{
			DEBUG_FATAL(!result, ("TreeFile::SearchCache::addCachedFile: insert failed for %s (possibly a duplicate entry)", fileName));
			delete cachedFile;
		}
	}

	return fileSize;
}

// ----------------------------------------------------------------------

void TreeFile::SearchCache::debugPrint()
{
	DEBUG_REPORT_PRINT(true, ("  %d=priority SearchCache [%i]\n", getPriority(), m_cachedFileMap->size()));
	DEBUG_OUTPUT_STATIC_VIEW("Foundation\\Treefile", ("  %d=priority SearchCache [%i]\n", getPriority(), m_cachedFileMap->size()));
}

// ----------------------------------------------------------------------

bool TreeFile::SearchCache::exists(char const * const fileName, bool & deleted) const
{
	deleted = false;

	TemporaryCrcString const crcString(fileName, true);
	return m_cachedFileMap->find((const CrcString*)&crcString) != m_cachedFileMap->end();
}

// ----------------------------------------------------------------------

int TreeFile::SearchCache::getFileSize(char const * const fileName, bool & deleted) const
{
	deleted = false;

	TemporaryCrcString const crcString(fileName, true);
	CachedFileMap::iterator iter = m_cachedFileMap->find((const CrcString*)&crcString);
	if (iter != m_cachedFileMap->end())
		return iter->second->getUncompressedLength();

	return -1;
}

// ----------------------------------------------------------------------

void TreeFile::SearchCache::getPathName(char const * const fileName, char * const pathName, int const pathNameLength) const
{
	NOT_NULL(fileName);
	NOT_NULL(pathName);
	UNREF(pathNameLength);

#ifdef _DEBUG
	bool deleted = false;
	DEBUG_FATAL(!exists(fileName, deleted), ("fileName does not exist"));

	int const stringLength = 13 + istrlen(fileName) + 1 + 1;
	DEBUG_FATAL(stringLength > pathNameLength, ("file name to long %d/%d", stringLength, pathNameLength));
#endif

	// make a pseudo-path name for the tree file
	strcpy(pathName, "SearchCache[");
	strcat(pathName, fileName);
	strcat(pathName, "]");
}

// ----------------------------------------------------------------------

AbstractFile * TreeFile::SearchCache::open(char const * const fileName, AbstractFile::PriorityType const, bool & deleted)
{
	deleted = false;

	TemporaryCrcString const crcString(fileName, true);
	CachedFileMap::iterator iter = m_cachedFileMap->find((const CrcString*)&crcString);
	if (iter != m_cachedFileMap->end())
		return iter->second->createAbstractFile();

	return 0;
}

// ======================================================================

