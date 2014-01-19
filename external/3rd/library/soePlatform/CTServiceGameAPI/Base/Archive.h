////////////////////////////////////////////////////////////////////////////////
//  The author of this code is Justin Randall
//
//  I have made modifications to the ByteStream
//  and AutoByteStream classes in order to make them suitable 
//  for use in messaging systems which require objects that 
//  are copyable and assignable.  It is also desirable for
//  the ByteStream object to use a flexible allocator system
//  that may support multi-threaded programming models.


#ifndef	BASE_ARCHIVE_H
#define	BASE_ARCHIVE_H


#include <assert.h>
#include <string>
#include <vector>
#include "Platform.h"


//#if defined _MT || defined _REENTRANT
//#   define USE_ARCHIVE_MUTEX
//#	include "Mutex.h"
//#endif

#ifdef WIN32
#   include "win32/Archive.h"
#elif linux
#   include "linux/Archive.h"
#elif sparc
#   include "solaris/Archive.h"
#else
    #error /Base/Archive.h: Undefine platform type
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

const unsigned MAX_ARRAY_SIZE = 1024;


////////////////////////////////////////////////////////////////////////////////

#if defined(USE_ARCHIVE_MUTEX)
    extern CMutex ByteStreamMutex;
#endif

    class ByteStream
    {
        public:
	        class ReadIterator
	        {
	            public:
		            ReadIterator();
		            ReadIterator(const ReadIterator & source);
		            explicit ReadIterator(const ByteStream & source);
		            ~ReadIterator();

		            ReadIterator &              operator =      (const ReadIterator & source);
		            void                        advance         (const unsigned int distance);
		            void                        get             (void * target, const unsigned long int readSize);
		            const unsigned int          getSize         () const;
		            const unsigned char * const getBuffer       () const;
		            const unsigned int          getReadPosition () const;

                private:
		            unsigned int readPtr;
		            const ByteStream *     stream;
	        };

        private: 
	        class Data
	        {
		        friend class ByteStream;
		        friend class ReadIterator;
	            public:
		            ~Data();
		            
		            static Data * getNewData();

		            const int getRef () const;
		            void      deref  ();
		            void      ref    ();
	            protected:
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
                    //explicit Data(unsigned char * buffer);
		            static std::vector<Data *> & getDataFreeList();
		            static void   releaseOldData(Data * oldData);
	            private:
		            int             refCount;
	        };

        friend class ReadIterator;
        public:
	        ByteStream();
	        ByteStream(const unsigned char * const buffer, const unsigned int bufferSize);
	        ByteStream(const ByteStream & source);
	        virtual ~ByteStream();

        public:
	        ByteStream(ReadIterator & source);
	        ByteStream &                operator = (const ByteStream & source);
	        ByteStream &                operator = (ReadIterator & source);
	        const ReadIterator &        begin() const;
	        void                        clear();
	        const unsigned char * const getBuffer() const;
	        const unsigned int          getSize() const;
	        void                        put(const void * const source, const unsigned int sourceSize);
	        bool                        overwriteEnd(const void * const source, const unsigned int sourceSize);

        private:
	        void                        get(void * target, ReadIterator & readIterator, const unsigned long int readSize) const;
	        void                        growToAtLeast(const unsigned int targetSize);
	        void                        reAllocate(const unsigned int newSize);

        private:
	        unsigned int                allocatedSize;
	        ReadIterator                beginReadIterator;
	        Data *                      data;
	        unsigned int                size;
			unsigned int				lastPutSize;

    };

    inline ByteStream::Data::Data() :
        buffer(0),
        size(0),
        refCount(1)
    {
    }

    inline ByteStream::Data::~Data()
    {
#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Lock();
#endif

        refCount = 0;
	    delete[] buffer;

#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Unlock();
#endif
    }

    inline void ByteStream::Data::deref()
    {
#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Lock();
#endif

        refCount--;

#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Unlock();
#endif

	    if(refCount < 1)
		    releaseOldData(this);
    }

    inline std::vector<ByteStream::Data *> & ByteStream::Data::getDataFreeList()
    {
	    static DataFreeList freeList;
	    return freeList.freeList;
    }

    inline ByteStream::Data * ByteStream::Data::getNewData()
    {
#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Lock();
#endif

        Data * result = 0;
	    if(getDataFreeList().empty())
	    {
		    result = new Data;
	    }
	    else
	    {
		    result = getDataFreeList().back();
		    getDataFreeList().pop_back();
	    }
	    result->refCount = 1;

#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Unlock();
#endif

	    return result;
    }

    inline const int ByteStream::Data::getRef() const
    {
	    return refCount;
    }

    inline void ByteStream::Data::ref() 
    {
#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Lock();
#endif

        refCount++;

#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Unlock();
#endif
    }

    inline void ByteStream::Data::releaseOldData(ByteStream::Data * oldData)
    {
#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Lock();
#endif

        getDataFreeList().push_back(oldData);
	    oldData->refCount = 0;

#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Unlock();
#endif
    }

    inline ByteStream::ReadIterator & ByteStream::ReadIterator::operator = (const ByteStream::ReadIterator & rhs)
    {
	    if(&rhs != this)
	    {
		    readPtr = rhs.readPtr;
		    stream = rhs.stream;
	    }
	    return *this;
    }

    inline void ByteStream::ReadIterator::get(void * target, const unsigned long int readSize)
    {
	    assert(stream);
	    stream->get(target, *this, readSize);
	    readPtr += readSize;
    }

    inline const unsigned int ByteStream::ReadIterator::getSize() const
    {
	    assert(stream);
	    return stream->getSize() - readPtr;
    }

    inline const ByteStream::ReadIterator & ByteStream::begin() const
    {
	    return beginReadIterator;
    }

    inline void ByteStream::ReadIterator::advance(const unsigned int distance)
    {
	    readPtr += distance;
    }

    inline const unsigned int ByteStream::ReadIterator::getReadPosition() const
    {
	    return readPtr;
    }

    inline const unsigned char * const ByteStream::ReadIterator::getBuffer() const
    {
	    if(stream)
		    return &stream->data->buffer[readPtr];

	    return 0;
    }

    inline void ByteStream::clear() 
    {
#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Lock();
#endif

        size = 0;

#if defined(USE_ARCHIVE_MUTEX)
        ByteStreamMutex.Unlock();
#endif
    }

    inline const unsigned char * const ByteStream::getBuffer() const
    {
	    return data->buffer;
    }

    inline const unsigned int ByteStream::getSize() const
    {
	    return size;
    }

    inline void ByteStream::growToAtLeast(const unsigned int targetSize)
    {
	    if(allocatedSize < targetSize)
	    {
		    reAllocate(allocatedSize + allocatedSize + targetSize);
	    }
    }


