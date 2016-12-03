#ifndef SERIALIZESTRINGVECTOR_H
#define	SERIALIZESTRINGVECTOR_H

#ifdef SOE__SERIALIZE_TEMPLATES_H
#error wrong odering, serialzeTemplates.h must be included after this file.
#endif

#include <list>
#include <vector>
#include <string>
#include "serializeClasses.h"

#define DEFAULT_MAX_LENGTH_STRING 1000
#define DEFAULT_MAX_NUMBER_ELEMENTS 1000


namespace soe
{
	///////////////////////////////////////////////////////////////////////////////////////////
	// SerializeStringVector
	//	This class is just a std::vector<std::string> with extra serialization routines.
	//
	//	This class is designed to handle the serialization of a vector of strings.  This was necessary
	//	because the default serialization routines did not handle the length of the string properly, and
	//	there was more risk involved in modifying all the serialization routines to handle the 
	//	extra length property for the string as well as the vector.
	//
	// this is intenede to be used with the serialization macros
	//	Ex:
	//		DefineMessageMember(myStringVector, SerializeStringVector)
	//		ImplementMessageMember(paymentInstrument, SerializeStringVector(DEFAULT_MAX_LENGTH_STRING, DEFAULT_MAX_NUMBER_ELEMENTS))
	///////////////////////////////////////////////////////////////////////////////////////////

	typedef std::list<std::string> stringList_t;
	typedef std::vector<std::string> stringVector_t;

