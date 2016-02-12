#ifndef SOE__SERIALIZE_H
#define SOE__SERIALIZE_H


#include <assert.h>
#include <string>
#include <vector>
#include "Types.h"

#if (defined(WIN32) || defined(linux)) && !defined(NETWORK_BIG_ENDIAN)
	#define BYTE1 0
	#define BYTE2 1
	#define BYTE3 2
	#define BYTE4 3
	#define BYTE5 4
	#define BYTE6 5
	#define BYTE7 6
	#define BYTE8 7
#elif defined(sparc) || defined(NETWORK_BIG_ENDIAN)
	#define BYTE1 7
	#define BYTE2 6
	#define BYTE3 5
	#define BYTE4 4
	#define BYTE5 3
	#define BYTE6 2
	#define BYTE7 1
	#define BYTE8 0
#endif


////////////////////////////////////////////////////////////////////////////////

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif

namespace soe
{

    using namespace Base;//for types
	////////////////////////////////////////////////////////////////////////////////

	
	//	388 ns (50 bytes)
	inline unsigned Write(unsigned char * stream, unsigned size, const unsigned char * data, unsigned dataLen)
	{
		if (size < dataLen)
			return 0;
		memcpy(stream, data, dataLen);
		return dataLen;
	}

	//	353 ns
	inline unsigned Write(unsigned char * stream, unsigned size, bool data)
	{
		if (size < sizeof(uint8))
			return 0;
		stream[BYTE1] = data;
		return sizeof(uint8);
	}

	//	362 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int8 data)
	{
		if (size < sizeof(int8))
			return 0;
		stream[BYTE1] = data;
		return sizeof(int8);
	}

	//  362 ns
	inline unsigned Write(unsigned char * stream, unsigned size, uint8 data)
	{
		if (size < sizeof(uint8))
			return 0;
		stream[BYTE1] = data;
		return sizeof(uint8);
	}

	//	355 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int16 data)
	{
		if (size < sizeof(int16))
			return 0;
		stream[BYTE1] = data&0xff;
		stream[BYTE2] = (data>>8)&0xff;
		return sizeof(int16);
	}

	//	360 ns
	inline unsigned Write(unsigned char * stream, unsigned size, uint16 data)
	{
		if (size < sizeof(uint16))
			return 0;
		stream[BYTE1] = data&0xff;
		stream[BYTE2] = (data>>8)&0xff;
		return sizeof(uint16);
	}

	//	360 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int32 data)
	{
		if (size < sizeof(int32))
			return 0;
		stream[BYTE1] = data&0xff;
		stream[BYTE2] = (data>>8)&0xff;
		stream[BYTE3] = (data>>16)&0xff;
		stream[BYTE4] = (data>>24)&0xff;
		return sizeof(int32);
	}

	//	360 ns
	inline unsigned Write(unsigned char * stream, unsigned size, uint32 data)
	{
		if (size < sizeof(uint32))
			return 0;
		stream[BYTE1] = data&0xff;
		stream[BYTE2] = (data>>8)&0xff;
		stream[BYTE3] = (data>>16)&0xff;
		stream[BYTE4] = (data>>24)&0xff;
		return sizeof(uint32);
	}

	//	370 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int64 data)
	{
		if (size < sizeof(int64))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 low = *(uint32 *)(&data);
		uint32 high = *((uint32 *)&data+1);
#elif defined(sparc)
		uint32 low = *((uint32 *)&data+1);
		uint32 high = *(uint32 *)(&data);
#endif		
		stream[BYTE1] = (unsigned char)low&0xff;
		stream[BYTE2] = (unsigned char)(low>>8)&0xff;
		stream[BYTE3] = (unsigned char)(low>>16)&0xff;
		stream[BYTE4] = (unsigned char)(low>>24)&0xff;
		stream[BYTE5] = (unsigned char)high&0xff;
		stream[BYTE6] = (unsigned char)(high>>8)&0xff;
		stream[BYTE7] = (unsigned char)(high>>16)&0xff;
		stream[BYTE8] = (unsigned char)(high>>24)&0xff;
		return sizeof(int64);
	}