////////////////////////////////////////////////////////////////////////////////

    
    inline void get(ByteStream::ReadIterator & source, ByteStream & target)
    {
        target.put(source.getBuffer(), source.getSize());
        source.advance(source.getSize());
    }

    inline void get(ByteStream::ReadIterator & source, double & target)
    {
        source.get(&target, 8);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, float & target)
    {
        source.get(&target, 4);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, uint64 & target)
    {
        source.get(&target, 8);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, int64 & target)
    {
        source.get(&target, 8);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, uint32 & target)
    {
        source.get(&target, 4);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, int32 & target)
    {
        source.get(&target, 4);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, uint16 & target)
    {
        source.get(&target, 2);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, int16 & target)
    {
        source.get(&target, 2);
        target = byteSwap(target);
    }

    inline void get(ByteStream::ReadIterator & source, uint8 & target)
    {
        source.get(&target, 1);
    }

    inline void get(ByteStream::ReadIterator & source, int8 & target)
    {
        source.get(&target, 1);
    }

    inline void get(ByteStream::ReadIterator & source, unsigned char * const target, const unsigned int targetSize)
    {
        source.get(target, targetSize);
    }

    inline void get(ByteStream::ReadIterator & source, bool & target)
    {
        source.get(&target, 1);
    }


////////////////////////////////////////////////////////////////////////////////


    inline void put(ByteStream & target, ByteStream::ReadIterator & source)
    {
        target.put(source.getBuffer(), source.getSize());
        source.advance(source.getSize());
    }

    inline void put(ByteStream & target, const double value)
    {
        double temp = byteSwap(value);
        target.put(&temp, 8);
    }

    inline void put(ByteStream & target, const float value)
    {
        float temp = byteSwap(value);
        target.put(&temp, 4);
    }

    inline void put(ByteStream & target, const uint64 value)
    {
        uint64 temp = byteSwap(value);
        target.put(&temp, 8);
    }

    inline void put(ByteStream & target, const int64 value)
    {
        int64 temp = byteSwap(value);
        target.put(&temp, 8);
    }

    inline void put(ByteStream & target, const uint32 value)
    {
        uint32 temp = byteSwap(value);
        target.put(&temp, 4);
    }

    inline void put(ByteStream & target, const int32 value)
    {
        int32 temp = byteSwap(value);
        target.put(&temp, 4);
    }

    inline void put(ByteStream & target, const uint16 value)
    {
        uint16 temp = byteSwap(value);
        target.put(&temp, 2);
    }

    inline void put(ByteStream & target, const int16 value)
    {
        int16 temp = byteSwap(value);
        target.put(&temp, 2);
    }

    inline void put(ByteStream & target, const uint8 value)
    {
        target.put(&value, 1);
    }

    inline void put(ByteStream & target, const int8 value)
    {
        target.put(&value, 1);
    }

    inline void put(ByteStream & target, const bool & source)
    {
        target.put(&source, 1);
    }


    inline void put(ByteStream & target, const unsigned char * const source, const unsigned int sourceSize)
    {
        target.put(source, sourceSize);
    }

    inline void put(ByteStream & target, const ByteStream & source)
    {
        target.put(source.begin().getBuffer(), source.begin().getSize());
    }

    void get(ByteStream::ReadIterator & source, std::string & target);
    void put(ByteStream & target, const std::string & source);

    
