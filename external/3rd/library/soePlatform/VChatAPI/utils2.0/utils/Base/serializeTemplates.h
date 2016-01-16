#ifndef SOE__SERIALIZE_TEMPLATES_H
#define SOE__SERIALIZE_TEMPLATES_H

#ifdef TRACK_READ_WRITE_FAILURES
#	include "trackMessageFailures.h"
#endif
#include <climits>
#include <map>

namespace soe
{
	////////////////////////////////////////////////////////////////////////////////
		
	template<typename K, class T>
	inline unsigned Write(unsigned char * stream, unsigned size, const std::map<K, T> & data, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		bytes += WriteEncoded(stream, size, (soe::uint32)data.size());
		if (!bytes) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to write vector length (%u bytes)", __FUNCTION__, __FILE__, __LINE__, sizeof(soe::uint32)));
#endif
			return 0;
		}
		size_t index = 0;
		for (typename std::map<K, T>::const_iterator iter = data.begin(); iter != data.end(); iter++, index++)
		{
			elementBytes = Write(stream+bytes, size-bytes, iter->second, version);
			if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to write map element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, index, size));
#endif
				return 0;
			}
			bytes += elementBytes;
		}

		return bytes;
	}

	template<typename K, class T>
	inline unsigned Read(const unsigned char * stream, unsigned size, std::map<K, T> & data, K T::*dataKey, unsigned maxLen, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		unsigned length = 0;
		data.clear();
		bytes += ReadEncoded(stream, size, length);
		if (!bytes || length > maxLen) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector, length = %u, maxLen = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, maxLen,  size));
#endif
			return 0;
		}
		for (unsigned i=0; i<length; i++)
		{
			T element;
			elementBytes = Read(stream+bytes, size-bytes, element, 1, version);
			if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, i, size));
#endif
				return 0;
			}
			data[element.*dataKey] = element;
			data.push_back(element);
			bytes += elementBytes;
		}
		return bytes;
	}

	template<typename K, class T>
	inline unsigned Read(const unsigned char * stream, unsigned size, std::map<K, T> & data, const K & (T::*dataKeyAccessor)() const, unsigned maxLen, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		unsigned length = 0;
		data.clear();
		bytes += ReadEncoded(stream, size, length);
		if (!bytes || length > maxLen) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector, length = %u, maxLen = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, maxLen,  size));
#endif
			return 0;
		}
		for (unsigned i=0; i<length; i++)
		{
			T element;
			elementBytes = Read(stream+bytes, size-bytes, element, 1, version);
			if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, i, size));
#endif
				return 0;
			}
			data[(element.*dataKeyAccessor)()] = element;
			bytes += elementBytes;
		}
		return bytes;
	}

	template<typename K, class T>
	inline unsigned Read(const unsigned char * stream, unsigned size, std::map<K, T> & data, K (T::*dataKeyAccessor)() const, unsigned maxLen, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		unsigned length = 0;
		data.clear();
		bytes += ReadEncoded(stream, size, length);
		if (!bytes || length > maxLen) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector, length = %u, maxLen = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, maxLen,  size));
#endif
			return 0;
		}
		for (unsigned i=0; i<length; i++)
		{
			T element;
			elementBytes = Read(stream+bytes, size-bytes, element, 1, version);
			if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, i, size));
