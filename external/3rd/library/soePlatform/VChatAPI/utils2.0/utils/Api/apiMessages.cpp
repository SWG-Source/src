#pragma warning (disable: 4514 4786)

#include "apiMessages.h"


namespace API_NAMESPACE
{


	namespace Message
	{


////////////////////////////////////////////////////////////////////////////////

	const std::map<unsigned, Base::DeepPointer> & Base::GetMessageMap() { return *ms_pClassMap; }

	//	returns the number of bytes Read, or zero if an error occured
	unsigned Base::Read(const unsigned char * stream, unsigned size, unsigned version)
	{
		unsigned bytesTotal = 0;
		bool succeeded = true;
		MemberVector_t::iterator iterator;
#ifdef TRACK_READ_WRITE_FAILURES
		soe::ClearMessageFailureStack();
#endif
		for(iterator=mMembers.begin(); iterator!=mMembers.end(); iterator++)
		{
			MemberInfo_t & memberInfo = *iterator;

			if (IsMemberIncluded(version, memberInfo.version, memberInfo.effect))
			{
				unsigned bytes = memberInfo.data->Read(stream, size, memberInfo.size, version);
				if (bytes == 0)
				{
#ifdef TRACK_READ_WRITE_FAILURES
					soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to read %s.%s, size = %u, version = %u", __FUNCTION__, __FILE__, __LINE__, MessageName(), memberInfo.data->VariableName(), size, version));
#endif
					bytesTotal = 0;
					succeeded = false;
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
		if (succeeded) 
		{
			for(iterator=mOptionalMembers.begin(); iterator!=mOptionalMembers.end(); iterator++)
			{
				MemberInfo_t & memberInfo = *iterator;
				unsigned bytes = memberInfo.data->Read(stream, size, memberInfo.size, version);
				if (bytes == 0)
				{
					// break, but don't fail the reading process
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

	//	returns the number of bytes written, or zero if an error occured
    unsigned Base::Write(unsigned char * stream, unsigned size, unsigned version) const
    {
		unsigned bytesTotal = 0;
		bool succeeded = true;
		MemberVector_t::const_iterator iterator;
#ifdef TRACK_READ_WRITE_FAILURES
		soe::ClearMessageFailureStack();
#endif

	    for (iterator=mMembers.begin(); iterator!=mMembers.end(); iterator++)
	    {
			const MemberInfo_t & memberInfo = *iterator;

			if (IsMemberIncluded(version, memberInfo.version, memberInfo.effect))
			{
				unsigned bytes = memberInfo.data->Write(stream, size, version);
				if (bytes == 0)
				{
#ifdef TRACK_READ_WRITE_FAILURES
					soe::PushMessageFailure(soe::PrintToString("%s, %s:%u - failed to write %s.%s, size = %u, version = %u", __FUNCTION__, __FILE__, __LINE__, MessageName(), memberInfo.data->VariableName(), size, version));
#endif
					bytesTotal = 0;
					succeeded = false;
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
		if (succeeded)
		{
			for (iterator=mOptionalMembers.begin(); iterator!=mOptionalMembers.end(); iterator++)
			{
				const MemberInfo_t & memberInfo = *iterator;
				unsigned bytes = memberInfo.data->Write(stream, size, version);
				if (bytes == 0)
				{
					// break, but don't fail the writing process
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

#ifdef PRINTABLE_MESSAGES
	int Base::Print(char * stream, unsigned size, unsigned maxDepth) const
	{
		int bytesTotal = 0;
		int bytes = 0;

		if (maxDepth == 0) {
			bytes = snprintf(stream, size, "%s{mMembers(%u), mOptionalMembers(%u)}", MessageName(), mMembers.size(), mOptionalMembers.size());
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
		MemberVector_t::const_iterator iterator;
		for (iterator=mMembers.begin(); iterator!=mMembers.end(); iterator++)
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

			const MemberInfo_t & memberInfo = *iterator;

			bytes = memberInfo.data->Print(stream, size, maxDepth-1);
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
		for (iterator=mOptionalMembers.begin(); iterator!=mOptionalMembers.end(); iterator++)
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

			const MemberInfo_t & memberInfo = *iterator;
			bytes = memberInfo.data->Print(stream, size, maxDepth-1);
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
		if (!soe::FailedToPrint(bytes, size)) {
			bytes = snprintf(stream, size, "}");

			if (soe::FailedToPrint(bytes, size)) {
				bytesTotal = bytes;
			} else {
				stream += bytes;
				size -= bytes;
				bytesTotal += bytes;
			}
		}
		return bytesTotal;
	}
#endif

	void * Base::operator new(size_t size)
	{
		return ::operator new(size);
	}

	void * Base::operator new(size_t size, void *ptr)
	{
		return ::operator new(size, ptr);
	}

	Base * Base::Clone() const
	{
		return new Base(*this);
	}

	Base * Base::Clone(StorageVector_t &storageVector) const
	{
		return new(storageVector) Base(*this);
	}

	Base * Base::Create(unsigned MsgId, StorageVector_t *pStorageVector)
	{
		Base * msg = nullptr;

		if (ms_pClassMap) {
			std::map<unsigned, Base::DeepPointer>::iterator classIter = ms_pClassMap->find(MsgId);

			if (classIter != ms_pClassMap->end()) {
				if (pStorageVector) {
					msg = classIter->second->Clone(*pStorageVector);
				} else {
					msg = classIter->second->Clone();
				}
			}
		}

		return msg;
	}

	const char	* Base::GetMessageName(unsigned msgId)
	{
		const char * requestString = nullptr;

		if (ms_pClassMap) {
			std::map<unsigned, Base::DeepPointer>::iterator classIter = ms_pClassMap->find(msgId);

			if (classIter != ms_pClassMap->end()) {
				requestString = classIter->second->MessageName();
			}
		}

		return requestString;
	}

	const char	* Base::GetMessageIDString(unsigned msgId)
	{
		const char * idString = nullptr;

		if (ms_pClassMap) {
			std::map<unsigned, Base::DeepPointer>::iterator classIter = ms_pClassMap->find(msgId);

			if (classIter != ms_pClassMap->end()) {
				idString = classIter->second->MessageIDString();
			}
		}

		return idString;
	}

	void Base::AddClassMapEntry(unsigned msgId, const Base * msg)
	{
		static std::map<unsigned, Base::DeepPointer> classMap;

		if (ms_pClassMap == nullptr) {
			ms_pClassMap = &classMap;
		}

		std::map<unsigned, Base::DeepPointer>::iterator classIter = ms_pClassMap->find(msgId);

		if (classIter == ms_pClassMap->end()) {
			(*ms_pClassMap)[msgId] = msg;
		}
	}

	void * Base::operator new(size_t storageSize, StorageVector_t &storageVector)
	{
		storageSize = (storageSize / sizeof(Storage_t)) + 1;
		if (storageVector.size() < storageSize) {
			storageVector.resize(storageSize);
		}

		return &storageVector[0];
	}

	Base::DeepPointer::DeepPointer() :
		m_ptr(nullptr)
	{
	}

	Base::DeepPointer::DeepPointer(const Base & source) :
		m_ptr(nullptr)
	{
		m_ptr = source.Clone(m_storageVector);
	}

	Base::DeepPointer::DeepPointer(const Base * source) :
		m_ptr(nullptr)
	{
		m_ptr = source->Clone(m_storageVector);
	}

	Base::DeepPointer::DeepPointer(const DeepPointer & source) :
		m_ptr(nullptr)
	{
		if (source.m_ptr) { m_ptr = source->Clone(m_storageVector); }
	}

	Base::DeepPointer::~DeepPointer()
	{
		if (m_ptr) { m_ptr->~Base(); }
	}

	void Base::DeepPointer::SetMessageType(unsigned MsgId)
	{
		if (m_ptr) { m_ptr->~Base(); }

		m_ptr = Create(MsgId, &m_storageVector);
	}

	Base::DeepPointer & Base::DeepPointer::operator=(const Base * rhs)
	{
		Base *oldPtr = m_ptr;

		m_ptr = rhs ? rhs->Clone(m_storageVector) : nullptr;

		if (oldPtr) {
			oldPtr->~Base();
		}

		return *this;
	}

	Base::DeepPointer & Base::DeepPointer::operator=(const DeepPointer & rhs)
	{
		Base *oldPtr = m_ptr;
		
		m_ptr = rhs.m_ptr ? rhs->Clone(m_storageVector) : nullptr;

		if (oldPtr) {
			oldPtr->~Base();
		}

		return *this;
	}

	Base * Base::DeepPointer::operator->() const
	{
		return m_ptr;
	}

	Base & Base::DeepPointer::operator*() const
	{
		return *m_ptr;
	}

	std::map<unsigned, Base::DeepPointer> *Base::ms_pClassMap = nullptr;

#if defined(PRINTABLE_MESSAGES) || defined(TRACK_READ_WRITE_FAILURES)
	Basic::Basic()
	{
		mMsgId.Set(0);
		Insert(mMsgId);
        mConnectId.Set(0);
        Insert(mConnectId);
	}

	Basic::Basic(const unsigned messageId, const unsigned connectId)
	{
		mMsgId.Set(messageId);
		Insert(mMsgId);
        mConnectId.Set(connectId);
        Insert(mConnectId);
	}
#else
	Basic::Basic() :
		mMsgId(0),
        mConnectId(0)

	{
		Insert(mMsgId);
        Insert(mConnectId);
	}

	Basic::Basic(const unsigned messageId, const unsigned connectId) :
		mMsgId(messageId),
        mConnectId(connectId)

	{
		Insert(mMsgId);
        Insert(mConnectId);
	}
#endif

	Basic::~Basic()
	{
	}

	Basic::Basic(const Basic & source) :
		mMsgId(source.mMsgId),
        mConnectId(source.mConnectId)
	{
		// do not copy member pointers - would lead to dangling pointers

		Insert(mMsgId);
        Insert(mConnectId);
	}

	Basic & Basic::operator=(const Basic & rhs)
	{
		// do not copy member pointers - would lead to dangling pointers

		mMsgId = rhs.mMsgId;
        mConnectId = rhs.mConnectId;

		return *this;
	}

////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////
    //	NameValuePair
    ImplementMessageBegin(NameValuePair, Basic)
        ImplementMessageMemberEx(Name, std::string(), 256)
        ImplementMessageMemberEx(Value, std::string(), 512)
    ImplementMessageEnd

	////////////////////////////////////////
    //  Connect
    ImplementMessageBegin(Connect, Basic)
        ImplementMessageMemberEx(Version, std::string(), 30)
        ImplementMessageMemberEx(Description, std::string(), 64)
        
    ImplementMessageEnd


    ////////////////////////////////////////
    //  ConnectReply
    ImplementMessageBegin(ConnectReply, Basic)
        ImplementMessageMember(Result, 0)
		ImplementMessageMemberEx(Version, std::string(), 30)
    ImplementMessageEnd


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  Tracked
    ImplementMessageBegin(Tracked, Basic)
		ImplementMessageMember(TrackingNumber, 0)
    ImplementMessageEnd


    ////////////////////////////////////////
    //  TrackedReply
    ImplementMessageBegin(TrackedReply, Tracked)
		ImplementMessageMember(Result, 0)
    ImplementMessageEnd

	////////////////////////////////////////
    //  TrackedReplyEx
    ImplementMessageBegin(TrackedReplyEx, TrackedReply)
		ImplementMessageMember(Status, 0)
    ImplementMessageEnd

    ////////////////////////////////////////
    //  TrackedInterimReply
    ImplementMessageBegin(TrackedInterimReply, TrackedReply)
        ImplementMessageMember(ParentTrackingNumber, 0)
    ImplementMessageEnd

    ////////////////////////////////////////
    //  GetStatus
    ImplementMessageBegin(GetStatus, Tracked)
    ImplementMessageEnd

    ////////////////////////////////////////
    //  GetStatusReply
    ImplementMessageBegin(GetStatusReply, TrackedReply)
		ImplementMessageMemberEx(NameValuePairs, std::vector<NameValuePair>(), 100000)
    ImplementMessageEnd

	////////////////////////////////////////
	//  ShutdownNotify
	ImplementMessageBegin(ShutdownNotify, Basic)
		ImplementMessageMember(UnfinishedRequests, 0)
	ImplementMessageEnd

	ImplementMessageBegin(ShutdownNotifyReply, Basic)
		ImplementMessageMember(UnfinishedRequests, 0)
	ImplementMessageEnd

////////////////////////////////////////////////////////////////////////////////


	}    

    void Read(soe::NameValuePair &dest, const Message::NameValuePair &src)
    {
        dest.name = src.GetName();
        dest.value = src.GetValue();
    }

    void Write(const soe::NameValuePair &src, Message::NameValuePair &dest)
    {
        dest.SetName(src.name);
        dest.SetValue(src.value);
    }
}