////////////////////////////////////////////////////////////////////////////////


    class AutoVariableBase
    {
        public:
	        AutoVariableBase();
	        virtual ~AutoVariableBase();

            virtual void     pack(ByteStream & target) const = 0;
	        virtual void     unpack(ByteStream::ReadIterator & source) = 0;
    };


////////////////////////////////////////////////////////////////////////////////


    class AutoByteStream 
    {
        public:
	        AutoByteStream();
	        virtual ~AutoByteStream();
	        void                 addVariable(AutoVariableBase & newVariable);
	        virtual const unsigned int getItemCount() const;
	        virtual void         pack(ByteStream & target) const;
	        virtual void         unpack(ByteStream::ReadIterator & source);
        protected:
	        std::vector<AutoVariableBase *>	members;
        private:
	        AutoByteStream(const AutoByteStream & source);
    };


////////////////////////////////////////////////////////////////////////////////


    template<class ValueType>
    class AutoVariable : public AutoVariableBase
    {
    public:
	    AutoVariable();
	    explicit AutoVariable(const ValueType & source);
	    virtual ~AutoVariable();

	    const ValueType & get() const;
	    virtual void      pack(ByteStream & target) const;
	    void              set(const ValueType & rhs);
	    virtual void      unpack(ByteStream::ReadIterator & source);

    private:
	    ValueType        value;
    };

    template<class ValueType>
    AutoVariable<ValueType>::AutoVariable() :
        AutoVariableBase(),
        value()
    {
    }

    template<class ValueType>
    AutoVariable<ValueType>::AutoVariable(const ValueType & source) :
        AutoVariableBase(),
        value(source)
    {
    }

    template<class ValueType>
    AutoVariable<ValueType>::~AutoVariable()
    {
    }

    template<class ValueType>
    const ValueType & AutoVariable<ValueType>::get() const
    {
	    return value;
    }

    template<class ValueType>
    void AutoVariable<ValueType>::pack(ByteStream & target) const
    {
	    Base::put(target, value);
    }

    template<class ValueType>
    void AutoVariable<ValueType>::set(const ValueType & rhs)
    {
	    value = rhs;
    }

    template<class ValueType>
    void AutoVariable<ValueType>::unpack(ByteStream::ReadIterator & source)
    {
        Base::get(source, value);
    }


