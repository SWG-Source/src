#ifdef WIN32
#pragma warning (disable: 4786)
#endif


#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <algorithm>
#include "Base/timer.h"
#include "Base/serialize.h"
#include "apiPinned.h"

using namespace std;

#ifdef NAMESPACE
namespace NAMESPACE
{
#endif

	const unsigned CONNECT_TIMEOUT = 5;

	////////////////////////////////////////////////////////////////////////////////

	TrackedRequest::TrackedRequest() :
		mMsgId(0),
		mTrackingNumber(0),
		mResult(0),
		mUserData(0),
		mTimeout(0)
	{
	}

	TrackedRequest::TrackedRequest(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned duration) :
		mMsgId(messageId),
		mTrackingNumber(trackingNumber),
		mResult(result),
		mUserData((void *)userData),
		mTimeout(0)
	{
		mTimeout = time(0) + duration;
	}

	////////////////////////////////////////////////////////////////////////////////

	CommonAPI::CommonAPI(const char * hostList, const char * failoverHostList, unsigned connectionLimit, unsigned maxMsgSize, unsigned bufferSize) 
		: mManager()
		, mIdleHosts()
		, mHostMap()
		, mActiveHosts()
		, mCallbackQueue()
		, mRequestMap()
		, mTimeoutMap()
		, mTimeoutTimer(0)
		, mStatusTimer(0)
		, mConnectionLimit(connectionLimit)
		, mInCallback(false)
		, mTrackingIndex(1)
	{
#ifdef UDP_LIBRARY
		UdpManager::Params params = GetConnectionParams();
		mManager = new UdpManager(&params);
#else
		TcpManager::TcpParams params = GetConnectionParams();
		mManager = new TcpManager(params);
#endif
		srand(time(0));
		if (hostList && hostList[0])
		{
			std::vector<std::string> orderArray;
			std::string hosts = hostList;
			unsigned offset = 0;
			do
			{
				unsigned index = hosts.find(' ', offset);
				orderArray.push_back(hosts.substr(offset, (index != std::string::npos) ? index-offset : index));
				offset = (index != std::string::npos) ? index+1 : index;
			}
			while (offset != std::string::npos);

			while (!orderArray.empty())
			{
				mIdleHosts[0].push_back(orderArray.back());
				orderArray.pop_back();
			}
		}
		if (failoverHostList && failoverHostList[0])
		{
			std::vector<std::string> orderArray;
			std::string hosts = failoverHostList;
			unsigned offset = 0;
			do
			{
				unsigned index = hosts.find(' ', offset);
				orderArray.push_back(hosts.substr(offset, (index != std::string::npos) ? index-offset : index));
				offset = (index != std::string::npos) ? index+1 : index;
			}
			while (offset != std::string::npos);

			while (!orderArray.empty())
			{
				mIdleHosts[1].push_back(orderArray.back());
				orderArray.pop_back();
			}
		}
	}

	CommonAPI::~CommonAPI()
	{ 
		//	ensure that callback mecahnism isn't triggered at this point
		HostMap_t::iterator iter;
		for (iter = mHostMap[0].begin(); iter != mHostMap[0].end(); iter++)
		{
			iter->first->SetHandler(0);
			iter->first->Release();
		}
		for (iter = mHostMap[1].begin(); iter != mHostMap[1].end(); iter++)
		{
			iter->first->SetHandler(0);
			iter->first->Release();
		} 
		mManager->Release();
	}


#ifdef UDP_LIBRARY
	UdpManager::Params CommonAPI::GetConnectionParams()

	{
		UdpManager::Params params;
		//	LAN connection
		params.maxRawPacketSize						= 1460;
		params.outgoingBufferSize					= 4 * 1024 * 1024;
		params.incomingBufferSize					= 4 * 1024 * 1024;
		params.keepAliveDelay						= 15 * 1000;
		params.reliableOverflowBytes				= 10 * 1024 * 1024;
		params.pooledPacketMax						= 5000;
		params.crcBytes								= 2;
		params.icmpErrorRetryPeriod					= 2000;
		params.allowPortRemapping					= false;				// set to false (optional)
		params.oldestUnacknowledgedTimeout			= 15 * 1000;			// set to 30000 (longer for debugging tolerance)
		params.maxDataHoldTime						= 0;					// set to 0 (this is critical)
		params.reliable[0].maxOutstandingBytes		= 2 * 1024 * 1024;		// set to 2mb
		params.reliable[0].maxOutstandingPackets	= 4000;					// set to 4000
		params.reliable[0].resendDelayAdjust		= 1500;					// set to 1500
		params.reliable[0].congestionWindowMinimum  = 50000;				// set to 50000

		return params;
	}
#else
	TcpManager::TcpParams CommonAPI::GetConnectionParams()