	//	370 ns
	inline unsigned Write(unsigned char * stream, unsigned size, uint64 data)
	{
		if (size < sizeof(uint64))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 low = *(uint32 *)(&data);
		uint32 high = *((uint32 *)&data+1);
#elif defined(sparc)
		uint32 low = *((uint32 *)&data+1);
		uint32 high = *(uint32 *)(&data);
#endif		
		stream[BYTE1] = (unsigned char)low&0xff;
		stream[BYTE2] = (unsigned char)(low>>8)&0xff;
		stream[BYTE3] = (unsigned char)(low>>16)&0xff;
		stream[BYTE4] = (unsigned char)(low>>24)&0xff;
		stream[BYTE5] = (unsigned char)high&0xff;
		stream[BYTE6] = (unsigned char)(high>>8)&0xff;
		stream[BYTE7] = (unsigned char)(high>>16)&0xff;
		stream[BYTE8] = (unsigned char)(high>>24)&0xff;
		return sizeof(uint64);
	}

	//	360 ns
	inline unsigned Write(unsigned char * stream, unsigned size, float data)
	{
		uint32 & dataRef = *(uint32 *)(&data);
		if (size < sizeof(float))
			return 0;
		stream[BYTE1] = dataRef&0xff;
		stream[BYTE2] = (dataRef>>8)&0xff;
		stream[BYTE3] = (dataRef>>16)&0xff;
		stream[BYTE4] = (dataRef>>24)&0xff;
		return sizeof(float);
	}

	//	370 ns
	inline unsigned Write(unsigned char * stream, unsigned size, double data)
	{
		if (size < sizeof(double))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 low = *(uint32 *)(&data);
		uint32 high = *((uint32 *)&data+1);
#elif defined(sparc)
		uint32 low = *((uint32 *)&data+1);
		uint32 high = *(uint32 *)(&data);
#endif		
		stream[BYTE1] = (unsigned char)low&0xff;
		stream[BYTE2] = (unsigned char)(low>>8)&0xff;
		stream[BYTE3] = (unsigned char)(low>>16)&0xff;
		stream[BYTE4] = (unsigned char)(low>>24)&0xff;
		stream[BYTE5] = (unsigned char)high&0xff;
		stream[BYTE6] = (unsigned char)(high>>8)&0xff;
		stream[BYTE7] = (unsigned char)(high>>16)&0xff;
		stream[BYTE8] = (unsigned char)(high>>24)&0xff;
		return sizeof(double);
	}

	//	361 ns
	inline unsigned WriteEncoded(unsigned char * stream, unsigned size, uint32 data)
	{
		unsigned encoded = ((data & 0xffffffc0) << 2) | (data & 0x3f);
		if (data > 0x3fffffff)
		{
			return 0;
		}
		else if (data > 0x3fffff && size >= 4)
		{
			encoded |= 0xc0;
			stream[BYTE1] = encoded & 0xff;
			stream[BYTE2] = (encoded>>8) & 0xff;
			stream[BYTE3] = (encoded>>16) & 0xff;
			stream[BYTE4] = (encoded>>24) & 0xff;
			return 4;
		}
		else if (data > 0x3fff)
		{
			encoded |= 0x80;
			stream[BYTE1] = encoded & 0xff;
			stream[BYTE2] = (encoded>>8) & 0xff;
			stream[BYTE3] = (encoded>>16) & 0xff;
			return 3;
		}
		else if (data > 0x3f)
		{
			encoded |= 0x40;
			stream[BYTE1] = encoded & 0xff;
			stream[BYTE2] = (encoded>>8) & 0xff;
			return 2;
		}
		else if (size >= 1)
		{
			stream[BYTE1] = encoded & 0xff;
			return 1;
		}
		return 0;		
	}

	//	630 ns
	inline unsigned Write(unsigned char * stream, unsigned size, const std::string & data, bool isPascal=true)
	{
        if (isPascal)
        {
		    unsigned fieldLen = 0;
		    unsigned bytes = 0;
		    fieldLen = Write(stream, size, static_cast<unsigned>(data.length()));
		    if (!fieldLen)
			    return 0;
            bytes += fieldLen;
            if (data.length())
            {
		        fieldLen = Write(stream+bytes, size-bytes, reinterpret_cast<const unsigned char *>(data.data()), data.length());
		        if (!fieldLen)
			        return 0;
                bytes += fieldLen;
            }
		    return bytes;
        }
        else
        {
            unsigned bytes = Write(stream, size, reinterpret_cast<const unsigned char *>(data.c_str()), data.size()+1);
            if (!bytes)
                return 0;
            return bytes;
        }
	}

