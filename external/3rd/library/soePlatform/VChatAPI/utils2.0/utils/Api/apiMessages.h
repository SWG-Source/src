//	Must protect from multiple definition from outside this header
//#ifndef _API_MESSAGES_H
//#define _API_MESSAGES_H


#ifndef API_NAMESPACE
#pragma message ("apiMessages.h: API_NAMESPACE undefined")
//#else
//#pragma message ("apiMessages.h: API_NAMESPACE:")
//#pragma message API_NAMESPACE
#endif

#include <map>
#include <vector>
#include "Base/serializeClasses.h"
//#include "Base/serialize.h"
#include "apiMacros.h"
#include "apiTypeNameValuePair.h"

// Storage *will* be freed if an exception is thrown in the constructor,
// so disable that warning here.
#pragma warning(disable:4291)

////////////////////////////////////////////////////////////////////////////////

#ifdef API_NAMESPACE
namespace API_NAMESPACE
{
#endif

	enum
    {
        MESSAGE_NULL                                = 0x0000,
        MESSAGE_NAME_VALUE_PAIR                     = 0x0001,

        //  Client Messages
        MESSAGE_CONNECT                             = 0x0ff3,
        MESSAGE_GET_STATUS                          = 0x0ff4,
		MESSAGE_GET_CONFIGURATION                   = 0x0ff5,

		MESSAGE_SHUTDOWN_NOTIFY						= 0x0f00,

        //  Server Messages
        MESSAGE_CONNECT_REPLY                       = 0x7ff3,
        MESSAGE_GET_STATUS_REPLY                    = 0x7ff4,
		MESSAGE_GET_CONFIGURATION_REPLY             = 0x7ff5,

		MESSAGE_SHUTDOWN_NOTIFY_REPLY				= 0x7f00
    };


	namespace Message
	{


////////////////////////////////////////////////////////////////////////////////

	class DECLSPEC Base
	{
	protected:
		struct DECLSPEC MemberInfo
		{
			MemberInfo(soe::AutoVariableBase *dataIn = nullptr, unsigned sizeIn = 1,  unsigned versionIn = 0, soe::EVersionEffect effectIn = soe::eNoEffect)
				: data(dataIn)
				, size(sizeIn)
				, version(versionIn)
				, effect(effectIn)
					{ }

			soe::AutoVariableBase *data;
			unsigned size;
			unsigned version;
			soe::EVersionEffect effect;
		};
		typedef struct MemberInfo MemberInfo_t;
		// typedef std::pair<soe::AutoVariableBase *, unsigned> MemberInfo_t;
		typedef std::vector<MemberInfo_t> MemberVector_t;

	public:

		typedef soe::uint32 Storage_t;
		typedef std::vector<Storage_t> StorageVector_t;

		Base() { }
		virtual ~Base() { }
		void * operator new(size_t size);
		void * operator new(size_t size, void *ptr);

		void                Insert(soe::AutoVariableBase & member, unsigned size = 1, unsigned version = 0, soe::EVersionEffect effect = soe::eNoEffect);
		void                InsertOptional(soe::AutoVariableBase & member, unsigned size = 1, unsigned version = 0, soe::EVersionEffect effect = soe::eNoEffect);
		virtual unsigned    Write(unsigned char * stream, unsigned size, unsigned version = 0) const;
		virtual unsigned    Read(const unsigned char * stream, unsigned size, unsigned version = 0);
#ifdef PRINTABLE_MESSAGES
		virtual int			Print(char * stream, unsigned size, unsigned maxDepth = INT_MAX) const;
#endif

		virtual Base		* Clone() const;
		virtual Base		* Clone(StorageVector_t &storageVector) const;
		virtual const char	* MessageName() const { return "Base"; }
		virtual const char	* MessageIDString() const { return "nullptr"; }
		static Base			* Create(unsigned MsgId, StorageVector_t *pStorageVector = nullptr);
		static const char	* GetMessageName(unsigned msgId);
		static const char	* GetMessageIDString(unsigned msgId);