	{
		TcpManager::TcpParams params;

		params.incomingBufferSize	= 512*1024;
		params.outgoingBufferSize	= 512*1024;
		params.allocatorBlockSize	= 8*1024;
		params.allocatorBlockCount	= 1024;
		params.maxRecvMessageSize	= 8*1024;
		params.keepAliveDelay		= 15 * 1000;
		//params.noDataTimeout		= 30 * 1000; // server only setting.

		return params;
	}
#endif


	void CommonAPI::RegisterTrackedMessage(unsigned messageId)
	{
		mTrackedMessages.insert(messageId);
	}

	bool CommonAPI::IsTracked(unsigned messageId)
	{
		return mTrackedMessages.find(messageId) != mTrackedMessages.end();
	}

	unsigned CommonAPI::GetTrackingNumber()
	{
		if (!mTrackingIndex)
			mTrackingIndex++;
		return mTrackingIndex++;
	}

	unsigned CommonAPI::GetConnectionCount()
	{
		if (mActiveHosts[0].size())
			return mActiveHosts[0].size();
		else
			return mActiveHosts[1].size();
	}

#ifdef UDP_LIBRARY
	void CommonAPI::OnConnectComplete(UdpConnection * connection)
#else
	void CommonAPI::OnConnectRequest(TcpConnection * connection)
#endif
	{
		HostMap_t::iterator iter;
		if ((iter = mHostMap[0].find(connection)) != mHostMap[0].end())
		{
			mActiveHosts[0].insert(connection);
		}
		else if ((iter = mHostMap[1].find(connection)) != mHostMap[1].end())
		{
			mActiveHosts[1].insert(connection);
		}
		OnConnectionOpened(iter->second.c_str());
		//	send initial status message
		unsigned char buffer[1024];
		unsigned size = FormatStatusMsg(buffer, sizeof(buffer));
		if (size)
		{
#ifdef UDP_LIBRARY
			connection->Send(cUdpChannelReliable1, buffer, size);
			connection->FlushChannels();
#else
			connection->Send((const char*)buffer, size);
#endif
		}
	}

#ifdef UDP_LIBRARY
	void CommonAPI::OnTerminated(UdpConnection * connection)
#else
	void CommonAPI::OnTerminated(TcpConnection * connection)
#endif
	{
		HostMap_t::iterator iter;
		if ((iter = mHostMap[0].find(connection)) != mHostMap[0].end())
		{
			if (mActiveHosts[0].erase(connection) != 0)
			{
				OnConnectionClosed(iter->second.c_str(), "unknown");
			}
			else
			{
				OnConnectionFailed(iter->second.c_str());
			}
			iter->first->Release();
			mIdleHosts[0].push_back(iter->second);
			mHostMap[0].erase(iter);
		}
		else if ((iter = mHostMap[1].find(connection)) != mHostMap[1].end())
		{
			if (mActiveHosts[1].erase(connection) != 0)
			{
				OnConnectionClosed(iter->second.c_str(), "unknown");
			}
			else
			{
				OnConnectionFailed(iter->second.c_str());
			}
			iter->first->Release();
			mIdleHosts[1].push_back(iter->second);
			mHostMap[1].erase(iter);
		}
	}

#ifdef UDP_LIBRARY
	void CommonAPI::OnRoutePacket(UdpConnection * connection, const unsigned char * data, int dataLen)
#else
	void CommonAPI::OnRoutePacket(TcpConnection * connection, const unsigned char * data, int dataLen)
#endif
	{
		//	read message id
		unsigned msgId = CheckMessageId(data, dataLen);
		//	if tracked message, lookup in request map
		if (IsTracked(msgId))
		{
			//	read tracking number
			unsigned trackingNumber = CheckTrackingNumber(data, dataLen);

			RequestMap_t::iterator reqIterator = mRequestMap.find(trackingNumber);
			if (reqIterator != mRequestMap.end())
			{
				TrackedRequest & request = reqIterator->second;
				Callback(data, dataLen, msgId, (void *)request.mUserData);
				mRequestMap.erase(reqIterator);
			}
		}
		else if (msgId)
		{
			Callback(data, dataLen, msgId, 0);
		}
	}

