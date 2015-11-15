#include "CommonMessages.h"


namespace Base 
{


    void get(ByteStream::ReadIterator & source, apiAccount & target)
    {
        char name[apiAccountNameWidth];
        apiAccountId id;
        apiAccountStatus status;

        get(source, (unsigned char *)name, apiAccountNameWidth);
        get(source, id);
        get(source, status);

        target.SetName(name);
        target.SetId(id);
        target.SetStatus(status);
    }

    void get(ByteStream::ReadIterator & source, apiSubscription & target)
    {
        apiGamecode gamecode;
        apiProviderId provider;
        apiSubscriptionStatus status;
        apiProductFeatures subscriptionFeatures;
        apiProductFeatures gameFeatures;
        unsigned durationSeconds;
        unsigned parentalLimitSeconds;

        get(source, gamecode);
        get(source, provider);
        get(source, status);
        get(source, subscriptionFeatures);
        get(source, gameFeatures);
        get(source, durationSeconds);
        get(source, parentalLimitSeconds);

        target.SetGamecode(gamecode);
        target.SetProviderId(provider);
        target.SetStatus(status);
        target.SetSubscriptionFeatures(subscriptionFeatures);
        target.SetGameFeatures(gameFeatures);
        target.SetDurationSeconds(durationSeconds);
        target.SetParentalLimitSeconds(parentalLimitSeconds);

    }
    
    void get(ByteStream::ReadIterator & source, apiSession & target)
    {
        apiSessionType type;
        apiProviderId provider;
        char id[apiSessionIdWidth];
        apiIP clientIP;
        unsigned durationSeconds;
        unsigned parentalLimitSeconds;
        unsigned char timeoutMinutes;
        bool isSecure;

        get(source, type);
        get(source, provider);
        get(source, (unsigned char *)id, apiSessionIdWidth);
        get(source, clientIP);
        get(source, durationSeconds);
        get(source, parentalLimitSeconds);
        get(source, timeoutMinutes);
        get(source, isSecure);

        target.SetType(type);
        target.SetProviderId(provider);
        target.SetId(id);
        target.SetClientIP(clientIP);
        target.SetDurationSeconds(durationSeconds);
        target.SetParentalLimitSeconds(parentalLimitSeconds);
        target.SetTimeoutMinutes(timeoutMinutes);
        target.SetIsSecure(isSecure);
    }

    void get(ByteStream::ReadIterator & source, apiAccount_v1 & target)
    {
        get(source, (unsigned char *)target.name, apiAccountNameWidth_v1);
        get(source, target.id);
        get(source, target.status);
    }

    void get(ByteStream::ReadIterator & source, apiSubscription_v1 & target)
    {
        get(source, target.gamecode);
        get(source, target.status);
        get(source, target.subscriptionFeatures);
        get(source, target.gameFeatures);
        get(source, target.durationSeconds);
        get(source, target.parentalLimitSeconds);
    }
    
    void get(ByteStream::ReadIterator & source, apiSession_v1 & target)
    {
        get(source, target.type);
        get(source, (unsigned char *)target.id, apiSessionIdWidth_v1);
        get(source, target.clientIP);
        get(source, target.durationSeconds);
        get(source, target.parentalLimitSeconds);
        get(source, target.timeoutMinutes);
        get(source, target.isSecure);
    }

	void  get(ByteStream::ReadIterator & source, SessionTypeDesc & target)
	{
		unsigned tmp = 0;
		get(source, target.type);
		get(source, (unsigned char *)target.gameCode, gameCodeWidth);
		get(source, target.gameCodeID);
		get(source, (unsigned char *)target.description, sessionTypeDescriptionWidth);
		get(source, tmp);
		target.independent = (tmp == 1);
		get(source, tmp);
		target.exclusive = (tmp == 1);
	}

	void  get(ByteStream::ReadIterator & source, GameCodeDescription & target)
	{
		unsigned tmp = 0;
		get(source, target.gameID);
		get(source, (unsigned char *)target.code, gameCodeWidth);
		get(source, (unsigned char *)target.description, sessionTypeDescriptionWidth);
		get(source, tmp);
		target.createDynamic = (tmp == 1);
	}


    
    void put(ByteStream & target, const apiAccount & source)
    {
        put(target, (const unsigned char *)source.GetName(), apiAccountNameWidth);
        put(target, source.GetId());
        put(target, source.GetStatus());
    }