////////////////////////////////////////////////////////////////////////////////


    template<class ValueType>
    class AutoArray : public AutoVariableBase
    {
    public:
	    AutoArray();
	    AutoArray(const AutoArray & source);
	    ~AutoArray();

	    const std::vector<ValueType> & get() const;
	    void  set(const std::vector<ValueType> & source);
	    
	    virtual void pack(ByteStream & target) const;
	    virtual void unpack(ByteStream::ReadIterator & source);

    private:
	    std::vector<ValueType>	array;
    };

    template<class ValueType>
    inline AutoArray<ValueType>::AutoArray() 
    {
    }

    template<class ValueType>
    inline AutoArray<ValueType>::AutoArray(const AutoArray & source) :
        array(source.array)
    {
    }

    template<class ValueType>
    inline AutoArray<ValueType>::~AutoArray()
    {
    }

    template<class ValueType>
    inline const std::vector<ValueType> & AutoArray<ValueType>::get() const
    {
	    return array;
    }

    template<class ValueType>
    inline void AutoArray<ValueType>::set(const std::vector<ValueType> & source)
    {
	    array = source;
    }

    template<class ValueType>
    inline void AutoArray<ValueType>::pack(ByteStream & target) const
    {
	    unsigned int arraySize = array.size();
	    Base::put(target, arraySize);

	    typename std::vector<ValueType>::const_iterator i;
	    for(i = array.begin(); i != array.end(); ++i)
	    {
		    ValueType v = (*i);
		    Base::put(target, v);
	    }
    }

    template<class ValueType>
    inline void AutoArray<ValueType>::unpack(ByteStream::ReadIterator & source) 
    {
	    unsigned int arraySize;
	    Base::get(source, arraySize);
	    ValueType v;

        if (arraySize > MAX_ARRAY_SIZE)
            arraySize = 0;

	    for(unsigned int i = 0; i < arraySize; ++i)
	    {
		    Base::get(source, v);
		    array.push_back(v);
	    }
    }	


////////////////////////////////////////////////////////////////////////////////


    inline bool overwriteEnd(ByteStream & target, const double value)
    {
        double temp = byteSwap(value);
        return target.overwriteEnd(&temp, 8);
    }

    inline bool overwriteEnd(ByteStream & target, const float value)
    {
        float temp = byteSwap(value);
        return target.overwriteEnd(&temp, 4);
    }

    inline bool overwriteEnd(ByteStream & target, const uint64 value)
    {
        uint64 temp = byteSwap(value);
        return target.overwriteEnd(&temp, 8);
    }

    inline bool overwriteEnd(ByteStream & target, const int64 value)
    {
        int64 temp = byteSwap(value);
        return target.overwriteEnd(&temp, 8);
    }

    inline bool overwriteEnd(ByteStream & target, const uint32 value)
    {
        uint32 temp = byteSwap(value);
        return target.overwriteEnd(&temp, 4);
    }

    inline bool overwriteEnd(ByteStream & target, const int32 value)
    {
        int32 temp = byteSwap(value);
        return target.overwriteEnd(&temp, 4);
    }

    inline bool overwriteEnd(ByteStream & target, const uint16 value)
    {
        uint16 temp = byteSwap(value);
        return target.overwriteEnd(&temp, 2);
    }

    inline bool overwriteEnd(ByteStream & target, const int16 value)
    {
        int16 temp = byteSwap(value);
        return target.overwriteEnd(&temp, 2);
    }

    inline bool overwriteEnd(ByteStream & target, const uint8 value)
    {
        return target.overwriteEnd(&value, 1);
    }

    inline bool overwriteEnd(ByteStream & target, const int8 value)
    {
        return target.overwriteEnd(&value, 1);
    }

    inline bool overwriteEnd(ByteStream & target, const bool & source)
    {
	    return target.overwriteEnd(&source, 1);
    }

    inline bool overwriteEnd(ByteStream & target, const unsigned char * const source, const unsigned int sourceSize)
    {
	    return target.overwriteEnd(source, sourceSize);
    }

////////////////////////////////////////////////////////////////////////////////


};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif	
