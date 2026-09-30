//---------------------------------------------------------------------
#include "FirstArchive.h"
#include "ByteStream.h"
#include "ArchiveMutex.h"
#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>

// ======================================================================

//static volatile int s_dataFreeListLocked;
static Archive::ArchiveMutex s_archiveMutex;


// ======================================================================

namespace Archive {

//---------------------------------------------------------------------
/**
	@brief ReadIterator ctor

	Initializes the read position to zero, and the ByteStream member value
	is nullptr
*/
ReadIterator::ReadIterator() :
	readPtr(0),
	stream(0)
{
}

//---------------------------------------------------------------------
/**
	@brief ReadIterator copy constructor

	Sets the current position to the source read position and the
	ByteStream pointer to the source ByteStream.
*/
ReadIterator::ReadIterator(const ReadIterator & source) :
	readPtr(source.readPtr),
	stream(source.stream)
{
}


//---------------------------------------------------------------------
/**
	@brief construct a read iterator with the position at zero and
	the ByteStream set to the source.
*/
ReadIterator::ReadIterator(ByteStream const &source) :
	readPtr(0),
	stream(&source)
{
}

//---------------------------------------------------------------------
/**
	@brief destroy a read iterator object, set the ByteStream member to 
	zero.
*/
ReadIterator::~ReadIterator()
{
	stream = 0;
}

//---------------------------------------------------------------------
/**
	Comparison operator

	Indicates whether the two iterators point to the same place in the
	same bytestream.

	@author Acy Stapp
*/
bool ReadIterator::operator==(ReadIterator const &rhs) const
{
	// We are almost guaranteed to be comparing iterators from the same
	// bytestream, so we put that test last, giving the other test a 
	// chance to fail and avoid the almost certainty.
	if (readPtr != rhs.readPtr)
		return false;
	return stream == rhs.stream;
}

//---------------------------------------------------------------------
/**
	Construct an ByteStream object.

	This constructor merely initializes members, No buffer allocation
	occurs.

	@author Justin Randall
*/
ByteStream::ByteStream() :
	allocatedSize(0),
	allocatedSizeLimit(0),
	beginReadIterator(),
	data(0),
	size(0)
{
	beginReadIterator = ReadIterator(*this);
}

//---------------------------------------------------------------------
/**
	@brief Construct an ByteStream from a byte buffer

	@author Justin Randall
*/
ByteStream::ByteStream(unsigned char const * const newBuffer, const unsigned int bufferSize) :
	allocatedSize(0),
	allocatedSizeLimit(0),
	data(0),
	size(0)
{
	put(newBuffer, bufferSize);
	beginReadIterator = ReadIterator(*this);
}

//---------------------------------------------------------------------
/**
	ByteStream Copy Constructor	
*/
ByteStream::ByteStream(ByteStream const &source):
	allocatedSize(source.getSize()),	// only allocate what is really there, be opportinistic when grow()'ing
	allocatedSizeLimit(0),
	data(source.data),
	size(source.getSize())
{
	if (source.data)
		source.data->ref();
	beginReadIterator = ReadIterator(*this);
}

//-----------------------------------------------------------------------
/**
	@brief ByteStream copy constructor
	
	Creates a new byte stream, deep copying the source stream from the
	source read iterator.
	
*/
ByteStream::ByteStream(ReadIterator &source) :
	allocatedSize(0),
	allocatedSizeLimit(0),
	data(0),
	size(0)
{
	put(source.getBuffer(), source.getSize());
	source.advance(source.getSize());
	beginReadIterator = ReadIterator(*this);
}

//---------------------------------------------------------------------
/**
	ByteStream destructor

	Deletes the assocated buffer and resets allocateSize, buffer, and 
	size to zero.

	@author Justin Randall
*/
ByteStream::~ByteStream()
{
	if (data)
	{
		data->deref();
		data = 0; //lint !e672 (data deref insures the data is deleted if no one references it)
	}
	allocatedSize = 0;
	size = 0;
}

//---------------------------------------------------------------------
/**
	Assignment operator

	If the ByteStream right hand side is not the same ByteStream, performs
	a deep copy of the ByteStream.

	@author Justin Randall
*/
ByteStream &ByteStream::operator=(ByteStream const &rhs)
{
	if (this != &rhs)
	{
		if (data)
			data->deref(); // deref local data
		if (rhs.data)
			rhs.data->ref();
		allocatedSize = rhs.allocatedSize;
		allocatedSizeLimit = rhs.allocatedSizeLimit;
		size = rhs.size;
		data = rhs.data; //lint !e672 (data is ref counted)
	}
	return *this;
}

//---------------------------------------------------------------------
/**
	@brief Accesses ByteStream data

	@param target       a user supplied buffer that must be at least as
	                    large as targetSize
	@param readIterator An accessor that indicates where the read operation
	                    will begin
	@param targetSize   specifies the number of bytes to read from the 
	                    ByteStream

	@author Justin Randall
*/
void ByteStream::get(void *target, ReadIterator &readIterator, const uint32_t targetSize) const
{
	unsigned int const position = readIterator.getReadPosition();
	if (position > size || targetSize > size - position)
		throw ReadException("Archive::ByteStream - read beyond end of buffer");
	if (targetSize)
		memcpy(target, data->buffer + position, targetSize);
}

//---------------------------------------------------------------------
/**
	@brief deep copy new data to the ByteStream

	If the data member is shared with another ByteStream, then it is 
	copied before this operation begins. The ByteStream buffer is increased
	to hold the new data (determined by the requested sourceSize). 

	@param source      a user supplied buffer that contains data to be
	                   copied to the ByteStream
	@param sourceSize  The number of bytes in the source buffer which 
	                   will be copied to the ByteStream.

	@author Justin Randall
*/
void ByteStream::put(void const * const source, const unsigned int sourceSize)
{
	if (sourceSize > (std::numeric_limits<unsigned int>::max)() - size)
		throw std::length_error("Archive::ByteStream - buffer size exceeds unsigned int range");
	if (!sourceSize)
		return;
	unsigned int const newSize = size + sourceSize;
	unsigned int const newCapacity = getAllocationSize(newSize);
	if (!data || data->getRef() > 1 || data->size < newCapacity)
	{
		std::auto_ptr<Data> replacement(Data::getNewData());
		if (replacement->size < newCapacity)
		{
			unsigned char * const buffer = new unsigned char[newCapacity];
			delete[] replacement->buffer;
			replacement->buffer = buffer;
			replacement->size = newCapacity;
		}
		if (size)
			memcpy(replacement->buffer, data->buffer, size);
		// Keep old storage alive until a source inside that storage is copied.
		memcpy(replacement->buffer + size, source, sourceSize);
		if (data)
			data->deref();
		data = replacement.release();
	}
	else
		memcpy(data->buffer + size, source, sourceSize);
	allocatedSize = newCapacity;
	size = newSize;
}

//---------------------------------------------------------------------

void ByteStream::reAllocate(const unsigned int newSize)
{
	if (!data || data->getRef() > 1 || data->size < newSize)
	{
		std::auto_ptr<Data> replacement(Data::getNewData());
		if (replacement->size < newSize)
		{
			unsigned char * const buffer = new unsigned char[newSize];
			delete[] replacement->buffer;
			replacement->buffer = buffer;
			replacement->size = newSize;
		}
		if (size)
			memcpy(replacement->buffer, data->buffer, size);
		if (data)
			data->deref();
		data = replacement.release();
	}
	allocatedSize = newSize;
}

//---------------------------------------------------------------------

std::vector<ByteStream::Data *> &ByteStream::Data::lockDataFreeList()
{
	static DataFreeList freeList;
	s_archiveMutex.enter();
	return freeList.freeList;
}

//-----------------------------------------------------------------------

void ByteStream::Data::unlockDataFreeList()
{
	s_archiveMutex.leave();
}
	
//-----------------------------------------------------------------------

ByteStream::Data *ByteStream::Data::getNewData()
{
	Data *result = 0;
	std::vector<Data *> &dataFreeList = lockDataFreeList();
	if (dataFreeList.empty())
	{
		unlockDataFreeList();
		result = new Data;
	}
	else
	{
		result = dataFreeList.back();
		dataFreeList.pop_back();
		unlockDataFreeList();
	}
	result->refCount = 1;
	return result;
}

//---------------------------------------------------------------------

void ByteStream::Data::releaseOldData(ByteStream::Data *oldData)
{
	assert(reinterpret_cast<uintptr_t>(oldData) != ~uintptr_t(0) / 255 * 0xef);

	if (oldData->size > 4096)
		delete oldData;
	else
	{
		std::vector<Data *> &dataFreeList = lockDataFreeList();

		if (dataFreeList.size() > 256)
			delete oldData;
		else
		{
			oldData->refCount = 0;
			try
			{
				dataFreeList.push_back(oldData);
			}
			catch (...)
			{
				// Pooling is optional; destruction must not fail or retain the lock.
				unlockDataFreeList();
				delete oldData;
				return;
			}
		}

		unlockDataFreeList();
	}
}

//---------------------------------------------------------------------

}//namespace Archive

//---------------------------------------------------------------------
