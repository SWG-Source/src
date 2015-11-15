#ifndef COMMON_API__COMMON_MESSAGES_H
#define COMMON_API__COMMON_MESSAGES_H


#include <vector>
#include "Base/Archive.h"
#include "CommonAPI.h"


////////////////////////////////////////////////////////////////////////////////


namespace Base 
{

    /*
    void  get(ByteStream::ReadIterator & source, apiAccountStatus & target);
    void  get(ByteStream::ReadIterator & source, apiSessionType & target);
    void  get(ByteStream::ReadIterator & source, apiGamecode & target);
    void  get(ByteStream::ReadIterator & source, apiSubscriptionStatus & target);
    void  get(ByteStream::ReadIterator & source, apiKickReason & target);
    void  get(ByteStream::ReadIterator & source, apiKickResult & target);
    */
    void  get(ByteStream::ReadIterator & source, apiAccount_v1 & target);
    void  get(ByteStream::ReadIterator & source, apiSubscription_v1 & target);
    void  get(ByteStream::ReadIterator & source, apiSession_v1 & target);
    void  get(ByteStream::ReadIterator & source, apiAccount & target);
    void  get(ByteStream::ReadIterator & source, apiSubscription & target);
    void  get(ByteStream::ReadIterator & source, apiSession & target);

    /*
    void  put(ByteStream & target, const apiAccountStatus & source);
    void  put(ByteStream & target, const apiSessionType & source);
    void  put(ByteStream & target, const apiGamecode & source);
    void  put(ByteStream & target, const apiSubscriptionStatus & source);
    void  put(ByteStream & target, const apiKickReason & source);
    void  put(ByteStream & target, const apiKickResult & source);
    */
    void  put(ByteStream & target, const apiAccount_v1 & source);
    void  put(ByteStream & target, const apiSubscription_v1 & source);
    void  put(ByteStream & target, const apiSession_v1 & source);
    void  put(ByteStream & target, const apiAccount & source);
    void  put(ByteStream & target, const apiSubscription & source);
    void  put(ByteStream & target, const apiSession & source);

}


////////////////////////////////////////////////////////////////////////////////


namespace Message
{


    class Basic : public Base::AutoByteStream
    {
        public:
            Basic();
            
            Basic(const unsigned char * const buffer, const unsigned int bufferSize);
            explicit Basic(const unsigned short messageId);
            explicit Basic(Base::ByteStream::ReadIterator & source);
            
            Basic(const Basic & source);
            Basic & operator=(const Basic & rhs);

            virtual ~Basic();
            const unsigned short GetMessageID() const;
        private:
            Base::AutoVariable<unsigned short> mMessageID;
    };

    inline const unsigned short Basic::GetMessageID() const
    {
        return mMessageID.get(); //lint !e1037 choosing a const or non const conversion is NOT ambiguous
    }


}


////////////////////////////////////////////////////////////////////////////////
//  These macros are used to define VNL messages.  These marcos make the message
//  objects easier to read and easier to define.  There are three macros used
//  to define message objects and three macros used to implement the message
//  objects.

