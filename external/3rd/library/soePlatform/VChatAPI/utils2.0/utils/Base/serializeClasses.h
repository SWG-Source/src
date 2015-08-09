#ifndef SOE__SERIALIZE_CLASSES_H
#define SOE__SERIALIZE_CLASSES_H


#include <cassert>
#include <cstring>
#include <string>
#include <list>
#include <vector>

#ifdef PRINTABLE_MESSAGES

#	include <stdio.h>
#	include <time.h>

#	if defined(WIN32) && !defined(snprintf)
#		define snprintf _snprintf
#	endif

#endif

#ifdef TRACK_READ_WRITE_FAILURES
#	include "trackMessageFailures.h"
#	define  MESSAGE_FAILURE_TRACE_GENERIC	\
			soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to transmit, size = %u", __FUNCTION__, __FILE__, __LINE__, size))
#	define  MESSAGE_FAILURE_TRACE_LABEL(__label__)	\
			soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to transmit %s, size = %u", __FUNCTION__, __FILE__, __LINE__, __label__, size))
#	define  MESSAGE_FAILURE_TRACE_INDEX(__index__)	\
			soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to transmit item #%u, size = %u", __FUNCTION__, __FILE__, __LINE__, __index__, size))
#	define  MESSAGE_FAILURE_TRACE_LABEL_INDEX(__label__, __index__)	\
			soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to transmit %s, item #%u, size = %u", __FUNCTION__, __FILE__, __LINE__, __label__, __index__, size))
#else
#	define  MESSAGE_FAILURE_TRACE_GENERIC
#	define  MESSAGE_FAILURE_TRACE_LABEL(__label__)
#	define  MESSAGE_FAILURE_TRACE_INDEX(__index__)
#endif

#define DefineClassScribe(ClassName)																							\
	inline unsigned Read(const unsigned char * stream, unsigned size, ClassName & data, unsigned maxLen, unsigned version = 0)	\
	{																															\
		return data.Read(stream, size, version);																				\
	}																															\
	inline unsigned Write(unsigned char * stream, unsigned size, const ClassName & data, unsigned version = 0)					\
	{																															\
		return data.Write(stream, size, version);																				\
	}																															\
	inline int Print(char * stream, unsigned size, const ClassName & data, unsigned maxDepth = INT_MAX)							\
	{																															\
		return data.Print(stream, size, maxDepth);																				\
	}

#define DECLARE_SCRIBE_MEMBERS_INTERNAL																			\
	unsigned		Read(const unsigned char * stream, unsigned size, unsigned version = 0);					\
	unsigned		Write(unsigned char * stream, unsigned size, unsigned version = 0) const;					\
	int				Print(char * stream, unsigned size, unsigned maxDepth = INT_MAX) const;						\
	class Scribe;																								\
	static Scribe & GetScribe();

#define DECLARE_VIRTUAL_SCRIBE_MEMBERS_INTERNAL																	\
	virtual unsigned	Read(const unsigned char * stream, unsigned size, unsigned version = 0);				\
	virtual unsigned	Write(unsigned char * stream, unsigned size, unsigned version = 0) const;				\
	virtual int			Print(char * stream, unsigned size, unsigned maxDepth = INT_MAX) const;					\
	class Scribe;																									\
	static Scribe & GetScribe();

#define DECLARE_SCRIBE_MEMBERS																					\
	unsigned		Read(const unsigned char * stream, unsigned size, unsigned version = 0);					\
	unsigned		Write(unsigned char * stream, unsigned size, unsigned version = 0) const;					\
	int				Print(char * stream, unsigned size, unsigned maxDepth = INT_MAX) const;

#define DECLARE_VIRTUAL_SCRIBE_MEMBERS																			\
	virtual unsigned	Read(const unsigned char * stream, unsigned size, unsigned version = 0);				\
	virtual unsigned	Write(unsigned char * stream, unsigned size, unsigned version = 0) const;				\
	virtual int			Print(char * stream, unsigned size, unsigned maxDepth = INT_MAX) const;

