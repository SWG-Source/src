//This code is not used by anything intentionally
//The Connection class was getting mistakenly linked in
//instead of the SWG Connection class that is used for
//all of our connections. Removing the whole thing
//to prevent the issue from coming up again.
#if 0

#ifndef COMMON_SERVER__LISTENER_H
#define COMMON_SERVER__LISTENER_H

#include <string>
#include <list>
#include <set>
#include "UdpLibrary.hpp"

namespace UdpLibrary
{
	////////////////////////////////////////////////////////////////////////////////    

	class RequestBase;
	class Connection;
	class Listener : public UdpManagerHandler
	{
		friend class Connection;
		struct QueueNode
		{
			QueueNode();
			QueueNode(Connection * connection, RequestBase * request);
			bool operator<(const QueueNode & rhs) { return request < rhs.request; }

			Connection * connection;
			RequestBase * request;
		};

		public:
			Listener();
			virtual ~Listener();

			void                        QueueRequest(Connection * connection, RequestBase * request);
			bool                        IsIdle() const;
			unsigned                    Process();
			void						GetStats(UdpManagerStatistics & statsStruct);
			void						ResetStats();

			void						RequestSleep(RequestBase * request);
			void						RequestWake(RequestBase * request);

			virtual bool                IsActive() const = 0;
			virtual unsigned			GetActiveRequestMax() = 0;
			virtual UdpManager::Params	GetConnectionParams() = 0;

			virtual void                OnStartup();
			virtual void                OnShutdown();
			virtual void                OnConnectionOpened(Connection * connection);
			virtual void                OnConnectionClosed(Connection * connection, const char * reason);
			virtual void                OnConnectionDestroyed(Connection * connection);
			virtual void                OnReceive(Connection * connection, const unsigned char * data, unsigned dataLen) = 0;
			virtual void                OnCrcReject(Connection *connection, const unsigned char * data, unsigned dataLen);
			virtual void				OnPacketCorrupt(Connection *con, const unsigned char *data, int dataLen, UdpCorruptionReason reason);
			virtual void                DestroyRequest(RequestBase * request) = 0;

			virtual void                OnConnectRequest(UdpConnection *con);

		protected:
			UdpManager::Params          mParams;
			UdpManager *                mUdpManager;
       
			std::set<Connection *>			mConnections;
			unsigned                    mConnectionCount;
			std::list<Connection *>		mClosedConnections;

			std::list<QueueNode>        mQueuedRequests;
			std::list<QueueNode>        mActiveRequests;
			std::set<RequestBase *>		mSleepingRequests;
			unsigned                    mActiveCount;
			unsigned                    mActiveMax;
	};


	////////////////////////////////////////////////////////////////////////////////    


	class Connection : public UdpConnectionHandler
	{
		friend class Listener;
		public:
			Connection(Listener & listener, UdpConnection * connection);
			virtual ~Connection();

			unsigned            Send(const uchar * data, unsigned dataLen);
			void                Disconnect();
			bool                IsConnected() const;

			const std::string &	GetHost() const;
			const unsigned  	GetHostIP() const;
			unsigned            GetQueuedRequests() const;
			unsigned            GetActiveRequests() const;

			virtual void		OnTerminated(UdpConnection *con);
			virtual void		OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen);
			virtual void		OnCrcReject(UdpConnection *con, const uchar *data, int dataLen);
			virtual void		OnPacketCorrupt(UdpConnection *con, const uchar *data, int dataLen, UdpCorruptionReason reason);

		protected:
			void                NotifyQueuedRequest(RequestBase * request);
			void                NotifyDiscardRequest(RequestBase * request);
			void                NotifyBeginRequest(RequestBase * request);
			void                NotifyEndRequest(RequestBase * request);

		protected:
			Listener &			mListener;
			UdpConnection *     mConnection;
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
			RequestBase(Connection * connection);
			virtual ~RequestBase();

			virtual bool	    Process() = 0;

		protected:
			Connection *			mConnection;
			unsigned            mProcessState;
	};


	////////////////////////////////////////////////////////////////////////////////    
}


#endif 

#endif