#define BeginDefineMessage(ClassName,BaseClass,MessageID)                                                   \
    class ClassName : public BaseClass                                                                      \
    {                                                                                                       \
        enum { MESSAGE_ID = MessageID };                                                                    \
        private:                                                                                            \
            void InitializeMembers();                                                                       \
        public:                                                                                             \
            ClassName(const unsigned short messageId = MESSAGE_ID);                                         \
            ClassName(Base::ByteStream::ReadIterator & source);

#define DefineMessageMember(MemberName,Type)                                                                \
        private:                                                                                            \
            Base::AutoVariable<Type> m##MemberName;                                                         \
        public:                                                                                             \
            const Type & Get##MemberName() const                                                            \
                { return (const Type &)m##MemberName.get(); }                                               \
            void Set##MemberName(const Type & value)                                                        \
                { m##MemberName.set((const Type &)value); }

/*
#define DefineMessageMemberEnum(MemberName,Type,Internal)                                                   \
        private:                                                                                            \
            Base::AutoVariable<Internal> m##MemberName;                                                     \
        public:                                                                                             \
            const Type & Get##MemberName() const                                                            \
                { return (const Type &)m##MemberName.get(); }                                               \
            void Set##MemberName(const Type & value)                                                        \
                { m##MemberName.set((const Internal &)value); }
*/

#define DefineMessageMemberArray(MemberName, MemberType)                                                    \
        private:                                                                                            \
            Base::AutoArray<MemberType> m##MemberName;                                                      \
        public:                                                                                             \
            const std::vector<MemberType> & Get##MemberName() const { return m##MemberName.get(); }         \
            void Set##MemberName(const std::vector<MemberType> & value) { m##MemberName.set(value); }                                 

#define EndDefineMessage                                                                                    \
    };

#define BeginImplementMessage(ClassName,BaseClass)                                                          \
    ClassName::ClassName(const unsigned short messageId) : BaseClass(messageId)                             \
    {                                                                                                       \
        InitializeMembers();                                                                                \
    }                                                                                                       \
    ClassName::ClassName(Base::ByteStream::ReadIterator & source) : BaseClass(MESSAGE_ID)                   \
    {                                                                                                       \
        InitializeMembers();                                                                                \
        unpack(source);                                                                                     \
    }                                                                                                       \
    void ClassName::InitializeMembers()                                                                     \
    {                                                                                   

#define ImplementMessageMember(MemberName,DefaultValue)                                                     \
        m##MemberName.set(DefaultValue);                                                                    \
        addVariable(m##MemberName);

#define EndImplementMessage                                                                                 \
    }


////////////////////////////////////////////////////////////////////////////////


namespace Message
{


    enum
    {
        MESSAGE_NULL                                = 0x0000,

        //  Client Messages
        MESSAGE_CONNECT_v1                          = 0x0ff0,
        MESSAGE_CANCEL_REQUEST                      = 0x0ff1,
        MESSAGE_CONNECT                             = 0x0ff2,
        
        //  Server Messages
        MESSAGE_CONNECT_REPLY                       = 0x7ff0,
        MESSAGE_RECONNECT                           = 0x7ffd,
        MESSAGE_CONNECTION_CLOSED                   = 0x7ffe,
        MESSAGE_CONNECTION_FAILED                   = 0x7fff,
        
        MESSAGE_UNKNOWN                             = 0xffff
    };


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  Null
    BeginDefineMessage(Null, Basic, MESSAGE_NULL)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  Connect_v1
    BeginDefineMessage(Connect_v1, Basic, MESSAGE_CONNECT_v1)
        DefineMessageMember(Version, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  Connect
    BeginDefineMessage(Connect, Basic, MESSAGE_CONNECT)
        DefineMessageMember(Version, std::string)
        DefineMessageMember(Description, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  ConnectReply
    BeginDefineMessage(ConnectReply, Basic, MESSAGE_CONNECT_REPLY)
        DefineMessageMember(Result, apiResult)
        DefineMessageMember(Version, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  Unknown
    BeginDefineMessage(Unknown, Basic, MESSAGE_UNKNOWN)
        DefineMessageMember(UnknownMessageID, unsigned short)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  Tracked
    BeginDefineMessage(Tracked, Basic, MESSAGE_NULL)
        DefineMessageMember(TrackingNumber, apiTrackingNumber)
    EndDefineMessage


    ////////////////////////////////////////
    //  TrackedReply
    BeginDefineMessage(TrackedReply, Tracked, MESSAGE_NULL)
        DefineMessageMember(Result, apiResult)
    EndDefineMessage

    
////////////////////////////////////////////////////////////////////////////////


}

#endif  //  LAUNCHPAD_API_COMMON_H