#endif
				return 0;
			}
			data[element.*dataKeyAccessor()] = element;
			data.push_back(element);
			bytes += elementBytes;
		}
		return bytes;
	}

	////////////////////////////////////////////////////////////////////////////////

	template<class T>
	inline unsigned Write(unsigned char * stream, unsigned size, const std::list<T> & data, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		bytes += WriteEncoded(stream, size, (soe::uint32)data.size());
		if (!bytes)
			return 0;
		for (typename std::list<T>::const_iterator iter = data.begin(); iter != data.end(); iter++)
		{
			elementBytes = Write(stream+bytes, size-bytes, *iter, version);
			if (!elementBytes)
				return 0;
			bytes += elementBytes;
		}

		return bytes;
	}

	template<class T>
	inline unsigned ReadContainer(const unsigned char * stream, unsigned size, std::list<T> & data, unsigned maxLen, unsigned maxElementLen, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		unsigned length = 0;
		data.clear();
		bytes += ReadEncoded(stream, size, length);
		if (!bytes || length > maxLen)
			return 0;
		for (unsigned i=0; i<length; i++)
		{
			T element;
			elementBytes = Read(stream+bytes, size-bytes, element, maxElementLen, version);
			if (!elementBytes)
				return 0;
			data.push_back(element);
			bytes += elementBytes;
		}
		return bytes;
	}

	template<class T>
	inline unsigned Read(const unsigned char * stream, unsigned size, std::list<T> & data, unsigned maxLen, unsigned version = 0)
	{
		return ReadContainer(stream, size, data, maxLen, 1, version);
	}

	////////////////////////////////////////////////////////////////////////////////

	template<class T>
	inline unsigned Write(unsigned char * stream, unsigned size, const std::vector<T> & data, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		bytes += WriteEncoded(stream, size, (soe::uint32)data.size());
		if (!bytes) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to write vector length (%u bytes)", __FUNCTION__, __FILE__, __LINE__, sizeof(soe::uint32)));
#endif
			return 0;
		}
		for (size_t index = 0; index < data.size(); index++)
		{
			elementBytes = Write(stream+bytes, size-bytes, data[index], version);
			if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to write vector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, index, size));
#endif
				return 0;
			}
			bytes += elementBytes;
		}

		return bytes;
	}

	template<class T>
	inline unsigned ReadContainer(const unsigned char * stream, unsigned size, std::vector<T> & data, unsigned maxLen, unsigned maxElementLen, unsigned version = 0)
	{
		unsigned bytes = 0;
		unsigned elementBytes = 0;
		unsigned length = 0;
		data.clear();
		bytes += ReadEncoded(stream, size, length);
		if (!bytes || length > maxLen) {
#ifdef TRACK_READ_WRITE_FAILURES
			PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector, length = %u, maxLen = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, length, maxLen,  size));
#endif
			return 0;
		}
		for (unsigned i=0; i<length; i++)
		{
			T element;
			elementBytes = Read(stream+bytes, size-bytes, element, maxElementLen, version);
			if (!elementBytes) {
#ifdef TRACK_READ_WRITE_FAILURES
				PushMessageFailure(PrintToString("%s, %s:%u - failed to read vector element, index = %u, size = %u", __FUNCTION__, __FILE__, __LINE__, i, size));
#endif
				return 0;
			}
			data.push_back(element);
			bytes += elementBytes;
		}
		return bytes;
	}
	
	template<class T>
	inline unsigned Read(const unsigned char * stream, unsigned size, std::vector<T> & data, unsigned maxLen, unsigned version = 0)
	{
		return ReadContainer(stream, size, data, maxLen, 1, version);
	}