		class DECLSPEC DeepPointer
		{
		public:
			DeepPointer();
			DeepPointer(const Base & source);
			explicit DeepPointer(const Base * source);
			DeepPointer(const DeepPointer & source);
			virtual ~DeepPointer();

			void SetMessageType(unsigned MsgId);

			DeepPointer & operator=(const Base * rhs);
			DeepPointer & operator=(const DeepPointer & rhs);

			Base * operator->() const;
			Base & operator*() const;

			operator Base * () const { return m_ptr; }

		private:
			Base *m_ptr;
			StorageVector_t m_storageVector;
		};

		static const std::map<unsigned, DeepPointer> & GetMessageMap();

	protected:
		Base(const Base & source) { }
		Base & operator=(const Base & rhs) { return *this; }
		void * operator new(size_t storageSize, StorageVector_t &storageVector);
	public:
		static void AddClassMapEntry(unsigned msgId, const Base * msg);
	protected:
		MemberVector_t mMembers;
		MemberVector_t mOptionalMembers;

		static std::map<unsigned, DeepPointer> *ms_pClassMap;
	};

	}


#ifdef API_NAMESPACE
}
#endif

namespace soe
{
    inline unsigned Write(unsigned char * stream, unsigned size, const API_NAMESPACE::Message::Base & data, unsigned version = 0)
    {
        unsigned written = 0;
        unsigned writtenSize = Write(stream, size, written, version);

        if (writtenSize <= 0) { return 0; }
        written = data.Write(stream + writtenSize, size - writtenSize, version);
        if (written <= 0) { return 0; }
        writtenSize = Write(stream, size, written, version);

        return (writtenSize + written);
    }

    inline unsigned Read(const unsigned char * stream, unsigned size, API_NAMESPACE::Message::Base & data, unsigned, unsigned version = 0)
    {
        unsigned messageSize = 0;
        unsigned readSize = Read(stream, size, messageSize, version);
        unsigned readIn = 0;

        size -= readSize;
        stream += readSize;
        if (messageSize > size) { return 0; }
        readIn = data.Read(stream, messageSize, version);
        if (readIn <= 0) {  return 0; }

        return (readSize + messageSize);
    }

    inline unsigned Write(unsigned char * stream, unsigned size, const API_NAMESPACE::Message::Base::DeepPointer & data, unsigned version = 0)
    {
        API_NAMESPACE::Message::Base *msg = (API_NAMESPACE::Message::Base *)data;

        if (msg) {
            return Write(stream, size, *msg, version);
        } else {
            return 0;
        }
    }

    inline unsigned Read(const unsigned char * stream, unsigned size, API_NAMESPACE::Message::Base::DeepPointer & data, unsigned, unsigned version = 0)
    {
        unsigned msgSize = 0;
        unsigned msgId = 0;
        unsigned sizeSize = Read(stream, size, msgSize, version);
        unsigned idSize = (sizeSize > 0) ? Read(stream + sizeSize, size - sizeSize, msgId, version) : 0;

        if (idSize <= 0) {
            return 0;
        }

        data.SetMessageType(msgId);

        API_NAMESPACE::Message::Base *msg = data;

        if (msg) {
            return Read(stream, size, *msg, 0, version);
        } else {
            return 0;
        }
    }

#ifdef PRINTABLE_MESSAGES
    inline int Print(char * stream, unsigned size, const API_NAMESPACE::Message::Base & data, unsigned maxDepth=INT_MAX)
    {
        return data.Print(stream, size, maxDepth);

    }

    inline int Print(char * stream, unsigned size, const API_NAMESPACE::Message::Base::DeepPointer & data, unsigned maxDepth=INT_MAX)
    {
        API_NAMESPACE::Message::Base *msg = (API_NAMESPACE::Message::Base *)data;

        if (msg) {
            return Print(stream, size, *msg, maxDepth);
        } else {
            return 0;
        }
    }
#endif

}


