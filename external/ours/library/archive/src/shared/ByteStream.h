#ifndef	_ByteStream_H
#define	_ByteStream_H

//---------------------------------------------------------------------
#if WIN32
#pragma warning ( disable : 4786 ) // dentifier was truncated to '255' characters in the debug information
#pragma warning ( disable : 4514 ) // inlined function has been removed
#endif

#include <exception>
#include <vector>

//---------------------------------------------------------------------

namespace Archive
{



//---------------------------------------------------------------------

class ReadException : public std::exception
{
public:
	explicit ReadException(const char * const what);
	ReadException(const ReadException & source);
	ReadException &  operator = (const ReadException & rhs);
	~ReadException() throw();
	
	const char * what() const throw();
	
private:
	const char * m_what;
};

//---------------------------------------------------------------------

inline ReadException::ReadException(const char * what) :
m_what(what)
{
}

//---------------------------------------------------------------------

inline ReadException::ReadException(const ReadException & source) :
m_what(source.m_what)
{
}

//---------------------------------------------------------------------

inline ReadException::~ReadException() throw()
{
}

//---------------------------------------------------------------------

inline ReadException & ReadException::operator = (const ReadException & rhs)
{
	m_what = rhs.m_what;
	return *this;
}

//---------------------------------------------------------------------

inline const char * ReadException::what() const throw()
{
	return m_what;
}

class ByteStream;

class ReadIterator
{
public:
		                        ReadIterator    ();
		                        ReadIterator    (const ReadIterator & source);
	explicit                    ReadIterator    (const ByteStream & source);
		                        ~ReadIterator   ();

	ReadIterator &              operator =      (const ReadIterator & source);
	bool                        operator ==     (const ReadIterator & other) const;
	bool                        operator !=     (const ReadIterator & other) const;
	void                        advance         (const unsigned int distance);
	void                        get             (void * target, const unsigned long int readSize);
	const unsigned int          getSize         () const;
	const unsigned char * const getBuffer       () const;
	const unsigned int          getReadPosition () const;
private:
	
	unsigned int readPtr;
	const ByteStream *     stream;
};

inline bool ReadIterator::operator != (const ReadIterator &other) const
{
	return !(*this == other);
}

//---------------------------------------------------------------------
/** \class ByteStream ByteStream.h "Archive/ByteStream.h"
	@brief A byte ByteStream
*/
class ByteStream
{
public: // ctor/dtors
	ByteStream();
	ByteStream(const unsigned char * const buffer, const unsigned int bufferSize);
	ByteStream(const ByteStream & source);
	virtual ~ByteStream();

	typedef Archive::ReadIterator ReadIterator;

public: // inner classes
	/** \class ReadIterator ByteStream.h "Archive/ByteStream.h"
		@brief an iterator object used to retrieve data from a ByteStream

		The ReadIterator is employed to coordinate multiple consumers of 
		a ByteStream object without altering the ByteStream itself. It is a const-like
		iterator that will not change the contents of a ByteStream. Multiple ByteStream
		consumers may access data in the ByteStream using ReadIterators.

		@author Justin Randall
	*/

public:
	ByteStream(ReadIterator & source);
	ByteStream &                operator = (const ByteStream & source);
	ByteStream &                operator = (ReadIterator & source);
	const ReadIterator &        begin() const;
	const ReadIterator          end() const;
	void                        clear();
	const unsigned char * const getBuffer() const;
	const unsigned int          getSize() const;
	void                        put(const void * const source, const unsigned int sourceSize);
	void                        setAllocatedSizeLimit(unsigned int limit);

private:
	void                        get(void * target, ReadIterator & readIterator, const unsigned long int readSize) const;
	void                        growToAtLeast(const unsigned int targetSize);
	void                        reAllocate(const unsigned int newSize);

private: // inner classes
	class Data
	{
	public:
		~Data();
		
		static Data * getNewData();

		const int getRef () const;
		void      deref  ();
		void      ref    ();
	protected:
		friend class ByteStream;
		friend class Archive::ReadIterator;
		unsigned char * buffer;
		unsigned long   size;
	private:
		struct DataFreeList
		{
			~DataFreeList()
			{
				std::vector<ByteStream::Data *>::iterator i;
				for(i = freeList.begin(); i != freeList.end(); ++i)
				{
					delete (*i);
				}
			};
			std::vector<Data *> freeList;
		};
		Data();
//		explicit Data(unsigned char * buffer);
		static std::vector<Data *> & lockDataFreeList();
		static void unlockDataFreeList();
		static void releaseOldData(Data * oldData);