	template<class T>
	inline unsigned Write(unsigned char * stream, unsigned size, const std::vector<T> & data)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		bytes += Write(stream, size, static_cast<unsigned>(data.size()));
		if (!bytes)
			return 0;
		for (typename std::vector<T>::const_iterator iter = data.begin(); iter != data.end(); iter++)
		{
			elementBytes = Write(stream+bytes, size-bytes, *iter);
			if (!elementBytes)
				return 0;
			bytes += elementBytes;
		}

		return bytes;
	}

	inline unsigned Read(const unsigned char * stream, unsigned size, unsigned char * data, unsigned dataLen)
	{
		if (size < dataLen)
			return 0;
		memcpy(data, stream, dataLen);
		return dataLen;
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, bool & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(uint8))
			return 0;
		data = stream[BYTE1] != 0;
		return sizeof(uint8);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int8 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(int8))
			return 0;
		data = stream[BYTE1];
		return sizeof(int8);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, uint8 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(uint8))
			return 0;
		data = stream[BYTE1];
		return sizeof(uint8);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int16 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(int16))
			return 0;
		data = stream[BYTE1];
		data |= stream[BYTE2]<<8;
		return sizeof(int16);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, uint16 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(uint16))
			return 0;
		data = stream[BYTE1];
		data |= stream[BYTE2]<<8;
		return sizeof(uint16);
	}

	//	360 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int32 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(int32))
			return 0;
		data = stream[BYTE1];
		data |= stream[BYTE2]<<8;
		data |= stream[BYTE3]<<16;
		data |= stream[BYTE4]<<24;
		return sizeof(int32);
	}

	//	360 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, uint32 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(uint32))
			return 0;
		data = stream[BYTE1];
		data |= stream[BYTE2]<<8;
		data |= stream[BYTE3]<<16;
		data |= stream[BYTE4]<<24;
		return sizeof(uint32);
	}

	//	380 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int64 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(int64))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 & low = *(uint32 *)(&data);
		uint32 & high = *((uint32 *)&data+1);
#elif defined(sparc)
		uint32 & low = *(uint32 *)(&data+1);
		uint32 & high = *(uint32 *)(&data);
