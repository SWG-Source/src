
//This code is not used by anything intentionally
//The Connection class was getting mistakenly linked in
//instead of the SWG Connection class that is used for
//all of our connections. Removing the whole thing
//to prevent the issue from coming up again.
#if 0


#ifndef TCP_LISTENER_H
#define TCP_LISTENER_H

#include <string>
#include <list>
#include <set>

#include "TcpManager.h"


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif

////////////////////////////////////////////////////////////////////////////////    

	class RequestBase;
	class Connection;
	class Listener : public TcpManagerHandler
	{
		friend class Connection;
		struct QueueNode
		{
			QueueNode();
			QueueNode(Connection * connection, RequestBase * request);
			bool operator<(const QueueNode & rhs) { return request < rhs.request; }

			Connection * connection;
			RequestBase *    request;
		};

		public:
			Listener();
			virtual ~Listener();

			void                        QueueRequest(Connection * connection, RequestBase * request);
			bool                        IsIdle() const;
			bool						IsAcceptingNewConnections() const;
			unsigned                    Process();
			void						SetAcceptingNewConnections(bool value) { mAcceptingNewConnections = value; }
			//void						GetStats(TcpManagerStatistics & statsStruct);
			//void						ResetStats();

			unsigned					GetNumberQueuedRequests() { return (unsigned)mQueuedRequests.size(); }

			void						RequestSleep(RequestBase * request);
			void						RequestWake(RequestBase * request);

			virtual bool                IsActive() const = 0;
			virtual unsigned			GetActiveRequestMax() = 0;
			virtual TcpManager::TcpParams GetConnectionParams() = 0;

			virtual void                OnStartup();
			virtual void                OnShutdown();
			virtual void                OnFailedStartup();
			virtual void                OnConnectionOpened(Connection * connection);
			virtual void                OnConnectionClosed(Connection * connection, const char * reason);
			virtual void                OnConnectionDestroyed(Connection * connection);
			virtual void                OnReceive(Connection * connection, const unsigned char * data, unsigned dataLen) = 0;
			virtual void                OnCrcReject(Connection *connection, const unsigned char * data, unsigned dataLen);
			//virtual void				OnPacketCorrupt(Connection *con, const unsigned char *data, int dataLen, TcpCorruptionReason reason);
			virtual void                DestroyRequest(RequestBase * request) = 0;

			virtual void                OnConnectRequest(TcpConnection *con);

		protected:
			TcpManager::TcpParams       mParams;
			TcpManager *                mTcpManager;
       
			std::set<Connection *>      mConnections;
			unsigned                    mConnectionCount;
			std::list<Connection *>     mClosedConnections;

			std::list<QueueNode>        mQueuedRequests;
			std::list<QueueNode>        mActiveRequests;
			std::set<RequestBase *>		mSleepingRequests;
			unsigned                    mActiveCount;
			unsigned                    mActiveMax;
			bool						mAcceptingNewConnections;
	};

////////////////////////////////////////////////////////////////////////////////    

	class Connection : public TcpConnectionHandler
	{
		friend class Listener;
		public:
			Connection(Listener & listener, TcpConnection * connection);
			virtual ~Connection();

			unsigned            Send(const unsigned char * data, unsigned dataLen);
			void                Disconnect();
			bool                IsConnected() const;

			const std::string &	GetHost() const;
			const unsigned  	GetHostIP() const;
			unsigned            GetQueuedRequests() const;
			unsigned            GetActiveRequests() const;

			virtual void		OnTerminated(TcpConnection *con);
			virtual void		OnConnectRequest(TcpConnection *con) {};
			virtual void		OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
			virtual void		OnCrcReject(TcpConnection *con, const unsigned char *data, int dataLen);
			//virtual void		OnPacketCorrupt(TcpConnection *con, const unsigned char *data, int dataLen, TcpCorruptionReason reason);

		protected:
			void                NotifyQueuedRequest(RequestBase * request);
			void                NotifyDiscardRequest(RequestBase * request);
			void                NotifyBeginRequest(RequestBase * request);
			void                NotifyEndRequest(RequestBase * request);

		protected:
			Listener &          mListener;
			TcpConnection *     mConnection;
			std::string         mHost;
			unsigned			mHostIp;
			const char *		mDisconnectReason;
			unsigned            mQueuedRequests;
			unsigned            mActiveRequests;
	};

////////////////////////////////////////////////////////////////////////////////    

	class RequestBase
	{
		friend class Listener;
		public:
			RequestBase(Connection * connection, bool isInternal=false);
			virtual ~RequestBase();

			virtual bool	    Process() = 0;

			Connection *		GetConnection() const { return mConnection; }
			unsigned			GetState() const { return mProcessState; }

		protected:
			Connection *        mConnection;
			unsigned            mProcessState;
            bool                mIsInternal;
	};

////////////////////////////////////////////////////////////////////////////////    

#ifdef EXTERNAL_DISTRO
}
#endif

#endif

#endif