    void put(ByteStream & target, const apiSubscription & source)
    {
        put(target, source.GetGamecode());
        put(target, source.GetProviderId());
        put(target, source.GetStatus());
        put(target, source.GetSubscriptionFeatures());
        put(target, source.GetGameFeatures());
        put(target, source.GetDurationSeconds());
        put(target, source.GetParentalLimitSeconds());
    }
    
    void put(ByteStream & target, const apiSession & source)
    {
        put(target, source.GetType());
        put(target, source.GetProviderId());
        put(target, (const unsigned char *)source.GetId(), apiSessionIdWidth);
        put(target, source.GetClientIP());
        put(target, source.GetDurationSeconds());
        put(target, source.GetParentalLimitSeconds());
        put(target, source.GetTimeoutMinutes());
        put(target, source.GetIsSecure());
    }

    void put(ByteStream & target, const apiAccount_v1 & source)
    {
        put(target, (unsigned char *)source.name, apiAccountNameWidth_v1);
        put(target, source.id);
        put(target, source.status);
    }

    void put(ByteStream & target, const apiSubscription_v1 & source)
    {
        put(target, source.gamecode);
        put(target, source.status);
        put(target, source.subscriptionFeatures);
        put(target, source.gameFeatures);
        put(target, source.durationSeconds);
        put(target, source.parentalLimitSeconds);
    }

    void put(ByteStream & target, const apiSession_v1 & source)
    {
        put(target, source.type);
        put(target, (unsigned char *)source.id, apiSessionIdWidth_v1);
        put(target, source.clientIP);
        put(target, source.durationSeconds);
        put(target, source.parentalLimitSeconds);
        put(target, source.timeoutMinutes);
        put(target, source.isSecure);
    }

	void  put (ByteStream & target, const SessionTypeDesc & source)
	{
		put(target, source.type);
		put(target, (unsigned char *)source.gameCode, gameCodeWidth);
		put(target, source.gameCodeID);
		put(target, (unsigned char *)source.description, sessionTypeDescriptionWidth);
		put(target, (unsigned)source.independent);
		put(target, (unsigned)source.exclusive);
	}

	void  put (ByteStream & target, const GameCodeDescription & source)
	{
		put(target, source.gameID);
		put(target, (unsigned char *)source.code, gameCodeWidth);
		put(target, (unsigned char *)source.description, sessionTypeDescriptionWidth);
		put(target, (unsigned)source.createDynamic);
	}

}


////////////////////////////////////////////////////////////////////////////////
    

namespace Message
{


////////////////////////////////////////////////////////////////////////////////

    
    /*
    Basic::Basic() :
        AutoByteStream(),
        mMessageID(0)
    {
    }
    */

    Basic::Basic(const unsigned short newMessageId) :
        AutoByteStream(),
        mMessageID(newMessageId)
    {
        addVariable(mMessageID);
    }

    Basic::Basic(Base::ByteStream::ReadIterator & source) : 
        AutoByteStream(),
        mMessageID(0)
    {
        addVariable(mMessageID);
        AutoByteStream::unpack(source);
    }

    /*
    Basic::Basic(const Basic & source) :
        AutoByteStream(source),
        mMessageID(source.GetMessageID())
    {
    }

    Basic & Basic::operator=(const Basic & rhs)
    {
        if (&rhs != this)
        {

        }
    }
    */

    Basic::~Basic()
    {
    }


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  Null
    BeginImplementMessage(Null, Basic)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  Connect_v1
    BeginImplementMessage(Connect_v1, Basic)
        ImplementMessageMember(Version, "unknown")
    EndImplementMessage


    ////////////////////////////////////////
    //  Connect
    BeginImplementMessage(Connect, Basic)
        ImplementMessageMember(Version, "unknown")
        ImplementMessageMember(Description, "unknown")
    EndImplementMessage


    ////////////////////////////////////////
    //  ConnectReply
    BeginImplementMessage(ConnectReply, Basic)
        ImplementMessageMember(Result, RESULT_SUCCESS)
        ImplementMessageMember(Version, "unknown")
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  Unknown
    BeginImplementMessage(Unknown, Basic)
        ImplementMessageMember(UnknownMessageID,0)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  Tracked
    BeginImplementMessage(Tracked, Basic)
        ImplementMessageMember(TrackingNumber, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  TrackedReply
    BeginImplementMessage(TrackedReply, Tracked)
        ImplementMessageMember(Result, RESULT_TIMEOUT)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

}