#include "Base/serializeTemplates.h"

#ifdef API_NAMESPACE
namespace API_NAMESPACE
{
#endif
    namespace Message
	{

	class DECLSPEC Basic : public Base
    {
        public:
            Basic();
            Basic(const unsigned messageId, const unsigned connectId=0);
            virtual ~Basic();

			const unsigned		GetMsgId() const;
            const unsigned		GetConnectionId() const;
            void                SetConnectionId(const unsigned connectId);

			virtual const char * MessageName() const { return "Basic"; }
			virtual const char * MessageIDString() const { return "NULL_2"; }

        protected:
			Basic(const Basic & source);
			Basic & operator=(const Basic & rhs);
#if defined(PRINTABLE_MESSAGES) || defined(TRACK_READ_WRITE_FAILURES)
		public:
			class DECLSPEC PrintMsgId : public soe::AutoVariable<unsigned>
			{
				public:
					virtual const char * VariableName() const { return "MsgId"; }
#	ifdef PRINTABLE_MESSAGES
					virtual int	Print(char * stream, unsigned size, unsigned maxDepth=INT_MAX) const
					{
						int bytes = snprintf(stream, size, "%s(", this->VariableName());
						int bytesTotal = 0;

						if (soe::FailedToPrint(bytes, size)) {
							return bytes;
						}

						bytesTotal += bytes;
						bytes = snprintf(stream+bytesTotal, size, "0x%4.4X", Get());

						if (soe::FailedToPrint(bytes, size)) {
							return bytes;
						}

						bytesTotal += bytes;
						bytes = snprintf(stream+bytesTotal, size-bytesTotal, ")");

						if (soe::FailedToPrint(bytes, size-bytesTotal)) {
							return bytes;
						}

						bytesTotal += bytes;

						return bytesTotal;
					}
#	endif	// PRINTABLE_MESSAGES
			};
			class PrintConnectionId : public soe::AutoVariable<unsigned>
			{
			public:
				virtual const char * VariableName() const { return "CID"; }
#	ifdef PRINTABLE_MESSAGES
				virtual int	Print(char * stream, unsigned size, unsigned maxDepth=INT_MAX) const
				{
					int bytesTotal = 0;

					if (Get()) {
						int bytes = snprintf(stream, size, "%s(", this->VariableName());

						if (soe::FailedToPrint(bytes, size)) {
							return bytes;
						}

						bytesTotal += bytes;
						bytes = snprintf(stream+bytesTotal, size, "0x%4.4X", Get());

						if (soe::FailedToPrint(bytes, size)) {
							return bytes;
						}

						bytesTotal += bytes;
						bytes = snprintf(stream+bytesTotal, size-bytesTotal, ")");

						if (soe::FailedToPrint(bytes, size-bytesTotal)) {
							return bytes;
						}

						bytesTotal += bytes;
					}
					return bytesTotal;
				}
#	endif	// PRINTABLE_MESSAGES
			};
		protected:
			PrintMsgId mMsgId;
            PrintConnectionId mConnectId;
#else
			soe::AutoVariable<unsigned> mMsgId;
            soe::AutoVariable<unsigned> mConnectId;
#endif	// defined(PRINTABLE_MESSAGES) || defined(TRACK_READ_WRITE_FAILURES)
    };


	inline void Base::Insert(soe::AutoVariableBase & member, unsigned size, unsigned version, soe::EVersionEffect effect)
    {
	    mMembers.push_back(MemberInfo_t(&member,size,version,effect));
    }

	inline void Base::InsertOptional(soe::AutoVariableBase & member, unsigned size, unsigned version, soe::EVersionEffect effect)
	{
		mOptionalMembers.push_back(MemberInfo_t(&member,size,version,effect));
	}

    inline const unsigned Basic::GetMsgId() const
    {
        return mMsgId.Get();
    }

