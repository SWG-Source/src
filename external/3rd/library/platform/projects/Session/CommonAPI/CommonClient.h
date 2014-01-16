#ifndef COMMON_API__COMMON_CLIENT_H
#define COMMON_API__COMMON_CLIENT_H


#include <list>
#include <set>
#include <map>
#include "Base/Archive.h"
#include "UdpLibrary.hpp"
#include "CommonAPI.h"
#include "CommonMessages.h"

#ifdef COMMON__UDPLIBRARY_HPP
using CommonAPI::UdpManager;
using CommonAPI::UdpConnectionHandler;
using CommonAPI::UdpConnection;
using CommonAPI::uchar;
using CommonAPI::cUdpChannelReliable1;
#endif

////////////////////////////////////////////////////////////////////////////////


class apiTrackedRequest;


////////////////////////////////////////////////////////////////////////////////

class CConnectionManager;
class CConnectionHandler : public UdpConnectionHandler
{
    public:
        CConnectionHandler(CConnectionManager & parent, UdpConnection * connection, unsigned index);
        virtual ~CConnectionHandler();
        
        void Send(const Base::ByteStream & stream);

        virtual void OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen);
        virtual void OnConnectComplete(UdpConnection *con);
        virtual void OnTerminated(UdpConnection *con);

    public:
        CConnectionManager &    mParent;
        UdpConnection *         mConnection;
        unsigned                mIndex;
};


////////////////////////////////////////////////////////////////////////////////


class CConnectionManager
{
    public:
        CConnectionManager(apiCore & parent, std::vector<std::string> & serverList, unsigned maxConnections);
        virtual ~CConnectionManager();

        void Process();
        void Connect();
        void Send(const Base::ByteStream & stream);
        bool CanSend();
        void SetConnectionDelay();

        apiCore & GetCore() { return mParent; }

        void OnConnectionOpened(unsigned index);
        void OnConnectionClosed(unsigned index);

        bool ParseAddress(const std::string & full, std::string & address, int & port);

    protected:
        apiCore &                           mParent;
        std::vector<std::string>            mServerArray;
        unsigned                            mServerCount;
        unsigned                            mMaxConnections;

        UdpManager::Params                  mParams;
        UdpManager *                        mUdpManager;
        
        std::vector<CConnectionHandler *>   mConnectionArray;
        std::set<unsigned>                  mConnectedSet;
        std::set<unsigned>                  mPendingSet;
        std::list<unsigned>                 mDisconnectedQueue;
        std::list<unsigned>                 mSendQueue;
        unsigned                            mConnectionCount;
        unsigned                            mActiveCount;

        unsigned                            mConnectionDelay;
};


////////////////////////////////////////////////////////////////////////////////


class apiTrackedRequest
{
    public:
        apiTrackedRequest(apiTrackingNumber trackingNumber, const unsigned short messageId, const void * userData, unsigned duration = 20);
        apiTrackedRequest(const apiTrackedRequest & copy);
        ~apiTrackedRequest();

        apiTrackingNumber       GetTrackingNumber();
        unsigned short          GetMessageId();
        const void *            GetUserData();

        bool                    Expired();

    private:
        apiTrackingNumber       mTrackingNumber;
        unsigned short          mMessageId;
        const void *            mUserData;
        unsigned                mTimeout;
        unsigned                mDuration;
};


////////////////////////////////////////////////////////////////////////////////


class apiCore
{
    friend class CConnectionHandler;
    friend class CConnectionManager;

    public:
        apiCore(apiClient * parent, const char * version, const char * serverList, const char * description, unsigned maxConnections);
        apiCore(apiClient * parent, const char * version, const char ** serverList, unsigned serverCount, const char * description, unsigned maxConnections);
        virtual ~apiCore();

        apiClient *         GetParent() { return mParent; }

        void Process();

        void RegisterTrackedMessage(const unsigned short messageId);
        bool IsTrackedMessage(const unsigned short messageId);

        virtual int         GetKeepAliveDelay();
        virtual int         GetPortAliveDelay();
        virtual int         GetNoDataTimeout();
        virtual int         GetUnacknowledgedDataTimeout();
        virtual void        NotifyConnectionOpened(const char *address, unsigned connectionCount);
        virtual void        NotifyConnectionClosed(const char *address, unsigned connectionCount);
        virtual void        NotifyConnectionFailed(const char *address, unsigned connectionCount);

    protected:
        void                SubmitRequest(Message::Basic & input);
        apiTrackingNumber   SubmitRequest(Message::Tracked & input, Message::TrackedReply & output, const void * userData, unsigned timeout=0);
        
        virtual bool        Callback(Base::ByteStream & stream, void * userData) = 0;
        virtual void        Timeout(unsigned short messageId, apiTrackingNumber trackingNumber, void * userData) = 0;

    private:
        void                OnConnectionOpened(const std::string & address, unsigned connectionCount);
        void                OnConnectionClosed(const std::string & address, unsigned connectionCount);
        void                OnConnectionFailed(const std::string & address, unsigned connectionCount);

        void                QueueRequest(Message::Basic & input);
        void                QueueTrackedRequest(Message::Tracked & input, apiTrackedRequest & request);

        void                QueueCallback(const Base::ByteStream & stream);
        void                QueueCallback(const Message::Basic & input);
        void                QueueTrackedCallback(const Base::ByteStream & stream);
        void                QueueTrackedCallback(const Message::Basic & input);

        std::string &       GetVersion();
        std::string &       GetDescription();

        void                Update();

    private:
        apiClient *                     mParent;
        std::string                     mVersion;
        std::string                     mDescription;
        std::vector<std::string>        mServerArray;
        unsigned                        mMaxConnections;

        CConnectionManager *            mConnectionManager;

        apiTrackingNumber               mTrackingIndex;
    
        std::set<unsigned short>        mTrackedMessages;
        std::map<unsigned,apiTrackedRequest> mRequestMap;

        std::list<Base::ByteStream>     mCallbackQueue;
        std::list<Base::ByteStream>     mTrackedCallbackQueue;
        std::list<apiTrackingNumber>    mTimeoutQueue;

        unsigned                        mRequestQueueSize;
        unsigned                        mExpirationTimeout;
        bool                            mInCallback;
};


#endif

