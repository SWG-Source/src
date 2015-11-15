#ifndef BASE__API_H
#define BASE__API_H

#include <vector>
#include <list>
#include <set>
#include <map>
#include "Base/profile.h"

#ifdef UDP_LIBRARY
#include "UdpLibrary/UdpLibrary.hpp"
#else
#include "TcpLibrary/TcpConnection.h"
#include "TcpLibrary/TcpManager.h"
#endif
#include "apiMessages.h"

#ifdef NAMESPACE
namespace NAMESPACE
{
#endif

	////////////////////////////////////////////////////////////////////////////////

	struct TrackedRequest
	{
		TrackedRequest();
		TrackedRequest(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout = 5);

		unsigned	mMsgId;
		unsigned	mTrackingNumber;
		unsigned	mResult;
		void *		mUserData;
		unsigned    mTimeout;
	};

	////////////////////////////////////////////////////////////////////////////////

#ifdef UDP_LIBRARY
	class CommonAPI : UdpConnectionHandler
#else
	class CommonAPI : TcpConnectionHandler
#endif
	{
#ifdef UDP_LIBRARY
		typedef UdpConnection ApiConnection;
#else
		typedef TcpConnection ApiConnection;
#endif
		typedef std::map<ApiConnection *, std::string> HostMap_t;
		typedef std::set<ApiConnection *> ConnectionSet_t;
		typedef std::map<unsigned, TrackedRequest> RequestMap_t;
		typedef std::list<std::pair<unsigned,unsigned> > TimeoutList_t;
		typedef std::map<unsigned, TimeoutList_t> TimeoutMap_t;
		typedef std::list<unsigned> TrackingList_t;
		typedef std::set<unsigned> MessagIdSet_t;
		public:
			CommonAPI(const char * hostList, const char * failoverHostList = 0, unsigned connectionLimit = 0, unsigned maxMsgSize = 1*1024, unsigned bufferSize = 64*1024);
			virtual ~CommonAPI();

			void						Process();
			unsigned					GetConnectionCount();
			unsigned					GetTrackingNumber();
			bool						Send(unsigned char hashValue, const unsigned char *data, unsigned dataLen);
			void						PrepareCallback(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout = 0);
        
		protected:
#ifdef UDP_LIBRARY
			virtual UdpManager::Params	GetConnectionParams();
			virtual void				OnConnectComplete(UdpConnection *con);
			virtual void				OnTerminated(UdpConnection *con);
			virtual void				OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen);
#else
			virtual TcpManager::TcpParams GetConnectionParams();
			virtual void				OnConnectRequest(TcpConnection *con);
			virtual void				OnTerminated(TcpConnection *con);
			virtual void				OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
#endif

			virtual unsigned			CheckMessageId(const unsigned char * data, unsigned dataLen) = 0;
			virtual unsigned			CheckTrackingNumber(const unsigned char * data, unsigned dataLen) = 0;

			bool						IsTracked(unsigned msgId);
			void						RegisterTrackedMessage(unsigned msgId);
			//	FormatStatusMsg provides a buffer/size for the derived class to write to and returns the size of the message written
			//		This function is called after connecting and periodically once connected.
			virtual unsigned			FormatStatusMsg(unsigned char * buffer, unsigned bufferLen) = 0;

			virtual void				OnConnectionOpened(const char * address) = 0;
			virtual void				OnConnectionFailed(const char * address) = 0;
			virtual void				OnConnectionClosed(const char * address, const char * reason) = 0;
			virtual void				Callback(const unsigned char * data, unsigned dataLen, unsigned messageId, void * userData) = 0;
			virtual void				Callback(unsigned messageId, unsigned trackingNumber, unsigned result, void * userData) = 0;

		private:
#ifdef UDP_LIBRARY
			UdpManager *				mManager;
#else
			TcpManager *				mManager;
#endif
			std::list<std::string>		mIdleHosts[2];
			HostMap_t					mHostMap[2];
			ConnectionSet_t				mActiveHosts[2];
			TrackingList_t				mCallbackQueue;
			RequestMap_t				mRequestMap;
			TimeoutMap_t				mTimeoutMap;
			unsigned					mTrackingIndex;
			MessagIdSet_t				mTrackedMessages;
			unsigned					mTimeoutTimer;
			unsigned					mStatusTimer;
			unsigned					mConnectionLimit;
			unsigned					mMaxMsgSize;
			unsigned					mBufferSize;
			bool						mInCallback;
	};

#ifdef NAMESPACE
}
#endif

#endif