	private:
		int             refCount;
	};

private:
	friend class Archive::ReadIterator;
	unsigned int                allocatedSize;
	unsigned int                allocatedSizeLimit;
	ReadIterator                beginReadIterator;
	Data *                      data;
	unsigned int                size;
}; //lint !e1934

//---------------------------------------------------------------------

inline ByteStream::Data::Data() :
buffer(0),
size(0),
refCount(1)
{
}

//---------------------------------------------------------------------
/*
inline ByteStream::Data::Data(unsigned char * newBuffer) :
buffer(newBuffer),
refCount(1)
{
}
*/
//---------------------------------------------------------------------

inline ByteStream::Data::~Data()
{
	refCount = 0;
	delete[] buffer;
}

//---------------------------------------------------------------------

inline void ByteStream::Data::deref()
{
	refCount--;
	if(refCount < 1)
		releaseOldData(this);
//		delete this;

}

//-----------------------------------------------------------------------

inline const int ByteStream::Data::getRef() const
{
	return refCount;
}

//---------------------------------------------------------------------

inline void ByteStream::Data::ref() 
{
	refCount++;
}

//-----------------------------------------------------------------------

inline ReadIterator & ReadIterator::operator = (const ReadIterator & rhs)
{
	if(&rhs != this)
	{
		readPtr = rhs.readPtr;
		stream = rhs.stream;
	}
	return *this;
}

//---------------------------------------------------------------------

inline void ReadIterator::get(void * target, const unsigned long int readSize)
{
	if(stream)
	{
		stream->get(target, *this, readSize);
		readPtr += readSize;
	}
	else
	{
		static const char * const desc = "Archive::ReadIterator::get - read operation on nullptr stream object";
		ReadException ex(desc);
		throw (ex);
	}
}

//---------------------------------------------------------------------

inline const unsigned int ReadIterator::getSize() const
{
	if(stream)
		return stream->getSize() - readPtr;
	return 0;
}

//---------------------------------------------------------------------

inline const ReadIterator & ByteStream::begin() const
{
	return beginReadIterator;
}

//---------------------------------------------------------------------

inline const ReadIterator ByteStream::end() const
{
	ReadIterator result(*this);
	result.advance(size);
	return result;
}

//---------------------------------------------------------------------

inline void ReadIterator::advance(const unsigned int distance)
{
	readPtr += distance;
}

//---------------------------------------------------------------------

inline const unsigned int ReadIterator::getReadPosition() const
{
	return readPtr;
}

//---------------------------------------------------------------------

inline const unsigned char * const ReadIterator::getBuffer() const
{
	if(stream && stream->data)
		return &stream->data->buffer[readPtr];

	return 0;
}

//---------------------------------------------------------------------
/**
	@brief clear read and write settings for the ByteStream

	This does not free allocated memory. Instead, it keeps the ByteStream
	buffer and sets the read and write pointers back to the beginning
	of the ByteStream

	@author Justin Randall
*/
inline void ByteStream::clear() 
{
	size = 0;

	if ((allocatedSizeLimit) && (allocatedSize > allocatedSizeLimit))
	{
		if (data)
		{
			data->deref();
			data = 0;
		}
		allocatedSize = 0;

		reAllocate(allocatedSizeLimit);
	}
}

//---------------------------------------------------------------------
/**
	@brief get the whole ByteStream buffer (very const)
*/
inline const unsigned char * const ByteStream::getBuffer() const
{
	if (data)
		return data->buffer;
	return 0;
}

//---------------------------------------------------------------------
/**
	@brief Get the size, in bytes, of the ByteStream buffer

	@return the size, in bytes, of the ByteStream buffer

	@author Justin Randall
*/
inline const unsigned int ByteStream::getSize() const
{
	return size;
}

//---------------------------------------------------------------------
/**
	This private method ensures that the ByteStream buffer is large enough
	to receive data at least as large as the requested targetSize.

	@param unsigned int targetSize   The desired size to increase the
	                                 ByteStream buffer.

	@author Justin Randall
*/

inline void ByteStream::growToAtLeast(const unsigned int targetSize)
{
	if(allocatedSize < targetSize)
	{
		if(allocatedSize < 4096)
		{
			reAllocate(allocatedSize + allocatedSize + targetSize);
		}
		else
		{
			reAllocate(allocatedSize + targetSize);
		}
	}
}

//---------------------------------------------------------------------

inline void ByteStream::setAllocatedSizeLimit(const unsigned int limit)
{
	allocatedSizeLimit = limit;

	if ((allocatedSizeLimit) && (allocatedSize < allocatedSizeLimit))
		reAllocate(allocatedSizeLimit);
}

} // namespace Archive
//---------------------------------------------------------------------

#endif	// _ByteStream_H
