// ======================================================================
//
// IndexedCache.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/IndexedCache.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "fileInterface/StdioFile.h"

#include <algorithm>


IndexedCache::IndexedCache(const std::string &i_filename, int i_length, int i_version)
: filename(i_filename), length(i_length), version(i_version)
{
	lockExclusive(filename);
	file = new StdioFile(filename.c_str(), "r+b");
	if (!file->isOpen())
	{
		delete file;
		file = new StdioFile(filename.c_str(), "w+b");
		if (!file->isOpen())
		{
			delete file;
			file = 0;
		}
	}
	if (file)
	{
		if (file->length() == 0)
		{
			// Write the header out
			initializeFile(i_version);
		}
		else
		{
			file->seek(AbstractFile::SeekBegin, 0);

			Header header;
			file->read(&header, sizeof(header));
			if (header.elements != i_length || header.version != i_version)
			{
				initializeFile(i_version);
			}
		}
	}
	else
	{
		length = 0;
	}
	unlockExclusive(filename);
}

IndexedCache::IndexedCache(const std::string &i_filename)
: filename(i_filename), length(0), version(0)
{
	file = new StdioFile(filename.c_str(), "r+b");
	if (file->length() > 0)
	{
		file->seek(AbstractFile::SeekBegin, 0);

		Header header;
		file->read(&header, sizeof(header));
		length = header.elements;
		version = header.version;
	}
}

IndexedCache::~IndexedCache()
{
	delete file;
	file = 0;
}

void IndexedCache::initializeFile(int i_version)
{
	file->seek(AbstractFile::SeekBegin, 0);

	Header header;
	header.elements = length;
	header.formatVersion = 1;
	header.version = i_version;
	file->write(sizeof(header), &header);

	// Write out a bunch of zero entries
	int toWrite = length;
	const int COUNT = 8192;
	Entry * entries = new Entry[COUNT];
	int x;
	for (x=0; x<COUNT; ++x)
	{
		entries[x].offset = 0;
	}
	while (toWrite > 0)
	{
		file->write(std::min(toWrite, COUNT) * sizeof(Entry), entries);
		toWrite -= COUNT;
	}

	delete [] entries;

	file->flush();
}

/* attempt to get data into the cache. 
If there is no data in the cache, returns 0.
If the index is out of range, returns 0 and no error.
*/
Archive::ByteStream * IndexedCache::getData(int index) const
{
	if (index < 0 || index >= length)
		return 0;

	lockForRead();

	Entry entry;
	readEntry(index, entry);

	if (entry.offset == 0)
	{
		unlock();
		return 0;
	}
	file->seek(AbstractFile::SeekBegin, entry.offset);
	Datum datum;
	file->read(&datum, sizeof(datum));
	unsigned char * data = new unsigned char[datum.length];
	file->read(data, datum.length);
	Archive::ByteStream * result = new Archive::ByteStream(data, datum.length);
	delete [] data;
	unlock();

	return result;
}

/* Test to see if the data is already in the cache
If there is no data in the cache, returns false.
If the index is out of range, returns true. (The typical 
use is to check contains() before creating the bytestream 
to write data. This prevents an out-of-range index from 
having to generate data)
*/
bool IndexedCache::contains(int index) const
{
	if (index < 0 || index >= length)
		return true;

	lockForRead();

	Entry entry;
	readEntry(index, entry);

	unlock();
	return entry.offset != 0;
}

/* Set data into the cache.
The new data is appended and the index is updated, even if the data is already in the cache.  
Use contains() to test if needed.  This can potentially cause duplicate data to
be entered into the cache twice, but this will typicallly not be a problem.
This is also useful to allow partial caching, updating the cached data as more
information becomes available.
*/
void IndexedCache::setData(int index, Archive::ByteStream const * data)
{
	if (index < 0 || index >= length)
		return;

	lockForWrite();

	Entry entry;

	file->seek(AbstractFile::SeekEnd, 0);
	entry.offset = file->tell();

	Datum datum;
	datum.index = index;
	datum.length = data->getSize();
	file->write(sizeof(datum), &datum);
	file->write(data->getSize(), data->getBuffer());

	writeEntry(index, entry);

	file->flush();

	unlock();
}

void IndexedCache::readEntry(int index, Entry &o_entry) const
{
	file->seek(AbstractFile::SeekBegin, sizeof(Header) + index * sizeof(Entry));
	file->read(&o_entry, sizeof(o_entry));
}

void IndexedCache::writeEntry(int index, Entry const &i_entry)
{
	file->seek(AbstractFile::SeekBegin, sizeof(Header) + index * sizeof(Entry));
	file->write(sizeof(i_entry), &i_entry);
}

void IndexedCache::lockExclusive(const std::string &filename)
{
	UNREF(filename);
}

void IndexedCache::unlockExclusive(const std::string &filename)
{
	UNREF(filename);
}

void IndexedCache::lockForRead() const
{
}

void IndexedCache::lockForWrite()
{
}

void IndexedCache::unlock() const
{
}

/* Repacks and sorts the file by index order.
Returns false if the repack fails for any reason.
Any attempts to access the file during repacking will fail.
Repacking will fail if the file is already opened for use.
*/
bool IndexedCache::repack(const std::string &filename)
{
	lockExclusive(filename);

	std::string outputFilename(filename + ".repack");

	{
		IndexedCache input(filename);
		int size = input.getSize();
		if (size == 0)
		{
			unlockExclusive(filename);
			return false;
		}
		
		IndexedCache output(filename, size, input.getVersion());
		int x;
		for (x=0; x<size; ++x)
		{
			Archive::ByteStream * data = input.getData(x);
			if (data)
			{
				output.setData(x, data);
				delete data;
			}
		}
	}

	_unlink(filename.c_str());
	rename(outputFilename.c_str(), filename.c_str());

	unlockExclusive(filename);

	return true;
}
