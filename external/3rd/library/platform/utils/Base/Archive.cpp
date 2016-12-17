////////////////////////////////////////////////////////////////////////////////
//  The author of this code is Justin Randall
//
//  I have made modifications to the ByteStream
//  and AutoByteStream classes in order to make them suitable 
//  for use in messaging systems which require objects that 
//  are copyable and assignable.  It is also desirable for
//  the ByteStream object to use a flexible allocator system
//  that may support multi-threaded programming models.


#include "Archive.h"


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{


////////////////////////////////////////////////////////////////////////////////

    
#if defined(USE_ARCHIVE_MUTEX)
    CMutex ByteStreamMutex;
#endif

#if defined (PASCAL_STRING)
	void get(Base::ByteStream::ReadIterator& source, std::string& target)
	{
		unsigned int size = 0;
		Base::get (source, size);

		const unsigned char *buf            = source.getBuffer();

		target.assign((const char *)buf, (unsigned int)(buf + size));

		const unsigned int readSize                = size * sizeof(char);
		source.advance(readSize);
	}

	void put(ByteStream& target, const std::string& source)
	{
		const unsigned int size = source.size();
		put(target, size);
		target.put(source.data(), size * sizeof (char));
	}

#else
    void get(ByteStream::ReadIterator & source, std::string & target)
    {
	    target = reinterpret_cast<const char *>(source.getBuffer());
	    source.advance(target.length() + 1);
    }

 
    void put(ByteStream & target, const std::string & source)
    {
	    target.put(source.c_str(), source.size()+1);
    }
#endif

    Base::ReadIterator::ReadIterator() :
    readPtr(0),
    stream(0)
    {
    }

    Base::ReadIterator::ReadIterator(const ReadIterator & source) :
    readPtr(source.readPtr),
    stream(source.stream)
    {
    }

    Base::ReadIterator::ReadIterator(const ByteStream & source) :
    readPtr(0),
    stream(&source)
    {
    }

    ByteStream::ReadIterator::~ReadIterator()
    {
	    stream = 0;
    }

    ByteStream::ByteStream() :
    allocatedSize(0),
    beginReadIterator(),
    data(nullptr),
    size(0)
    {
	    data = Data::getNewData();
	    beginReadIterator = ReadIterator(*this);
    }

    ByteStream::ByteStream(const unsigned char * const newBuffer, const unsigned int bufferSize) :
    allocatedSize(bufferSize),
    data(0),
    size(bufferSize)
    {
	    data = Data::getNewData();
	    if(data->size < size)
	    {
		    delete[] data->buffer;
		    data->buffer = new unsigned char[size];
		    data->size = size;
	    }
	    memcpy(data->buffer, newBuffer, size);
	    beginReadIterator = ReadIterator(*this);
    }

    ByteStream::ByteStream(const ByteStream & source):
    allocatedSize(source.getSize()),	// only allocate what is really there, be opportinistic when grow()'ing
    data(source.data),
    size(source.getSize())
    {
	    source.data->ref();
	    beginReadIterator = ReadIterator(*this);
    }

    ByteStream::ByteStream(ReadIterator & source) :
    allocatedSize(0),
    size(0)
    {
	    data = Data::getNewData();
	    put(source.getBuffer(), source.getSize());
	    source.advance(source.getSize());
	    beginReadIterator = ReadIterator(*this);
    }

    ByteStream::~ByteStream()
    {
	    data->deref();
	    allocatedSize = 0;
	    data = 0; //lint !e672 (data deref insures the data is deleted if no one references it)
	    size = 0;
    }

    ByteStream & ByteStream::operator=(const ByteStream & rhs)
    {
	    if(this != &rhs)
	    {
		    data->deref(); // deref local data
		    rhs.data->ref();
		    allocatedSize = rhs.allocatedSize;
		    size = rhs.size;
		    data = rhs.data; //lint !e672 (data is ref counted)
	    }
	    return *this;
    }

    void ByteStream::get(void * target, ReadIterator & readIterator, const unsigned long int targetSize) const
    {
	    assert(readIterator.getReadPosition() + targetSize <= allocatedSize);
	    memcpy(target, &data->buffer[readIterator.getReadPosition()], targetSize);
    }

    void ByteStream::put(const void * const source, const unsigned int sourceSize)
    {
	    if(data->getRef() > 1)
	    {
		    const unsigned char * const tmp = data->buffer;
		    data->deref();
		    data = Data::getNewData();
		    if(data->size < sourceSize)
		    {
			    delete[] data->buffer;
			    data->buffer = new unsigned char[size];
			    data->size = size;
		    }
		    memcpy(data->buffer, tmp, size);
		    allocatedSize = size;		
	    }
	    growToAtLeast(size + sourceSize);
	    memcpy(&data->buffer[size], source, sourceSize);
	    size += sourceSize;
    }

    void ByteStream::reAllocate(const unsigned int newSize)
    {
	    allocatedSize = newSize;
	    if(data->size < allocatedSize)
	    {
		    unsigned char * tmp = new unsigned char[newSize];
		    if(data->buffer != nullptr)
			    memcpy(tmp, data->buffer, size);
		    delete[] data->buffer;
		    data->buffer = tmp;
		    data->size = newSize;
	    }
    }


////////////////////////////////////////////////////////////////////////////////


    AutoByteStream::AutoByteStream() :
    members()
    {
    }

    AutoByteStream::~AutoByteStream()
    {
    }

    void AutoByteStream::addVariable(AutoVariableBase & newVariable)
    {
	    members.push_back(&newVariable);
    }

    const unsigned int AutoByteStream::getItemCount() const
    {
	    return members.size();
    }

    void AutoByteStream::pack(ByteStream & target) const
    {
	    std::vector<AutoVariableBase *>::const_iterator i;
	    unsigned short packedSize=static_cast<unsigned short>(members.size());
	    put(target,packedSize);
	    for(i = members.begin(); i != members.end(); ++i)
	    {
		    (*i)->pack(target);
	    }
    }

    void AutoByteStream::unpack(ByteStream::ReadIterator & source)
    {
	    std::vector<AutoVariableBase *>::iterator i;
	    unsigned short packedSize;
	    get(source,packedSize);
	    for(i = members.begin(); i != members.end(); ++i)
	    {
		    (*i)->unpack(source);
	    }
    }


////////////////////////////////////////////////////////////////////////////////


    AutoVariableBase::AutoVariableBase()
    {
    }

    AutoVariableBase::~AutoVariableBase()
    {
    }


////////////////////////////////////////////////////////////////////////////////

};
#ifdef EXTERNAL_DISTRO
};
#endif

