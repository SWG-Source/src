// ======================================================================
//
// Callback.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_IndexedCache_H
#define INCLUDED_IndexedCache_H

#include <string>

class AbstractFile;

namespace Archive
{
	class ByteStream;
};

/*
IndexedCache provides a cache of variable-sized data indexed by an integer.

It is intended to provide a means of storing data which requires a significant amount
of time to produce (on the order of hundreds of milliseconds). Each read requires
at most two seeks; but the index will probably be cached in RAM, and in many cases
the data will as well, especially if the file has been packed.

It is essentially a very simple append-only filesystem.

The structure of the file is as follows:
1. A simple header
2. An array of offsets (from the start of file)
3. A sequence of index/length/arbitrary data

Updates are made by writing the data (never overwriting existing data) and then writing
the index part of the table. Assuming writes commit in order, no incorrect data can 
be stored. 

File corruption can still occur, however it does not damage data integrity. Appending
data and then crashing before the index can be updated will leave extra data at the end
of the file. Simply iterating through the data allows us to rebuild the most recent index.

*/

class IndexedCache
{
public:
	// Open the spatial cache. If it is not present or the stored width, height, and version are different, recreate it.
	IndexedCache(const std::string &filename, int length, int version);
	virtual ~IndexedCache();

  /* attempt to get data into the cache. 
	If there is no data in the cache, returns 0.
	If the index is out of range, returns 0 and no error.
	*/
	Archive::ByteStream * getData(int index) const;

  /* Test to see if the data is already in the cache
	If there is no data in the cache, returns false.
	If the index is out of range, returns true. (The typical 
	use is to check contains() before creating the bytestream 
	to write data. This prevents an out-of-range index from 
	having to generate data)
	*/
	bool contains(int index) const;

	/* Set data into the cache.
	The new data is appended and the index is updated, even if the data is already in the cache.  
	Use contains() to test if needed.  This can potentially cause duplicate data to
	be entered into the cache twice, but this will typicallly not be a problem.
	This is also useful to allow partial caching, updating the cached data as more
	information becomes available.
	*/
	void setData(int index, Archive::ByteStream const * data);

	/* Repacks and sorts the file by index order.
	Returns false if the repack fails for any reason.
	Any attempts to access the file during repacking will fail.
	Repacking will fail if the file is already opened for use.
	*/
	static bool repack(const std::string &filename);

	int getSize() { return length; }
	int getVersion() { return version; }
private:
	IndexedCache(const std::string &filename);
	struct Header
	{
		int elements;
		int formatVersion;
		int version;
	};
	struct Entry
	{
		int offset;
	};
	struct Datum
	{
		int index;
		int length;
		// char data[length];
	};

private:
	std::string filename;
	int length;
	int version;
	mutable AbstractFile * file;

	void initializeFile(int version);
	static void lockExclusive(const std::string &filename); // used to open the file for repacking.
	static void unlockExclusive(const std::string &filename);
	void lockForRead() const;
	void lockForWrite();
	void unlock() const;
	void readEntry(int index, Entry &o_entry) const;
	void writeEntry(int index, Entry const &i_entry);

};

#endif