#ifdef PRINTABLE_MESSAGES
#	define			PrintEnum(EnumName)																				\
	long lData = data;																								\
	return snprintf(stream, size, "('%s')%d", #EnumName, lData);
#else
#	define			PrintEnum(EnumName)																				\
	return 0;
#endif

#define SCRIBE_ENUM(EnumName)																							\
    inline unsigned Write( unsigned char * stream, unsigned size, const EnumName & data, unsigned version = 0)			\
    {																													\
        unsigned dataSize = sizeof(EnumName);																			\
        if (size < dataSize)																							\
            return 0;																									\
        memcpy(stream, (void*)&data, dataSize);																			\
        return dataSize;																								\
    }																													\
    inline unsigned Read( const unsigned char * stream, unsigned size, EnumName & data, unsigned, unsigned version = 0)	\
    {																													\
        unsigned dataSize = sizeof(EnumName);																			\
        if (size < dataSize)																							\
            return 0;																									\
        memcpy((void*)&data, stream, dataSize);																			\
        return dataSize;																								\
    }																													\
    inline int Print( char * stream, unsigned size, const EnumName & data, unsigned maxDepth = 0)						\
    {																													\
        PrintEnum(EnumName)																								\
    }

#ifdef WIN32
#	define mapped_type referent_type
#endif

#define DefineMapScribe(MapName)																						\
	unsigned Read(const unsigned char * stream, unsigned size, MapName & data, unsigned maxLen, unsigned version = 0);

#define ImplementMapScribeMember(MapName, KeyName)																		\
	unsigned Read(const unsigned char * stream, unsigned size, MapName & data, unsigned maxLen, unsigned version)		\
	{																													\
		return  Read(stream, size, data,  MapName::mapped_type::KeyName, maxLen, version);								\
	}
    
#define ImplementMapScribeAccessor(MapName, KeyName)																	\
	unsigned Read(const unsigned char * stream, unsigned size, MapName & data, unsigned maxLen, unsigned version)		\
	{																													\
		return  Read(stream, size, data,  &MapName::mapped_type::KeyName, maxLen, version);								\
	}

#ifdef API_NAMESPACE
	// forward/extern declarations
	namespace soe { class ClassScribeBase; }
	namespace API_NAMESPACE { void AddClassScribeToGlobalSet(soe::ClassScribeBase * pScribe); }
	// macro to add scribe to global set for APIs
#	define AddScribeToResetSet	API_NAMESPACE::AddClassScribeToGlobalSet(&classScribe);
#else
#	define AddScribeToResetSet	// NOP - nothing to do in this case
#endif

#define ImplementClassScribeBeginInternal(ClassName)															\
	class ClassName::Scribe : public soe::ClassScribe<ClassName>												\
	{																											\
	public:																										\
		Scribe();																								\
		void Initialize();																						\
	};																											\
	ClassName::Scribe & ClassName::GetScribe()																	\
	{																											\
		static ClassName::Scribe classScribe;																	\
		if (!classScribe.IsInitialized()) {																		\
			classScribe.ClearMembers();																			\
			classScribe.Initialize();																			\
		}																										\
		return classScribe;																						\
	}																											\
	unsigned ClassName::Read(const unsigned char * stream, unsigned size, unsigned version)						\
	{																											\
		return GetScribe().Read(stream, size, *this, version);													\
	}																											\
	unsigned ClassName::Write(unsigned char * stream, unsigned size, unsigned version) const					\
	{																											\
		return GetScribe().Write(stream, size, *this, version);													\
	}																											\
	int	ClassName::Print(char * stream, unsigned size, unsigned maxDepth) const									\
	{																											\
		return GetScribe().Print(stream, size, *this, maxDepth);												\
	}																											\
	ClassName::Scribe::Scribe()																					\
		: soe::ClassScribe<ClassName>(#ClassName)																\
	{ 																											\
		Initialize();																							\
	}																											\
	void ClassName::Scribe::Initialize()																		\
	{																											\
		typedef ClassName ScribedClass_t;																		\
		Scribe & classScribe = *this;																			\
		ClearMembers();

#define ImplementClassScribeBegin(ClassName)																	\
	soe::ClassScribe<ClassName> & GetScribe(const ClassName & object);											\
	unsigned ClassName::Read(const unsigned char * stream, unsigned size, unsigned version)						\
	{																											\
		return GetScribe(*this).Read(stream, size, *this, version);												\
	}																											\
	unsigned ClassName::Write(unsigned char * stream, unsigned size, unsigned version) const					\
	{																											\
		return GetScribe(*this).Write(stream, size, *this, version);											\
	}																											\
	int	ClassName::Print(char * stream, unsigned size, unsigned maxDepth) const									\
	{																											\
		return GetScribe(*this).Print(stream, size, *this, maxDepth);											\
	}																											\
	soe::ClassScribe<ClassName> & GetScribe(const ClassName & object)											\
	{																											\
		typedef ClassName ScribedClass_t;																		\
		static soe::ClassScribe<ClassName> classScribe(#ClassName);												\
		if (!classScribe.IsInitialized()) {																		\
			classScribe.ClearMembers();
		
#define ImplementClassScribeBeginEx(ClassName, ParentClassName)													\
	soe::ClassScribe<ClassName> & GetScribe(const ClassName & object);											\
	unsigned ClassName::Read(const unsigned char * stream, unsigned size, unsigned version)						\
	{																											\
		unsigned bytesTotal = 0;																				\
		unsigned bytes = 0;																						\
		bytesTotal += (bytes = GetScribe((const ParentClassName &)*this).Read(stream, size, *this, version));	\
		if (!bytes && (GetScribe((const ParentClassName &)*this).CountMembers() > 0)) { return 0; }				\
		bytesTotal += (bytes = GetScribe(*this).Read(stream+bytesTotal, size-bytesTotal , *this, version));		\
		if (!bytes && (GetScribe(*this).CountMembers() > 0)) { return 0; }										\
		return bytesTotal;																						\
	}																											\
	unsigned ClassName::Write(unsigned char * stream, unsigned size, unsigned version) const					\
	{																											\
		unsigned bytesTotal = 0;																				\
		unsigned bytes = 0;																						\
		bytesTotal += (bytes = GetScribe((const ParentClassName &)*this).Write(stream, size, *this, version));	\
		if (!bytes && (GetScribe((const ParentClassName &)*this).CountMembers() > 0)) { return 0; }				\
		bytesTotal += (bytes = GetScribe(*this).Write(stream+bytesTotal, size-bytesTotal , *this, version));	\
		if (!bytes && (GetScribe(*this).CountMembers() > 0)) { return 0; }										\
		return bytesTotal;																						\
	}																											\
	int	ClassName::Print(char * stream, unsigned size, unsigned maxDepth) const									\
	{																											\
		int bytesTotal = 0;																						\
		int bytes = 0;																							\
		bytesTotal += (bytes = GetScribe((const ParentClassName &)*this).Print(stream, size, *this, maxDepth));	\
		if (soe::FailedToPrint(bytes, size)) { return bytes; }													\
		bytesTotal += (bytes = GetScribe(*this).Print(stream+bytesTotal , size-bytesTotal, *this, maxDepth));	\
		if (soe::FailedToPrint(bytes, size-bytesTotal)) { return bytes; }										\
		return bytesTotal;																						\
	}																											\
	soe::ClassScribe<ClassName> & GetScribe(const ClassName & object)											\
	{																											\
		typedef ClassName ScribedClass_t;																		\
		static soe::ClassScribe<ClassName> classScribe(#ClassName);												\
		if (!classScribe.IsInitialized()) {																		\
			classScribe.ClearMembers();

#define ImplementClassScribeMember(MemberName)																	\
		classScribe.AddMember(#MemberName, &ScribedClass_t::MemberName, 1, 0, eNoEffect);

#define ImplementClassScribeMemberEx(MemberName,MaxLen)															\
		classScribe.AddMember(#MemberName, &ScribedClass_t::MemberName, MaxLen, 0, eNoEffect);

#define ImplementClassScribeMemberContainerEx(MemberName,MaxLen,MaxElementLen)									\
	classScribe.AddMemberContainer(#MemberName, &ScribedClass_t::MemberName, MaxLen, MaxElementLen, 0, eNoEffect);

#define ImplementVersionAddedClassScribeMember(MemberName,Version)												\
		classScribe.AddMember(#MemberName, &ScribedClass_t::MemberName, 1, Version, eAdded);

#define ImplementVersionAddedClassScribeMemberEx(MemberName,MaxLen,Version)										\
		classScribe.AddMember(#MemberName, &ScribedClass_t::MemberName, MaxLen, Version, eAdded);

#define ImplementVersionAddedClassScribeMemberContainerEx(MemberName,MaxLen,MaxElementLen,Version)				\
		classScribe.AddMemberContainer(#MemberName, &ScribedClass_t::MemberName, MaxLen, MaxElementLen, Version, eAdded);

#define ImplementVersionDroppedClassScribeMember(MemberName,Version)											\
		classScribe.AddMember(#MemberName, &ScribedClass_t::MemberName, 1, Version, eDropped);

#define ImplementVersionDroppedClassScribeMemberEx(MemberName,MaxLen,Version)									\
		classScribe.AddMember(#MemberName, &ScribedClass_t::MemberName, MaxLen, Version, eDropped);

#define ImplementVersionDroppedClassScribeMemberContainerEx(MemberName,MaxLen,MaxElementLen,Version)			\
		classScribe.AddMemberContainer(#MemberName, &ScribedClass_t::MemberName, MaxLen, MaxElementLen, Version, eDropped);

#define ImplementClassScribeEndInternal																			\
		SetInitialized(true);																					\
		AddScribeToResetSet																						\
	}

#define ImplementClassScribeEnd																					\
			classScribe.SetInitialized(true);																	\
			AddScribeToResetSet																					\
		}																										\
		return classScribe;																						\
	}

#include "types.h"

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


namespace soe
{


	////////////////////////////////////////////////////////////////////////////////

	
	//	388 ns (50 bytes)
	inline unsigned Write(unsigned char * stream, unsigned size, const unsigned char * data, unsigned dataLen, unsigned version = 0)
	{
		if (size < dataLen)
			return 0;
		memcpy(stream, data, dataLen);
		return dataLen;
	}

	//	353 ns
	inline unsigned Write(unsigned char * stream, unsigned size, bool data, unsigned version = 0)
	{
		if (size < sizeof(uint8))
			return 0;
		stream[BYTE1] = data;
		return sizeof(uint8);
	}

	//	362 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int8 data, unsigned version = 0)
	{
		if (size < sizeof(int8))
			return 0;
		stream[BYTE1] = data;
		return sizeof(int8);
	}

	//  362 ns
	inline unsigned Write(unsigned char * stream, unsigned size, uint8 data, unsigned version = 0)
	{
		if (size < sizeof(uint8))
			return 0;
		stream[BYTE1] = data;
		return sizeof(uint8);
	}

	//	355 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int16 data, unsigned version = 0)
	{
		if (size < sizeof(int16))
			return 0;
		stream[BYTE1] = data&0xff;
		stream[BYTE2] = (data>>8)&0xff;
		return sizeof(int16);
	}

	//	360 ns
	inline unsigned Write(unsigned char * stream, unsigned size, uint16 data, unsigned version = 0)
	{
		if (size < sizeof(uint16))
			return 0;
		stream[BYTE1] = data&0xff;
		stream[BYTE2] = (data>>8)&0xff;
		return sizeof(uint16);
	}

	//	360 ns
	inline unsigned Write(unsigned char * stream, unsigned size, int32 data, unsigned version = 0)
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
	inline unsigned Write(unsigned char * stream, unsigned size, uint32 data, unsigned version = 0)
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
	inline unsigned Write(unsigned char * stream, unsigned size, int64 data, unsigned version = 0)
	{
		if (size < sizeof(int64))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 low = *(uint32_t *) &data;
		uint32 high = *((uint32_t *)&data+1);
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
	inline unsigned Write(unsigned char * stream, unsigned size, uint64 data, unsigned version = 0)
	{
		if (size < sizeof(uint64))
			return 0;
#if (defined(WIN32) || defined(linux))
	uint32 low = *(uint32_t *) &data;		
	uint32 high = *((uint32_t *)&data+1);
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
	inline unsigned Write(unsigned char * stream, unsigned size, float data, unsigned version = 0)
	{
		uint32 & dataRef = *(uint32_t *)(&data);
		if (size < sizeof(float))
			return 0;
		stream[BYTE1] = dataRef&0xff;
		stream[BYTE2] = (dataRef>>8)&0xff;
		stream[BYTE3] = (dataRef>>16)&0xff;
		stream[BYTE4] = (dataRef>>24)&0xff;
		return sizeof(float);
	}

	//	370 ns
	inline unsigned Write(unsigned char * stream, unsigned size, double data, unsigned version = 0)
	{
		if (size < sizeof(double))
			return 0;
#if (defined(WIN32) || defined(linux))
		uint32 low = *(uint32_t *)(&data);
		uint32 high = *((uint32_t *)&data+1);
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
	inline unsigned Write(unsigned char * stream, unsigned size, const std::string & data, unsigned version = 0)
	{
		unsigned lengthBytes = 0;
		unsigned bytes = 0;
		lengthBytes += WriteEncoded(stream, size, (soe::uint32)data.length());
		if (!lengthBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to write length (%u bytes)", __FUNCTION__, __FILE__, __LINE__, sizeof(soe::uint32)));
#endif
			return 0;
		}
		if (data.length())
		{
			bytes = Write(stream+lengthBytes, size-lengthBytes, reinterpret_cast<const unsigned char *>(data.data()), (soe::uint32)data.length());
			if (!bytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to write %u string bytes.", __FUNCTION__, __FILE__,  __LINE__, data.length()));
#endif
				return 0;
			}
		}
		return lengthBytes + bytes;
	}

	inline unsigned Read(const unsigned char * stream, unsigned size, unsigned char * data, unsigned dataLen, unsigned version = 0)
	{
		if (size < dataLen)
			return 0;
		memcpy(data, stream, dataLen);
		return dataLen;
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, bool & data, unsigned = 1/*unused*/, unsigned version = 0)
	{
		if (size < sizeof(uint8))
			return 0;
		data = stream[BYTE1] != 0;
		return sizeof(uint8);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int8 & data, unsigned = 1/*unused*/, unsigned version = 0)
	{
		if (size < sizeof(int8))
			return 0;
		data = stream[BYTE1];
		return sizeof(int8);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, uint8 & data, unsigned = 1/*unused*/, unsigned version = 0)
	{
		if (size < sizeof(uint8))
			return 0;
		data = stream[BYTE1];
		return sizeof(uint8);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int16 & data, unsigned = 1/*unused*/, unsigned version = 0)
	{
		if (size < sizeof(int16))
			return 0;
		data = stream[BYTE1];
		data |= stream[BYTE2]<<8;
		return sizeof(int16);
	}

	//	355 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, uint16 & data, unsigned = 1/*unused*/, unsigned version = 0)
	{
		if (size < sizeof(uint16))
			return 0;
		data = stream[BYTE1];
		data |= stream[BYTE2]<<8;
		return sizeof(uint16);
	}

	//	360 ns
	inline unsigned Read(const unsigned char * stream, unsigned size, int32 & data, unsigned = 1/*unused*/, unsigned version = 0)
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
	inline unsigned Read(const unsigned char * stream, unsigned size, uint32 & data, unsigned = 1/*unused*/, unsigned version = 0)
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
	inline unsigned Read(const unsigned char * stream, unsigned size, int64 & data, unsigned = 1/*unused*/, unsigned version = 0)
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
	inline unsigned Read(const unsigned char * stream, unsigned size, uint64 & data, unsigned = 1/*unused*/, unsigned version = 0)
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
	inline unsigned Read(const unsigned char * stream, unsigned size, float & data, unsigned = 1/*unused*/, unsigned version = 0)
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
	inline unsigned Read(const unsigned char * stream, unsigned size, double & data, unsigned = 1/*unused*/, unsigned version = 0)
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

	inline unsigned Read(const unsigned char * stream, unsigned size, std::string & data, unsigned maxLen, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned length = 0;
		bytes += ReadEncoded(stream, size, length);
		if (!bytes || size < bytes+length || length > maxLen) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to read string, length = %u, maxLen = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, maxLen, size));
#endif
			return 0;
		}
		data.assign((const char *)stream+bytes, (const char *)stream+bytes+length);  // assign would be dangerous if unicode, single-byte characters are ok
		return bytes + length;
	}

#ifdef _USE_32BIT_TIME_T
	// Seems that VC8 has switched time_t to __int64 so this becomes a signature clash
	inline unsigned Read(const unsigned char * stream, unsigned size, time_t & data, unsigned maxLen = 1, unsigned version = 0)
	{
		soe::uint32 wordData = 0;
		unsigned bytesRead = Read(stream, size, wordData, maxLen);

		data = (time_t)wordData;

		return bytesRead;
	}

	inline unsigned Write(unsigned char * stream, unsigned size, time_t data, unsigned version = 0)
	{
		return Write(stream, size, (soe::uint32)data);
	}
#endif

	inline bool FailedToPrint(int bytesPrinted, unsigned maxBytes)
	{
		return ( (bytesPrinted < 0) || (bytesPrinted > (int)maxBytes) );
	}

#ifdef PRINTABLE_MESSAGES
	inline int Print(char * stream, unsigned size, time_t data, unsigned maxDepth=INT_MAX)
	{
		tm *timeStruct = localtime(&data);
		
		if (timeStruct) {
			size_t bytes = strftime(stream, size, "%Y-%m-%d %H:%M:%S", timeStruct);

			if (bytes > 0) {
				return (int)bytes;
			} else {
				return -1;
			}
		} else {
			return snprintf(stream, size, "(time_t)%u)", (unsigned)data);
		}
	}
	
	inline int Print(char * stream, unsigned size, const unsigned char * data, unsigned dataLen, unsigned maxDepth=INT_MAX)
	{
		int bytes = 0;
		int moreBytes = snprintf(stream, size, "[[%u]]", dataLen);

		if ((maxDepth == 0) || (FailedToPrint(moreBytes, size))) {
			return moreBytes;
		} else {
			bytes += moreBytes;
		}

		moreBytes = snprintf(stream+bytes, size-bytes, "0x");
		if (FailedToPrint(moreBytes, size-bytes)) {
			return moreBytes;
		} else {
			bytes += moreBytes;
		}

		for (unsigned index = 0; index < dataLen; index++)
		{
			moreBytes = snprintf(stream+bytes, size-bytes, "%x", data[index]);
			if (FailedToPrint(moreBytes, size-bytes)) { break; }
			bytes += moreBytes;
		}

		return FailedToPrint(moreBytes, size-bytes) ? moreBytes : bytes;
	}

	inline int Print(char * stream, unsigned size, bool data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%s", data ? "TRUE" : "FALSE");
	}

	inline int Print(char * stream, unsigned size, int8 data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%d", data);
	}

	inline int Print(char * stream, unsigned size, uint8 data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%u", data);
	}

	inline int Print(char * stream, unsigned size, int16 data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%d", data);
	}

	inline int Print(char * stream, unsigned size, uint16 data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%u", data);
	}

	inline int Print(char * stream, unsigned size, int32 data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%d", data);
	}

	inline int Print(char * stream, unsigned size, uint32 data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%u", data);
	}

	inline int Print(char * stream, unsigned size, int64 data, unsigned maxDepth=INT_MAX)
	{
#	ifdef WIN32
		return snprintf(stream, size, "%I64d", data);
#	else
		return snprintf(stream, size, "%lld", data);
#	endif
	}

	inline int Print(char * stream, unsigned size, uint64 data, unsigned maxDepth=INT_MAX)
	{
#	ifdef WIN32
		return snprintf(stream, size, "%I64u", data);
#	else
		return snprintf(stream, size, "%llu", data);
#	endif
	}

	inline int Print(char * stream, unsigned size, float data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%f", data);
	}

	inline int Print(char * stream, unsigned size, double data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%f", data);
	}

	inline int Print(char * stream, unsigned size, const std::string & data, unsigned maxDepth=INT_MAX)
	{
		return snprintf(stream, size, "%s", data.c_str());
	}
#endif

	////////////////////////////////////////////////////////////////////////////////

	class DECLSPEC AutoVariableBase
	{
        public:
			AutoVariableBase() {}
			virtual ~AutoVariableBase() {}

			virtual void Copy(AutoVariableBase *pSource) = 0;

			virtual unsigned Write(unsigned char * stream, unsigned size, unsigned version = 0) const = 0;
			virtual unsigned Read(const unsigned char * stream, unsigned size, unsigned maxLen = 1, unsigned version = 0) = 0;
#ifdef PRINTABLE_MESSAGES 
			virtual int	Print(char * stream, unsigned size, unsigned maxDepth=INT_MAX) const = 0;
#endif
#if defined(PRINTABLE_MESSAGES) || defined(TRACK_READ_WRITE_FAILURES)
			virtual const char * VariableName() const { return "(Unknown)"; }
#endif
	};
   
	////////////////////////////////////////////////////////////////////////////////

	enum EVersionEffect
	{
		eNoEffect,
		eDropped,
		eAdded,
	};

	inline bool IsMemberIncluded(unsigned messageVersion, unsigned memberVersion, EVersionEffect memberVersionEffect)
	{
		bool isIncluded = true;

		if (messageVersion) {
			switch(memberVersionEffect)
			{
			case eAdded:
				isIncluded = (messageVersion >= memberVersion);
				break;
			case eDropped:
				isIncluded = (messageVersion < memberVersion);
				break;
			case eNoEffect:
			default:
				break;
			}
		}

		return isIncluded;
	}

	////////////////////////////////////////////////////////////////////////////////
}

#endif