#ifdef PRINTABLE_MESSAGES
	template<typename K, class T>
	inline int Print(char * stream, unsigned size, const std::map<K, T> & data, unsigned maxDepth=INT_MAX)
	{
		int bytes = 0;
		int elementBytes = 0;

		if (maxDepth == 0) {
			bytes = snprintf(stream, size, "{size(%u)}", data.size());
			return bytes;
		}

		elementBytes = snprintf(stream+bytes, size-bytes, "{");

		if (FailedToPrint(elementBytes, size-bytes)) {
			bytes = elementBytes;
			return bytes;
		} else {
			bytes += elementBytes;
		}

		bool printedOne = false;
		size_t index = 0;
		for (typename std::map<K, T>::const_iterator iter = data.begin(); iter != data.end(); iter++, index++)
		{
			if (printedOne) {
				elementBytes = snprintf(stream+bytes, size-bytes, ", ");

				if (FailedToPrint(elementBytes, size-bytes)) {
					bytes = elementBytes;
					return bytes;
				} else {
					bytes += elementBytes;
				}
			} else {
				printedOne = true;
			}

			elementBytes = snprintf(stream+bytes, size-bytes, "[%u]", index);
			if (FailedToPrint(elementBytes, size-bytes)) {
				return elementBytes;
			}

			bytes += elementBytes;

			elementBytes = Print(stream+bytes, size-bytes, *iter, maxDepth-1);
			if (FailedToPrint(elementBytes, size-bytes)) {
				return elementBytes;
			}

			bytes += elementBytes;
		}

		elementBytes = snprintf(stream+bytes, size-bytes, "}");

		if (FailedToPrint(elementBytes, size-bytes)) {
			bytes = elementBytes;
			return bytes;
		} else {
			bytes += elementBytes;
		}
		
		return bytes;
	}

	template<class T>
	inline int Print(char * stream, unsigned size, const std::list<T> & data, unsigned maxDepth=INT_MAX)
	{
		int bytes = 0;
		int elementBytes = 0;

		if (maxDepth == 0) {
			bytes = snprintf(stream, size, "{size(%u)}", data.size());
			return bytes;
		}

		elementBytes = snprintf(stream+bytes, size-bytes, "{");

		if (FailedToPrint(elementBytes, size-bytes)) {
			bytes = elementBytes;
			return bytes;
		} else {
			bytes += elementBytes;
		}

		bool printedOne = false;
		size_t index = 0;
		for (typename std::list<T>::const_iterator iter = data.begin(); iter != data.end(); iter++, index++)
		{
			if (printedOne) {
				elementBytes = snprintf(stream+bytes, size-bytes, ", ");

				if (FailedToPrint(elementBytes, size-bytes)) {
					bytes = elementBytes;
					return bytes;
				} else {
					bytes += elementBytes;
				}
			} else {
				printedOne = true;
			}

			elementBytes = snprintf(stream+bytes, size-bytes, "[%u]", index);
			if (FailedToPrint(elementBytes, size-bytes)) {
				return elementBytes;
			}

			bytes += elementBytes;

			elementBytes = Print(stream+bytes, size-bytes, *iter, maxDepth-1);
			if (FailedToPrint(elementBytes, size-bytes)) {
				return elementBytes;
			}

			bytes += elementBytes;
		}

		elementBytes = snprintf(stream+bytes, size-bytes, "}");

		if (FailedToPrint(elementBytes, size-bytes)) {
			bytes = elementBytes;
			return bytes;
		} else {
			bytes += elementBytes;
		}
		
		return bytes;
	}

	template<class T>
	inline int Print(char * stream, unsigned size, const std::vector<T> & data, unsigned maxDepth=INT_MAX)
	{
		int bytes = 0;
		int elementBytes = 0;

		if (maxDepth == 0) {
			bytes = snprintf(stream, size, "{size(%u)}", data.size());
			return bytes;
		}

		elementBytes = snprintf(stream+bytes, size-bytes, "{");

		if (FailedToPrint(elementBytes, size-bytes)) {
			bytes = elementBytes;
			return bytes;
		} else {
			bytes += elementBytes;
		}

		bool printedOne = false;
		size_t index = 0;
		for (typename std::vector<T>::const_iterator iter = data.begin(); iter != data.end(); iter++, index++)
		{
			if (printedOne) {
				elementBytes = snprintf(stream+bytes, size-bytes, ", ");

				if (FailedToPrint(elementBytes, size-bytes)) {
					bytes = elementBytes;
					return bytes;
				} else {
					bytes += elementBytes;
				}
			} else {
				printedOne = true;
			}

			elementBytes = snprintf(stream+bytes, size-bytes, "[%u]", index);
			if (FailedToPrint(elementBytes, size-bytes)) {
				return elementBytes;
			}

			bytes += elementBytes;

			elementBytes = Print(stream+bytes, size-bytes, *iter, maxDepth-1);
			if (FailedToPrint(elementBytes, size-bytes)) {
				return elementBytes;
			}

			bytes += elementBytes;
		}

		elementBytes = snprintf(stream+bytes, size-bytes, "}");

		if (FailedToPrint(elementBytes, size-bytes)) {
			bytes = elementBytes;
			return bytes;
		} else {
			bytes += elementBytes;
		}
		
		return bytes;
	}