#endif		
		low = stream[BYTE1];
		low |= stream[BYTE2]<<8;
		low |= stream[BYTE3]<<16;
		low |= stream[BYTE4]<<24;
		high = stream[BYTE5];
		high |= stream[BYTE6]<<8;
		high |= stream[BYTE7]<<16;
		high |= stream[BYTE8]<<24;
		return sizeof(int64);
	}

	//	380 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, uint64 & data, unsigned = 1/*unused*/)
	{
		if (size < sizeof(uint64))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 & low = *(uint32 *)(&data);
		uint32 & high = *((uint32 *)&data+1);
#elif defined(sparc)
		uint32 & low = *(uint32 *)(&data+1);
		uint32 & high = *(uint32 *)(&data);
#endif		
		low = stream[BYTE1];
		low |= stream[BYTE2]<<8;
		low |= stream[BYTE3]<<16;
		low |= stream[BYTE4]<<24;
		high = stream[BYTE5];
		high |= stream[BYTE6]<<8;
		high |= stream[BYTE7]<<16;
		high |= stream[BYTE8]<<24;
		return sizeof(uint64);
	}

	//	370 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, float & data, unsigned = 1/*unused*/)
	{
		assert(sizeof(float) == 4);
		uint32 & dataRef = *(uint32 *)(&data);
		if (size < sizeof(float))
			return 0;
		dataRef = stream[BYTE1];
		dataRef |= stream[BYTE2]<<8;
		dataRef |= stream[BYTE3]<<16;
		dataRef |= stream[BYTE4]<<24;
		return sizeof(float);
	}

	//	380 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, double & data, unsigned = 1/*unused*/)
	{
		assert(sizeof(double) == 8);
		if (size < sizeof(double))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 & low = *(uint32 *)(&data);
		uint32 & high = *((uint32 *)&data+1);
#elif defined(sparc)
		uint32 & low = *(uint32 *)(&data+1);
		uint32 & high = *(uint32 *)(&data);
#endif		
		low = stream[BYTE1];
		low |= stream[BYTE2]<<8;
		low |= stream[BYTE3]<<16;
		low |= stream[BYTE4]<<24;
		high = stream[BYTE5];
		high |= stream[BYTE6]<<8;
		high |= stream[BYTE7]<<16;
		high |= stream[BYTE8]<<24;
		return sizeof(double);
	}

	//	360 ns
	inline unsigned ReadEncoded(const unsigned char * stream, unsigned size, uint32 & data)
	{
		if (size < 1)
			return 0;

		unsigned encoded = stream[BYTE1];
		unsigned mask = encoded & 0xc0;
		if (mask == 0)
		{
			data = encoded & 0x3f;
			return 1;
		}
		else if (mask == 0x40 && size >=2)
		{
			data = (stream[BYTE2]<<6) | (encoded & 0x3f);
			return 2;
		}
		else if (mask == 0x80 && size >=3)
		{
			data = (stream[BYTE3]<<14) | (stream[BYTE2]<<6) | (encoded & 0x3f);
			return 3;
		}
		else if (mask == 0xc0 && size >=4)
		{
			data = (stream[BYTE4]<<22) | (stream[BYTE3]<<14) | (stream[BYTE2]<<6) | (encoded & 0x3f);
			return 4;
		}
		return 0;		
	}

	inline unsigned Read(const unsigned char * stream, unsigned size, std::string & data, unsigned maxLen, bool isPascal=true)
	{
        if (isPascal)
        {
		    unsigned bytes = 0;
		    unsigned length = 0;
		    bytes += Read(stream, size, length);
		    if (!bytes || size < bytes+length || length > maxLen)
			    return 0;
		    data.assign((const char *)stream+bytes, (const char *)stream+bytes+length);  // assign would be dangerous if unicode, single-byte characters are ok
		    return bytes + length;
        }
        else
        {
            //find nullptr terminator, if don't find it before maxLen, then err
            const unsigned char *strEnd = nullptr;
            unsigned strLen = 0;
            for (;strLen<maxLen && strLen < size;strLen++)
            {
                if (stream[strLen] == 0)
                {
                    //found it
                    strEnd = (stream+strLen);
                    break;
                }
            }
            if (strEnd == nullptr)
                return 0;

            data.assign((const char *)stream, (const char *)strEnd);
            return strLen+1;
        }
	}

	template<class T>
	inline unsigned Read(const unsigned char * stream, unsigned size, std::vector<T> & data, unsigned maxVecLen, unsigned maxFieldLen = 1)
	{
		T element;
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		unsigned length = 0;
		data.clear();
		bytes += Read(stream, size, length);
		if (!bytes || length > maxVecLen)
			return 0;
		for (unsigned i=0; i<length; i++)
		{
			elementBytes = Read(stream+bytes, size-bytes, element, maxFieldLen);
			if (!elementBytes)
				return 0;
			data.push_back(element);
			bytes += elementBytes;
		}
		return bytes;
	}
	////////////////////////////////////////////////////////////////////////////////

	class AutoVariableBase
	{
        public:
			AutoVariableBase() {}
			virtual ~AutoVariableBase() {}

			virtual unsigned Write(unsigned char * stream, unsigned size) const = 0;
			virtual unsigned Read(const unsigned char * stream, unsigned size, unsigned maxLen=1) = 0;
	};
    
	template<class ValueType>
	class AutoVariable : public AutoVariableBase
    {
		public:
			AutoVariable() : AutoVariableBase(), mValue() {}
			explicit AutoVariable(const ValueType & source) : AutoVariableBase(), mValue(source) {}
			AutoVariable(const AutoVariable & copy) : AutoVariableBase(), mValue(copy.mValue) {}
			virtual ~AutoVariable() {}

			const ValueType &	Get() const { return mValue; }
			void				Set(const ValueType & rhs) { mValue = rhs; }
			virtual unsigned	Write(unsigned char * stream, unsigned size) const { return soe::Write(stream, size, mValue); }
			virtual unsigned	Read(const unsigned char * stream, unsigned size, unsigned maxLen) { return soe::Read(stream, size, mValue, maxLen); }

		private:
			ValueType			mValue;
    };


	////////////////////////////////////////////////////////////////////////////////


}
#ifdef EXTERNAL_DISTRO
};
#endif

#endif