	bool CommonAPI::Send(unsigned char hashValue, const unsigned char *data, unsigned dataLen)
	{
		if (!mActiveHosts[0].empty())
		{
            // get connection that corresponds to hash index
            ApiConnection * connection = nullptr;
            unsigned hashIndex = hashValue % mActiveHosts[0].size();
            unsigned curIndex = 0;
            for (ConnectionSet_t::iterator it = mActiveHosts[0].begin(); it != mActiveHosts[0].end(); it++, curIndex++)
            {
                if (hashIndex == curIndex)
                {
                    connection = *it;
                    break;
                }
            }
            if (!connection)
                return false;
#ifdef UDP_LIBRARY
			connection->Send(cUdpChannelReliable1, data, dataLen);
#else
			connection->Send((const char*)data, dataLen);
#endif
			return true;
		}
		else if (!mActiveHosts[1].empty())
		{
            // get connection that corresponds to hash index
            ApiConnection * connection = nullptr;
            unsigned hashIndex = hashValue % mActiveHosts[1].size();
            unsigned curIndex = 0;
            for (ConnectionSet_t::iterator it = mActiveHosts[1].begin(); it != mActiveHosts[1].end(); it++, curIndex++)
            {
                if (hashIndex == curIndex)
                {
                    connection = *it;
                    break;
                }
            }
            if (!connection)
                return false;

#ifdef UDP_LIBRARY
			connection->Send(cUdpChannelReliable1, data, dataLen);
#else
			connection->Send((const char *)data, dataLen);
#endif
			return true;
		}
		return false;
	}

	void CommonAPI::PrepareCallback(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout)
	{
		TrackedRequest request(messageId, trackingNumber, result, userData, timeout);
		mRequestMap[trackingNumber] = request;
		
		if (timeout)
		{
			mTimeoutMap[timeout].push_back(std::pair<unsigned,unsigned>(trackingNumber,time(0)+timeout));
		}
		else
		{
			mCallbackQueue.push_back(trackingNumber);
		}
	} 

	void CommonAPI::Process()
	{
		if (mInCallback)
			return;
		mInCallback = true;
		unsigned currentTime = time(0);

		////////////////////////////////////////
		//  Process connection manager
		if (!mIdleHosts[0].empty() && (mHostMap[0].size() < mConnectionLimit || !mConnectionLimit))
		{
			std::string hostName = mIdleHosts[0].front();
			mIdleHosts[0].pop_front();
			unsigned index = hostName.find(':');
			std::string host = hostName.substr(0, index);
			int port = (index != std::string::npos) ? atoi(hostName.substr(index+1, index).c_str()) : 0;

			ApiConnection * connection = mManager->EstablishConnection(host.c_str(), port, CONNECT_TIMEOUT);
			if (connection)
			{
				connection->SetHandler(this);
				mHostMap[0][connection] = hostName;
			}
		}
		if (!mIdleHosts[1].empty() && (mHostMap[1].size() < mConnectionLimit || !mConnectionLimit))
		{
			std::string hostName = mIdleHosts[1].front();
			mIdleHosts[1].pop_front();
			unsigned index = hostName.find(':');
			std::string host = hostName.substr(0, index);
			int port = (index != std::string::npos) ? atoi(hostName.substr(index+1, index).c_str()) : 0;

			ApiConnection * connection = mManager->EstablishConnection(host.c_str(), port, CONNECT_TIMEOUT);
			if (connection)
			{
				connection->SetHandler(this);
				mHostMap[1][connection] = hostName;
			}
		}
		mManager->GiveTime();

		////////////////////////////////////////
		//  Check for requests that have timed out once per second
		if (mTimeoutTimer != currentTime)
		{
			TimeoutMap_t::iterator iterator;
			for (iterator = mTimeoutMap.begin(); iterator != mTimeoutMap.end(); iterator++)
			{
				TimeoutList_t & timeoutList = iterator->second;
				while (!timeoutList.empty() && timeoutList.front().second < currentTime)
				{
					RequestMap_t::iterator reqIterator = mRequestMap.find(timeoutList.front().first);
					if (reqIterator != mRequestMap.end())
					{
						TrackedRequest & request = reqIterator->second;
						Callback(request.mMsgId, request.mTrackingNumber, request.mResult, (void *)request.mUserData);
						mRequestMap.erase(reqIterator);
					}
					timeoutList.pop_front();
				}
			}
			mTimeoutTimer = currentTime;
		}

		////////////////////////////////////////
		//  Process Callback Queue
		while (!mCallbackQueue.empty())
		{   
			RequestMap_t::iterator reqIterator = mRequestMap.find(mCallbackQueue.front());
			if (reqIterator != mRequestMap.end())
			{
				TrackedRequest & request = reqIterator->second;
				Callback(request.mMsgId, request.mTrackingNumber, request.mResult, (void *)request.mUserData);
				mRequestMap.erase(reqIterator);
			}
			mCallbackQueue.pop_front();
		}
    
		mInCallback = false;
	}

	
	////////////////////////////////////////////////////////////////////////////////

#ifdef NAMESPACE
}
#endif