#endif

	////////////////////////////////////////////////////////////////////////////////
    
	template<class ValueType>
	class DECLSPEC AutoVariable : public AutoVariableBase
    {
		public:
			//AutoVariable() : AutoVariableBase(), mValue() {}
			AutoVariable() : AutoVariableBase() {} // rls - removed redundant default constructor for mValue
			explicit AutoVariable(const ValueType & source) : AutoVariableBase(), mValue(source) {}
			AutoVariable(const AutoVariable & copy) : AutoVariableBase(), mValue(copy.mValue) {}
			virtual ~AutoVariable() {}

			const ValueType &	Get() const { return mValue; }
			ValueType &			Get()	{ return mValue; }
			void				Set(const ValueType & rhs) { mValue = rhs; }

			virtual void Copy(AutoVariableBase *pSource) { AutoVariable<ValueType> *pOther = static_cast<AutoVariable<ValueType> *>(pSource); mValue = pOther->mValue; }

			virtual unsigned	Write(unsigned char * stream, unsigned size, unsigned version = 0) const { return soe::Write(stream, size, mValue, version); }
			virtual unsigned	Read(const unsigned char * stream, unsigned size, unsigned maxLen, unsigned version = 0) { return soe::Read(stream, size, mValue, maxLen, version); }
#ifdef PRINTABLE_MESSAGES 
			virtual int			Print(char * stream, unsigned size, unsigned maxDepth=INT_MAX) const
			{
				int bytes = snprintf(stream, size, "%s(", this->VariableName());
				int bytesTotal = 0;

				if (FailedToPrint(bytes, size)) {
					return bytes;
				}

				bytesTotal += bytes;
				bytes = soe::Print(stream+bytesTotal, size-bytesTotal, mValue, maxDepth);

				if (FailedToPrint(bytes, size-bytesTotal)) {
					return bytes;
				}

				bytesTotal += bytes;
				bytes = snprintf(stream+bytesTotal, size-bytesTotal, ")");

				if (FailedToPrint(bytes, size-bytesTotal)) {
					return bytes;
				}

				bytesTotal += bytes;

				return bytesTotal;
			}
#endif

		private:
			ValueType			mValue;
    };


	////////////////////////////////////////////////////////////////////////////////

	template<typename ClassType>
	class DECLSPEC MemberScribeBase
	{
    public:

		MemberScribeBase(const char * variableName, unsigned version = 0, EVersionEffect effect = eNoEffect) 
			: mVariableName(variableName)
			, mVersion(version)
			, mEffect(effect)
				{}

		virtual ~MemberScribeBase() {}

		virtual unsigned Read(const unsigned char * stream, unsigned size, ClassType & data, unsigned version = 0) const = 0;
		virtual unsigned Write(unsigned char * stream, unsigned size, const ClassType & data, unsigned version = 0) const = 0;
		virtual int	Print(char * stream, unsigned size, const ClassType & data, unsigned maxDepth=INT_MAX) const = 0;

		inline const char * VariableName() const { return mVariableName; }
		inline unsigned Version() const { return mVersion; }
		inline EVersionEffect Effect() const { return mEffect; }

	protected:
		const char *mVariableName;
		unsigned mVersion;
		EVersionEffect mEffect;
	};
   
	////////////////////////////////////////////////////////////////////////////////

	template<typename ClassType, typename MemberType>
	class DECLSPEC MemberScribe : public MemberScribeBase<ClassType>
	{
	public:
		typedef MemberType ClassType::*MemberPointer_t;

		MemberScribe(const char * variableName, MemberPointer_t memberPointer, unsigned size, unsigned version, EVersionEffect effect) 
			: MemberScribeBase<ClassType>(variableName, version, effect)
			, mMemberPointer(memberPointer)
			, mSize(size)
				{}

		virtual ~MemberScribe() {}

		virtual unsigned Read(const unsigned char * stream, unsigned size, ClassType & data, unsigned version = 0) const
		{
			return soe::Read(stream, size, data.*mMemberPointer, mSize, version);
		}
		virtual unsigned Write(unsigned char * stream, unsigned size, const ClassType & data, unsigned version = 0) const
		{
			return soe::Write(stream, size, data.*mMemberPointer, version);
		}
		virtual int	Print(char * stream, unsigned size, const ClassType & data, unsigned maxDepth=INT_MAX) const
		{
#ifdef PRINTABLE_MESSAGES
			return soe::Print(stream, size, data.*mMemberPointer, maxDepth);
#else
			return 0;
#endif
		}

	protected:
		MemberPointer_t mMemberPointer;
		unsigned mSize;
	};

	// For use with list/vector reading with string/vector/list (anything with maxSize) elements
	template<typename ClassType, typename MemberType>
	class DECLSPEC MemberScribeWithElements : public MemberScribe<ClassType, MemberType>
	{
	public:
		MemberScribeWithElements(const char * variableName, typename MemberScribe<ClassType, MemberType>::MemberPointer_t memberPointer, unsigned size, unsigned elementSize, unsigned version = 0, EVersionEffect effect = eNoEffect) 
			: MemberScribe<ClassType, MemberType>(variableName, memberPointer, size, version, effect)
			, mElementSize(elementSize)
		{}

		virtual ~MemberScribeWithElements() {}

		// overrides read function *only*; parent behavior OK for others
		virtual unsigned Read(const unsigned char * stream, unsigned size, ClassType & data, unsigned version = 0) const
		{
			return soe::ReadContainer(stream, size, data.*MemberScribe<ClassType, MemberType>::mMemberPointer, MemberScribe<ClassType, MemberType>::mSize, mElementSize, version);
		}
	protected:
		unsigned mElementSize;
	};
	
	class DECLSPEC ClassScribeBase
	{
	public:
		ClassScribeBase(const char * className) : mClassName(className), mInitialized(false) { }
		virtual ~ClassScribeBase() { }

		inline bool IsInitialized() const { return mInitialized; }
		inline void SetInitialized(bool initialized) { mInitialized = initialized; }

		const char * ClassName() const { return mClassName; }

		virtual void ClearMembers() = 0;
		virtual size_t CountMembers() const = 0;

	protected:
		const char * mClassName;

	private:
		bool mInitialized;
	};

	template<typename ClassType>
	class DECLSPEC ClassScribe : public ClassScribeBase
	{
	protected:
		typedef MemberScribeBase<ClassType> MemberScribe_t;
		typedef std::vector<MemberScribe_t *> MemberScribeVector_t;

	public:
		ClassScribe(const char * className) : ClassScribeBase(className) { }
		~ClassScribe() { }

		unsigned Read(const unsigned char * stream, unsigned size, ClassType & data, unsigned version) const;
		unsigned Write(unsigned char * stream, unsigned size, const ClassType & data, unsigned version) const;
		int	Print(char * stream, unsigned size, const ClassType & data, unsigned maxDepth) const;

		virtual void ClearMembers()
		{
			typename MemberScribeVector_t::const_iterator memberIter;
			for (memberIter = mMemberScribes.begin(); memberIter != mMemberScribes.end(); memberIter++)
			{
				MemberScribe_t * pMemberScribe = *memberIter;
				delete pMemberScribe;
			}
			mMemberScribes.clear();
		}

		virtual size_t CountMembers() const { return mMemberScribes.size(); }

		template<typename MemberType>
		void AddMember(const char * variableName, MemberType ClassType:: * memberPointer, unsigned size, unsigned version, EVersionEffect effect)
		{
			MemberScribeBase<ClassType> * pMemberScribe = new MemberScribe<ClassType, MemberType>(variableName, memberPointer, size, version, effect);
			mMemberScribes.push_back(pMemberScribe);
		}

		template<typename MemberType>
		void AddMemberContainer(const char * variableName, MemberType ClassType:: * memberPointer, unsigned size , unsigned elementSize, unsigned version, EVersionEffect effect)
		{
			MemberScribeBase<ClassType> * pMemberScribe = new MemberScribeWithElements<ClassType, MemberType>(variableName, memberPointer, size, elementSize, version, effect);
			mMemberScribes.push_back(pMemberScribe);
		}

	private:
		MemberScribeVector_t mMemberScribes;
	};

	template<typename ClassType>
	unsigned ClassScribe<ClassType>::Read(const unsigned char * stream, unsigned size, ClassType & data, unsigned version) const
	{
		unsigned bytesTotal = 0;
		typename MemberScribeVector_t::const_iterator memberIter;

		for (memberIter = mMemberScribes.begin(); memberIter != mMemberScribes.end(); memberIter++)
		{
			MemberScribe_t * pMemberScribe = *memberIter;

			if (IsMemberIncluded(version, pMemberScribe->Version(), pMemberScribe->Effect()))
			{
				unsigned bytes = pMemberScribe->Read(stream, size, data, version);
				if (bytes == 0)
				{
#ifdef TRACK_READ_WRITE_FAILURES
					soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to read %s.%s, size = %u, version = %u", __FUNCTION__, __FILE__, __LINE__, ClassName(), pMemberScribe->VariableName(), size, version));
#endif
					bytesTotal = 0;
					break;
				}
				else
				{
					stream += bytes;
					size -= bytes;
					bytesTotal += bytes;
				}
			}
		}

		return bytesTotal;
	}

	template<typename ClassType>
	unsigned ClassScribe<ClassType>::Write(unsigned char * stream, unsigned size, const ClassType & data, unsigned version) const
	{
		unsigned bytesTotal = 0;
		typename MemberScribeVector_t::const_iterator memberIter;

		for (memberIter = mMemberScribes.begin(); memberIter != mMemberScribes.end(); memberIter++)
		{
			const MemberScribe_t * pMemberScribe = *memberIter;

			if (IsMemberIncluded(version, pMemberScribe->Version(), pMemberScribe->Effect()))
			{
				unsigned bytes = pMemberScribe->Write(stream, size, data, version);
				if (bytes == 0)
				{
#ifdef TRACK_READ_WRITE_FAILURES
					soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to read %s.%s, size = %u, version = %u", __FUNCTION__, __FILE__, __LINE__, ClassName(), pMemberScribe->VariableName(), size, version));
#endif
					bytesTotal = 0;
					break;
				}
				else
				{
					stream += bytes;
					size -= bytes;
					bytesTotal += bytes;
				}
			}
		}

		return bytesTotal;
	}

	template<typename ClassType>
	int	ClassScribe<ClassType>::Print(char * stream, unsigned size, const ClassType & data, unsigned maxDepth) const
	{
		int bytesTotal = 0;

#ifdef PRINTABLE_MESSAGES
		int bytes = 0;

		if (maxDepth == 0) {
			bytes = snprintf(stream, size, "%s{mMembers(%u)}", ClassName(), mMemberScribes.size());
			return bytes;
		}

		bytes = snprintf(stream, size, "{");

		if (soe::FailedToPrint(bytes, size)) {
			bytesTotal = bytes;
			return bytesTotal;
		} else {
			stream += bytes;
			size -= bytes;
			bytesTotal += bytes;
		}

		bool printedOne = false;
		typename MemberScribeVector_t::const_iterator memberIter;

		for (memberIter = mMemberScribes.begin(); memberIter != mMemberScribes.end(); memberIter++)
		{
			int bytes = 0;

			if (printedOne) {
				bytes = snprintf(stream, size, ", ");
				if (soe::FailedToPrint(bytes, size)) {
					bytesTotal = bytes;
					return bytesTotal;
				} else {
					stream += bytes;
					size -= bytes;
					bytesTotal += bytes;
				}
			} else {
				printedOne = true;
			}

			const MemberScribe_t * pMemberScribe = *memberIter;

			bytes = pMemberScribe->Print(stream, size, maxDepth-1);
			if (soe::FailedToPrint(bytes, size))
			{
				bytesTotal = bytes;
				return bytesTotal;
			}
			else
			{
				stream += bytes;
				size -= bytes;
				bytesTotal += bytes;
			}
		}
#endif

		return bytesTotal;
	}

	////////////////////////////////////////////////////////////////////////////////
}

#endif