	class SerializeStringList : public stringList_t
	{
	private:
		unsigned mMaxStringLength;
		unsigned mMaxListLength;
	protected:
	public:
		SerializeStringList()
			: mMaxStringLength(DEFAULT_MAX_LENGTH_STRING), mMaxListLength(DEFAULT_MAX_NUMBER_ELEMENTS)
		{};
		// writing will use the default values and assume that the vector is valid
		//SerializeStringList(stringList_t vStrings);
		SerializeStringList(unsigned maxStringLength, unsigned maxListLength)
			: mMaxStringLength(maxStringLength), mMaxListLength(maxListLength)
		{
		}
		SerializeStringList(stringList_t vStrings)
			: mMaxStringLength(DEFAULT_MAX_LENGTH_STRING), mMaxListLength(DEFAULT_MAX_NUMBER_ELEMENTS)
		{
			for(stringList_t::iterator i = vStrings.begin(); i != vStrings.end(); i++)
			{
				push_back(*i);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		unsigned Read(const unsigned char * stream, unsigned size)
		{
			// fix this:
			std::string element;
			unsigned bytes = 0;
			unsigned elementBytes = 0;
			unsigned length = 0;
			clear();
			bytes += ReadEncoded(stream, size, length);
			if (!bytes || length > mMaxListLength) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to read StringList, length = %u, mMaxListLength = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, mMaxListLength,  size));
#endif
				return 0;
			}
			for (unsigned i=0; i<length; i++)
			{
				elementBytes = soe::Read(stream+bytes, size-bytes, element, mMaxStringLength);
				if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
					PushMessageFailure(PrintToString("%s, %s:%u - failed to read StringList element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, i, size));
#endif
					return 0;
				}
				push_back(element);
				bytes += elementBytes;
			}
			return bytes;
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		unsigned Write(unsigned char * stream, unsigned size) const
		{
			unsigned index = 0;
			unsigned bytes = 0;
			unsigned elementBytes = 0;
			bytes += WriteEncoded(stream, size, (soe::uint32)this->size());
			if (!bytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to write StringList length (%u bytes)", __FUNCTION__, __FILE__, __LINE__, sizeof(soe::uint32)));
#endif
				return 0;
			}
			for (stringList_t::const_iterator iter = begin(); iter != end(); iter++, index++)
			{
				elementBytes = soe::Write(stream+bytes, size-bytes, *iter);
				if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
					PushMessageFailure(PrintToString("%s, %s:%u - failed to write StringList element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, index, size));
#endif
					return 0;
				}
				bytes += elementBytes;
			}

			return bytes;
		}

		//operator stringList_t () { return *this; }

		//unsigned Read(const unsigned char * stream, unsigned size);
		//DECLARE_SCRIBE_MEMBERS
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	inline unsigned Write(unsigned char * stream, unsigned size, const SerializeStringList & data, unsigned version = 0)
	{
		return data.Write(stream, size);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	inline unsigned Read(const unsigned char * stream, unsigned size, SerializeStringList & data, unsigned, unsigned version = 0)
	{
		return data.Read(stream, size);
	}


	class SerializeStringVector : public stringVector_t
	{
	private:
		unsigned mMaxStringLength;
		unsigned mMaxVectorLength;
	protected:
	public:
		SerializeStringVector()
			: mMaxStringLength(DEFAULT_MAX_LENGTH_STRING), mMaxVectorLength(DEFAULT_MAX_NUMBER_ELEMENTS)
		{};
		// writing will use the default values and assume that the vector is valid
		//SerializeStringVector(stringVector_t vStrings);
		SerializeStringVector(unsigned maxStringLength, unsigned maxVectorLength)
			: mMaxStringLength(maxStringLength), mMaxVectorLength(maxVectorLength)
		{
		}
		SerializeStringVector(stringVector_t vStrings)
			: mMaxStringLength(DEFAULT_MAX_LENGTH_STRING), mMaxVectorLength(DEFAULT_MAX_NUMBER_ELEMENTS)
		{
			for(stringVector_t::iterator i = vStrings.begin(); i != vStrings.end(); i++)
			{
				push_back(*i);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		unsigned Read(const unsigned char * stream, unsigned size)
		{
			// fix this:
			std::string element;
			unsigned bytes = 0;
			unsigned elementBytes = 0;
			unsigned length = 0;
			clear();
			bytes += ReadEncoded(stream, size, length);
			if (!bytes || length > mMaxVectorLength) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to read StringVector, length = %u, mMaxListLength = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, mMaxVectorLength,  size));
#endif
				return 0;
			}
			for (unsigned i=0; i<length; i++)
			{
				elementBytes = soe::Read(stream+bytes, size-bytes, element, mMaxStringLength);
				if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
					PushMessageFailure(PrintToString("%s, %s:%u - failed to read StringVector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, i, size));
#endif
					return 0;
				}
				push_back(element);
				bytes += elementBytes;
			}
			return bytes;
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		unsigned Write(unsigned char * stream, unsigned size) const
		{
			unsigned index = 0;
			unsigned bytes = 0;
			unsigned elementBytes = 0;
			bytes += WriteEncoded(stream, size, (soe::uint32)this->size());
			if (!bytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to write StringVector length (%u bytes)", __FUNCTION__, __FILE__, __LINE__, sizeof(soe::uint32)));
#endif
				return 0;
			}
			for (stringVector_t::const_iterator iter = begin(); iter != end(); iter++, index++)
			{
				elementBytes = soe::Write(stream+bytes, size-bytes, *iter);
				if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
					PushMessageFailure(PrintToString("%s, %s:%u - failed to write StringVector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, index, size));
#endif
					return 0;
				}
				bytes += elementBytes;
			}

			return bytes;
		}

		//unsigned Read(const unsigned char * stream, unsigned size);
		//DECLARE_SCRIBE_MEMBERS
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	inline unsigned Write(unsigned char * stream, unsigned size, const SerializeStringVector & data, unsigned version = 0)
	{
		return data.Write(stream, size);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	inline unsigned Read(const unsigned char * stream, unsigned size, SerializeStringVector & data, unsigned, unsigned version = 0)
	{
		return data.Read(stream, size);
	}

}	// namespace soe

#endif //SERIALIZESTRINGVECTOR_H