    inline const unsigned Basic::GetConnectionId() const
    {
        return mConnectId.Get();
    }

    inline void Basic::SetConnectionId(const unsigned connectId)
    {
        mConnectId.Set(connectId);
		mMembers[1] = MemberInfo_t(&mConnectId,1,0,soe::eNoEffect);
    }

////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  NameValuePair
    DefineMessageBegin(NameValuePair, Basic, MESSAGE_NAME_VALUE_PAIR)
        DefineMessageMember(Name, std::string)
        DefineMessageMember(Value, std::string)
    DefineMessageEnd

    ////////////////////////////////////////
    //  Connect
    DefineMessageBegin(Connect, Basic, MESSAGE_CONNECT)
        DefineMessageMember(Version, std::string)
        DefineMessageMember(Description, std::string)

    DefineMessageEnd


    ////////////////////////////////////////
    //  ConnectReply
    DefineMessageBegin(ConnectReply, Basic, MESSAGE_CONNECT_REPLY)
        DefineMessageMember(Result, unsigned char)
		DefineMessageMember(Version, std::string)
    DefineMessageEnd

////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////
    //  Tracked
    DefineMessageBegin(Tracked, Basic, MESSAGE_NULL)
        DefineMessageMember(TrackingNumber, unsigned)
    DefineMessageEnd


    ////////////////////////////////////////
    //  TrackedReply
    DefineMessageBegin(TrackedReply, Tracked, MESSAGE_NULL)
        DefineMessageMember(Result, unsigned)
    DefineMessageEnd

    ////////////////////////////////////////
    //  TrackedReplyEx
    DefineMessageBegin(TrackedReplyEx, TrackedReply, MESSAGE_NULL)
		DefineMessageMember(Status, unsigned)
    DefineMessageEnd

    ////////////////////////////////////////
    //  TrackedInterimReply
    DefineMessageBegin(TrackedInterimReply, TrackedReply, MESSAGE_NULL)
        DefineMessageMember(ParentTrackingNumber, unsigned)
    DefineMessageEnd


    ////////////////////////////////////////
    //  GetStatus
    DefineMessageBegin(GetStatus, Tracked, MESSAGE_GET_STATUS)
    DefineMessageEnd

    DefineMessageBegin(GetStatusReply, TrackedReply, MESSAGE_GET_STATUS_REPLY)
		DefineMessageMember(NameValuePairs, std::vector<NameValuePair>)
    DefineMessageEnd

	////////////////////////////////////////
	//  ShutdownNotify
	DefineMessageBegin(ShutdownNotify, Basic, MESSAGE_SHUTDOWN_NOTIFY)
		DefineMessageMember(UnfinishedRequests, unsigned)
	DefineMessageEnd

	DefineMessageBegin(ShutdownNotifyReply, Basic, MESSAGE_SHUTDOWN_NOTIFY_REPLY)
		DefineMessageMember(UnfinishedRequests, unsigned)
	DefineMessageEnd

////////////////////////////////////////////////////////////////////////////////


	}

    DECLSPEC void Read(soe::NameValuePair &dest, const Message::NameValuePair &src);
    DECLSPEC void Write(const soe::NameValuePair &src, Message::NameValuePair &dest);

#ifdef API_NAMESPACE
}
#endif

namespace soe {

    inline unsigned Read(const unsigned char * stream, unsigned size, API_NAMESPACE::Message::NameValuePair & data, unsigned maxLen, unsigned version = 0)
    {
        NameValuePair dest;
        unsigned returnval = Read(stream, size, dest, maxLen, version);
        API_NAMESPACE::Write(dest, data);

        return returnval;
    }

    inline unsigned Write(unsigned char * stream, unsigned size, const API_NAMESPACE::Message::NameValuePair & data, unsigned version = 0)
    {
        NameValuePair dest;
        API_NAMESPACE::Read(dest, data);

        return Write(stream, size, dest, version);
    }

}

//#endif  //_API_MESSAGES_H
