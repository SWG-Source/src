#ifdef WIN32
#pragma warning (disable: 4786)
#endif


#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <algorithm>
#include "Base/timer.h"
#include "Base/serialize.h"
#include "api.h"

using namespace std;

#ifdef API_NAMESPACE
namespace API_NAMESPACE
{
#endif

	const unsigned CONNECT_TIMEOUT = 5000;
	const unsigned RECONNECT_TIMEOUT = 5;

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
		mTimeout = (unsigned)time(0) + duration;
	}

	////////////////////////////////////////////////////////////////////////////////

	CommonAPI::CommonAPI(const char * hostList, const char * failoverHostList, unsigned connectionLimit, unsigned maxMsgSize, unsigned bufferSize) 
		: mManager(nullptr)
		, mHostReconnectTimeout()
		, mIdleHosts()
		, mHostMap()
		, mActiveHosts()
		, mSendIterator()
		, mCallbackQueue()
		, mRequestMap()
		, mTimeoutMap()
		, mTimeoutTimer(0)
		, mStatusTimer(0)
		, mLastRequestInputTime(0)
		, mConnectionLimit(connectionLimit)
		, mInCallback(false)
		, mRetriedConnection(false)
		, mShouldTryToConnect(true)
		, mTrackingIndex(1)
	{
#ifdef UDP_LIBRARY
		UdpManager::Params params = GetConnectionParams();
		mManager = new UdpManager(&params);
#else
		TcpManager::TcpParams params = GetConnectionParams();
		mManager = new TcpManager(params);
#endif
		srand((unsigned)time(0));
		if (hostList && hostList[0])
		{
			std::vector<std::string> orderArray;
			std::string hosts = hostList;
			size_t offset = 0;
			do
			{
				size_t index = hosts.find(' ', offset);
				orderArray.push_back(hosts.substr(offset, (index != std::string::npos) ? index-offset : index));
				offset = (index != std::string::npos) ? index+1 : index;
			}
			while (offset != std::string::npos);

			random_shuffle(orderArray.begin(), orderArray.end());
			while (!orderArray.empty())
			{
				mHostReconnectTimeout[orderArray.back()] = 0;
				mIdleHosts[0].push_back(orderArray.back());
				orderArray.pop_back();
			}
		}
		if (failoverHostList && failoverHostList[0])
		{
			std::vector<std::string> orderArray;
			std::string hosts = failoverHostList;
			size_t offset = 0;
			do
			{
				size_t index = hosts.find(' ', offset);
				orderArray.push_back(hosts.substr(offset, (index != std::string::npos) ? index-offset : index));
				offset = (index != std::string::npos) ? index+1 : index;
			}
			while (offset != std::string::npos);

			random_shuffle(orderArray.begin(), orderArray.end());
			while (!orderArray.empty())
			{
				mHostReconnectTimeout[orderArray.back()] = 0;
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

	void CommonAPI::DisconnectAll()
	{
		HostMap_t::iterator iter;
		while (!mHostMap[0].empty())
		{
			mHostMap[0].begin()->first->Disconnect();
		}
		while (!mHostMap[1].empty())
		{
			mHostMap[1].begin()->first->Disconnect();
		}

		mShouldTryToConnect = false;
	}

	void CommonAPI::AllowConnections()
	{
		mShouldTryToConnect = true;
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
		params.maxRecvMessageSize	= 2048*1024;
		params.keepAliveDelay		= 0;
		//params.noDataTimeout		= 30 * 1000; server only setting

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

	unsigned CommonAPI::GetConnectionCount() const
	{
		if (!mActiveHosts[0].empty())
			return (unsigned)mActiveHosts[0].size();
		else
			return (unsigned)mActiveHosts[1].size();
	}

	unsigned CommonAPI::GetOutstandingRequestCount() const
	{
		return mRequestMap.size();
	}

	unsigned CommonAPI::GetLastRequestInputTime() const
	{
		return mLastRequestInputTime;
	}

#ifdef UDP_LIBRARY
	void CommonAPI::SendStatusMsg(UdpConnection *connection)
#else
	void CommonAPI::SendStatusMsg(TcpConnection *connection)
#endif
	{
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
		mLastRequestInputTime = time(nullptr);
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
			mActiveHosts[0][connection].mConnection = connection;
			mUsableHosts[0].insert(connection);
			mSendIterator[0] = mUsableHosts[0].begin();
		}
		else if ((iter = mHostMap[1].find(connection)) != mHostMap[1].end())
		{
			mActiveHosts[1][connection].mConnection = connection;
			mUsableHosts[1].insert(connection);
			mSendIterator[1] = mUsableHosts[1].begin();
		}
		OnConnectionOpened(iter->second.c_str());
		//	send initial status message
		SendStatusMsg(connection);
	}

#ifdef UDP_LIBRARY
	void CommonAPI::OnTerminated(UdpConnection * connection)
#else
	void CommonAPI::OnTerminated(TcpConnection * connection)
#endif
	{
		HostMap_t::iterator iter;
		ApiConnectionInfo * pInfo = FindConnectionInfo(connection);
		bool isShuttingDown = pInfo ? (pInfo->mIsShuttingDown && pInfo->mOutstandingRequests.empty()) : false;

		if ((iter = mHostMap[0].find(connection)) != mHostMap[0].end())
		{
			if (mActiveHosts[0].erase(connection) != 0)
			{
				const char * reason = "unknown";

				if (isShuttingDown)
				{
					reason = "server shutdown";
				}
				else if (!mShouldTryToConnect)
				{
					reason = "intentional disconnect";
				}
				mUsableHosts[0].erase(connection);
				mSendIterator[0] = mUsableHosts[0].begin();
				OnConnectionClosed(iter->second.c_str(), reason);

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
				const char * reason = "unknown";

				if (isShuttingDown)
				{
					reason = "server shutdown";
				}
				else if (!mShouldTryToConnect)
				{
					reason = "intentional disconnect";
				}
				mUsableHosts[1].erase(connection);
				mSendIterator[1] = mUsableHosts[1].begin();
				OnConnectionClosed(iter->second.c_str(), reason);
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
		switch(msgId )
		{
		case MESSAGE_CONNECT_REPLY:
			if (!CheckConnectReply(data, dataLen) && !mRetriedConnection) {
				// retry once if bad version
				SendStatusMsg(connection);
				mRetriedConnection = true;
			}
			break;
		case MESSAGE_SHUTDOWN_NOTIFY_REPLY:
			{
				ApiConnectionInfo * pInfo = FindConnectionInfo(connection);
				HostMap_t::iterator iter = mHostMap[0].find(connection);
				std::string hostString("ERROR:0");

				if (iter != mHostMap[0].end()) {
					hostString = iter->second;
					mUsableHosts[0].erase(connection);
					mSendIterator[0] = mUsableHosts[0].begin();
				} else if ((iter = mHostMap[1].find(connection)) != mHostMap[1].end()) {
					hostString = iter->second;
					mUsableHosts[1].erase(connection);
					mSendIterator[1] = mUsableHosts[1].begin();
				}
				// stop sending messages along this connection; it is about to shut down
				if (pInfo) {
					if ((iter = mHostMap[0].find(connection)) != mHostMap[0].end())
					pInfo->mIsShuttingDown = true;
					SendShutdownNotification(pInfo);
					OnConnectionShutdownNotified(hostString.c_str(), pInfo->mOutstandingRequests.size());
				}
			}
			break;
		default:
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
					StopTrackingRequest(trackingNumber);
					mRequestMap.erase(reqIterator);
				}
			}
			else if (msgId)
			{
				Callback(data, dataLen, msgId, 0);
			}
			break;
		}
	}

	bool CommonAPI::Send(const unsigned char *data, unsigned dataLen)
	{
		ApiConnectionInfo * connectionInfo = GetNextUsableConnection();

		if (connectionInfo)
		{
#ifdef UDP_LIBRARY
			connectionInfo->mConnection->Send(cUdpChannelReliable1, data, dataLen);
#else
			connectionInfo->mConnection->Send((const char*)data, dataLen);
#endif
			return true;
		}
		return false;
	}

	void CommonAPI::PrepareCallback(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout)
	{
		TrackedRequest request(messageId, trackingNumber, result, userData, timeout);
		mRequestMap[trackingNumber] = request;
		mLastRequestInputTime = request.mTimeout - timeout;

		if (timeout)
		{
			mTimeoutMap[timeout].push_back(std::pair<unsigned,unsigned>(trackingNumber,(unsigned)time(0)+timeout));
		}
		else
		{
			mCallbackQueue.push_back(trackingNumber);
		}
	}

	bool CommonAPI::SendAndPrepareCallback(const unsigned char *data, unsigned dataLen, unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout)
	{
		ApiConnectionInfo * connectionInfo = GetNextUsableConnection();
		TrackedRequest request(messageId, trackingNumber, result, userData, timeout);
		
		mRequestMap[trackingNumber] = request;
		mLastRequestInputTime = request.mTimeout - timeout;
		mTimeoutMap[timeout].push_back(std::pair<unsigned,unsigned>(trackingNumber,(unsigned)time(0)+timeout));

		if (connectionInfo)
		{
#ifdef UDP_LIBRARY
			connectionInfo->mConnection->Send(cUdpChannelReliable1, data, dataLen);
#else
			connectionInfo->mConnection->Send((const char*)data, dataLen);
#endif
			// keep track of outstanding tracked requests
			connectionInfo->mOutstandingRequests.insert(trackingNumber);
			mRequestConnections[trackingNumber] = connectionInfo->mConnection;

			return true;
		}
		return false;
	}

	bool CommonAPI::GetCallbackUserData(unsigned trackingNumber, const void **pUserData)
	{
		RequestMap_t::iterator reqIterator = mRequestMap.find(trackingNumber);
		bool found = false;
		
		*pUserData = nullptr;
		if (reqIterator != mRequestMap.end())
		{
			TrackedRequest & request = reqIterator->second;
			*pUserData = request.mUserData;
			found = true;
		}
		
		return found;
	}

	void CommonAPI::Process()
	{
		if (mInCallback)
			return;
		mInCallback = true;
		unsigned currentTime = (unsigned)time(0);

		////////////////////////////////////////
		//  Process connection manager
		if (mShouldTryToConnect)
		{
			if (!mIdleHosts[0].empty() && (mHostMap[0].size() < mConnectionLimit || !mConnectionLimit))
			{
				std::string hostName = mIdleHosts[0].front();
				mIdleHosts[0].pop_front();

				ApiConnection * connection = 0;
				//	check reconnect timeout
				if (mHostReconnectTimeout[hostName] <= time(0)) 
				{
					size_t index = hostName.find(':');
					std::string host = hostName.substr(0, index);
					int port = (index != std::string::npos) ? atoi(hostName.substr(index+1, index).c_str()) : 0;

					mHostReconnectTimeout[hostName] = time(0) + RECONNECT_TIMEOUT;
					connection = mManager->EstablishConnection(host.c_str(), port, CONNECT_TIMEOUT);
					if (connection)
					{
						connection->SetHandler(this);
						mHostMap[0][connection] = hostName;
					}
					else
					{
						OnConnectionFailed(hostName.c_str());
					}
				}
				if (!connection)
				{
					mIdleHosts[0].push_back(hostName);
				}
			}
			if (!mIdleHosts[1].empty() && (mHostMap[1].size() < mConnectionLimit || !mConnectionLimit))
			{
				std::string hostName = mIdleHosts[1].front();
				mIdleHosts[1].pop_front();
				ApiConnection * connection = 0;
				//	check reconnect timeout
				if (mHostReconnectTimeout[hostName] <= time(0)) 
				{
					size_t index = hostName.find(':');
					std::string host = hostName.substr(0, index);
					int port = (index != std::string::npos) ? atoi(hostName.substr(index+1, index).c_str()) : 0;

					mHostReconnectTimeout[hostName] = time(0) + RECONNECT_TIMEOUT;
					connection = mManager->EstablishConnection(host.c_str(), port, CONNECT_TIMEOUT);
					if (connection)
					{
						connection->SetHandler(this);
						mHostMap[1][connection] = hostName;
					}
					else
					{
						OnConnectionFailed(hostName.c_str());
					}
				}
				if (!connection)
				{
					mIdleHosts[1].push_back(hostName);
				}
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
						StopTrackingRequest(request.mTrackingNumber);
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
				StopTrackingRequest(request.mTrackingNumber);
				mRequestMap.erase(reqIterator);
			}
			mCallbackQueue.pop_front();
		}
    
		mInCallback = false;
	}

	CommonAPI::ApiConnectionInfo * CommonAPI::GetNextUsableConnection()
	{
		ApiConnectionInfo * connectionInfo = nullptr;

		if (!mUsableHosts[0].empty())
		{
			connectionInfo = FindConnectionInfo(*mSendIterator[0]);
			mSendIterator[0]++;
			if (mSendIterator[0] == mUsableHosts[0].end())
				mSendIterator[0] = mUsableHosts[0].begin();
		}
		else if (!mUsableHosts[1].empty())
		{
			connectionInfo = FindConnectionInfo(*mSendIterator[1]);
			mSendIterator[1]++;
			if (mSendIterator[1] == mUsableHosts[1].end())
				mSendIterator[1] = mUsableHosts[1].begin();
		}

		return connectionInfo;
	}

	CommonAPI::ApiConnectionInfo * CommonAPI::FindConnectionInfo(ApiConnection * connection)
	{
		// find the connection in the set
		ApiConnectionInfo * pInfo = nullptr;
		ConnectionMap_t::iterator iter;
		if ( ((iter = mActiveHosts[0].find(connection)) != mActiveHosts[0].end()) ||
			 ((iter = mActiveHosts[1].find(connection)) != mActiveHosts[1].end()) )
		{
			pInfo = &iter->second;
		}

		return pInfo;
	}

	void CommonAPI::StopTrackingRequest(unsigned trackingNumber)
	{
		// get the connection on which the request was sent
		ConnectionTrackMap_t::iterator trackIter = mRequestConnections.find(trackingNumber);
		if (trackIter != mRequestConnections.end())
		{
			// stop tracking the request
			ApiConnectionInfo * pInfo = FindConnectionInfo(trackIter->second);
			if (pInfo)
			{
				pInfo->mOutstandingRequests.erase(trackingNumber);

				if (pInfo->mIsShuttingDown && pInfo->mOutstandingRequests.empty())
				{
					SendShutdownNotification(pInfo);
				}
			}
			mRequestConnections.erase(trackIter);
		}
	}

	void CommonAPI::SendShutdownNotification(ApiConnectionInfo *pInfo)
	{
		Message::ShutdownNotify shutdownMessage;

		shutdownMessage.SetUnfinishedRequests(pInfo->mOutstandingRequests.size());

		unsigned char buffer[1024];
		unsigned size = shutdownMessage.Write(buffer, sizeof(buffer));

		if (size)
		{	
#ifdef UDP_LIBRARY
			pInfo->mConnection->Send(cUdpChannelReliable1, buffer, size);
			pInfo->mConnection->FlushChannels();	
#else
			pInfo->mConnection->Send((const char*)buffer, size);	
#endif
		}
	}

	////////////////////////////////////////////////////////////////////////////////

	VersionMap::InsertionObject::InsertionObject(const std::string &versionString, unsigned enumeratedValue)
	{
		addVersionPair(versionString, enumeratedValue);
	}

	void VersionMap::addVersionPair(const std::string &versionString, unsigned enumeratedValue)
	{	
		initializeMaps();

		(*mspVersionStringToEnumerationMap)[versionString] = enumeratedValue;
		(*mspEnumerationToVersionStringMap)[enumeratedValue] = versionString;
	}

	const std::string &VersionMap::getVersionString(unsigned enumeratedValue)
	{
		initializeMaps();

		std::map<unsigned, std::string>::const_iterator vIter = mspEnumerationToVersionStringMap->find(enumeratedValue);
		static std::string blankVersion = "";

		if (vIter != mspEnumerationToVersionStringMap->end()) {
			return vIter->second;
		} else {
			return blankVersion;
		}
	}

	unsigned VersionMap::getEnumeratedValue(const std::string &versionString)
	{
		initializeMaps();

		std::map<std::string, unsigned>::const_iterator vIter = mspVersionStringToEnumerationMap->find(versionString);
		unsigned enumeratedValue = 0;

		if (vIter != mspVersionStringToEnumerationMap->end()) {
			enumeratedValue = vIter->second;
		}

		return enumeratedValue;
	}

	const std::string &VersionMap::getEarliestVersionString()
	{
		initializeMaps();

		std::map<unsigned, std::string>::iterator vIter = mspEnumerationToVersionStringMap->begin();
		static std::string blankVersion = "";

		if (vIter == mspEnumerationToVersionStringMap->end()) {
			return vIter->second;
		} else {
			return blankVersion;
		}
	}

	unsigned VersionMap::getEarliestVersionEnumeration()
	{
		initializeMaps();

		std::map<std::string, unsigned>::iterator vIter = mspVersionStringToEnumerationMap->begin();
		unsigned enumeratedValue = 0;

		if (vIter != mspVersionStringToEnumerationMap->end()) {
			enumeratedValue = vIter->second;
		}

		return enumeratedValue;
	}

	const std::string &VersionMap::getLatestVersionString()
	{
		initializeMaps();

		std::map<unsigned, std::string>::reverse_iterator vIter = mspEnumerationToVersionStringMap->rbegin();
		static std::string blankVersion = "";

		if (vIter == mspEnumerationToVersionStringMap->rend()) {
			return vIter->second;
		} else {
			return blankVersion;
		}
	}

	unsigned VersionMap::getLatestVersionEnumeration()
	{
		initializeMaps();

		std::map<std::string, unsigned>::reverse_iterator vIter = mspVersionStringToEnumerationMap->rbegin();
		unsigned enumeratedValue = 0;

		if (vIter != mspVersionStringToEnumerationMap->rend()) {
			enumeratedValue = vIter->second;
		}

		return enumeratedValue;
	}

	void VersionMap::initializeMaps()
	{
		static std::map<std::string, unsigned> versionStringToEnumerationMap;
		static std::map<unsigned, std::string> enumerationToVersionStringMap;
	
		mspVersionStringToEnumerationMap = &versionStringToEnumerationMap;
		mspEnumerationToVersionStringMap = &enumerationToVersionStringMap;
	}

	std::map<std::string, unsigned> *VersionMap::mspVersionStringToEnumerationMap = nullptr;
	std::map<unsigned, std::string> *VersionMap::mspEnumerationToVersionStringMap = nullptr;

	////////////////////////////////////////////////////////////////////////////////

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, std::string &valueHolder)
	{
		ConfigurationMap::ConfigurationEntry entry(label, &valueHolder, eValueTypeString);
		addConfigurationEntry(entry);
	}

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, soe::uint32 &valueHolder)
	{
		ConfigurationMap::ConfigurationEntry entry(label, &valueHolder, eValueTypeUInt32);
		addConfigurationEntry(entry);
	}

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, soe::int32 &valueHolder)
	{
		ConfigurationMap::ConfigurationEntry entry(label, &valueHolder, eValueTypeInt32);
		addConfigurationEntry(entry);
	}

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, soe::uint16 &valueHolder)
	{
		ConfigurationMap::ConfigurationEntry entry(label, &valueHolder, eValueTypeUInt16);
		addConfigurationEntry(entry);
	}

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, soe::int16 &valueHolder)
	{

	}

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, double &valueHolder)
	{
		ConfigurationMap::ConfigurationEntry entry(label, &valueHolder, eValueTypeDouble);
		addConfigurationEntry(entry);
	}

	ConfigurationMap::InsertionObject::InsertionObject(const std::string &label, float &valueHolder)
	{
		ConfigurationMap::ConfigurationEntry entry(label, &valueHolder, eValueTypeFloat);
		addConfigurationEntry(entry);
	}

	ConfigurationMap::ConfigurationEntry::ConfigurationEntry(const std::string &labelIn, void * valueHolderIn, EValueType valueTypeIn)
		: label(labelIn)
		, valueHolder(valueHolderIn)
		, valueType(valueTypeIn)
	{ }

	void ConfigurationMap::addConfigurationEntry(const ConfigurationMap::ConfigurationEntry &entry)
	{
		initializeMap();

		(*mspLabelToEntryMap)[entry.label] = entry;
	}

	std::string api_uitoa(soe::uint64 number)
	{
		char buffer[256];

		sprintf(buffer, FMT_UINT64, number);

		return buffer;
	}

	std::string api_itoa(soe::int64 number)
	{
		char buffer[256];

		sprintf(buffer, FMT_INT64, number);

		return buffer;
	}

	std::string api_ftoa(double number)
	{
		char buffer[1024];

		sprintf(buffer, "%f", number);

		return buffer;
	}

	void ConfigurationMap::getConfigurationEntries(soe::NameValuePairs_t &labelsAndValues)
	{
		initializeMap();

		size_t i = labelsAndValues.size();

		labelsAndValues.resize(labelsAndValues.size() + mspLabelToEntryMap->size());
		for (LabelToEntryMap_t::const_iterator it = mspLabelToEntryMap->begin(); it != mspLabelToEntryMap->end(); it++, i++)
		{
			string value;

			labelsAndValues[i].name = it->second.label;

			switch(it->second.valueType)
			{
			case eValueTypeString:
				{
					const std::string * pStr = (const std::string *)it->second.valueHolder;

					value = *pStr;
				}
				break;
			case eValueTypeUInt32:
				{
					const soe::uint32 * pInt = (const soe::uint32 *)it->second.valueHolder;

					value = api_uitoa(*pInt);
				}
				break;
			case eValueTypeInt32:
				{
					const soe::int32 * pInt = (const soe::int32 *)it->second.valueHolder;

					value = api_itoa(*pInt);
				}
				break;
			case eValueTypeUInt16:
				{
					const soe::uint16 * pInt = (const soe::uint16 *)it->second.valueHolder;

					value = api_uitoa(*pInt);
				}
				break;
			case eValueTypeInt16:
				{
					const soe::int16 * pInt = (const soe::int16 *)it->second.valueHolder;

					value = api_itoa(*pInt);
				}
				break;
			case eValueTypeDouble:
				{
					const double * pFloat = (const double *)it->second.valueHolder;

					value = api_ftoa(*pFloat);
				}
				break;
			case eValueTypeFloat:
				{
					const float * pFloat = (const float *)it->second.valueHolder;

					value = api_ftoa(*pFloat);
				}
				break;
			default:
				break;
			}
			labelsAndValues[i].value = value;
		}
	}

	void ConfigurationMap::setConfigurationEntries(const soe::NameValuePairs_t &labelsAndValues)
	{
		initializeMap();

		for	(soe::NameValuePairs_t::const_iterator it = labelsAndValues.begin(); it != labelsAndValues.end(); it++)
		{
			LabelToEntryMap_t::iterator mIter = mspLabelToEntryMap->find(it->name);

			if (mIter != mspLabelToEntryMap->end()) {

				switch(mIter->second.valueType)
				{
				case eValueTypeString:
					{
						std::string * pStr = (std::string *)mIter->second.valueHolder;

						*pStr = it->value;
					}
					break;
				case eValueTypeUInt32:
					{
						soe::uint32 * pInt = (soe::uint32 *)mIter->second.valueHolder;

						*pInt = (soe::uint32)atoi64(it->value.c_str());
					}
					break;
				case eValueTypeInt32:
					{
						soe::int32 * pInt = (soe::int32 *)mIter->second.valueHolder;

						*pInt = (soe::int32)atoi64(it->value.c_str());
					}
					break;
				case eValueTypeUInt16:
					{
						soe::uint16 * pInt = (soe::uint16 *)mIter->second.valueHolder;

						*pInt = (soe::uint16)atoi64(it->value.c_str());
					}
					break;
				case eValueTypeInt16:
					{
						soe::int16 * pInt = (soe::int16 *)mIter->second.valueHolder;

						*pInt = (soe::int16)atoi64(it->value.c_str());
					}
					break;
				case eValueTypeDouble:
					{
						double * pFloat = (double *)mIter->second.valueHolder;

						*pFloat = atof(it->value.c_str());
					}
					break;
				case eValueTypeFloat:
					{
						float * pFloat = (float *)mIter->second.valueHolder;

						*pFloat = atof(it->value.c_str());
					}
					break;
				default:
					break;
				}
			}
		}
	}

	void ConfigurationMap::initializeMap()
	{
		static LabelToEntryMap_t labelToEntryMap;

		mspLabelToEntryMap = &labelToEntryMap;
	}

	ConfigurationMap::LabelToEntryMap_t *ConfigurationMap::mspLabelToEntryMap = nullptr;

	////////////////////////////////////////////////////////////////////////////////

	typedef std::set<soe::ClassScribeBase *> ClassScribeSet_t;
	ClassScribeSet_t classScribeSet;

	void AddClassScribeToGlobalSet(soe::ClassScribeBase * pScribe)
	{
		classScribeSet.insert(pScribe);
	}
	
	/************************************************************************/
	/* Call this function after changing any configurable variables			*/
	/*	that affect class serialization										*/
	/************************************************************************/
	void ForceReinitializationOfAllClassScribes()
	{
		ClassScribeSet_t::iterator scribeIter;

		for (scribeIter = classScribeSet.begin(); scribeIter != classScribeSet.end(); scribeIter++)
		{
			// This will cause the scribe to reinitialize itself next time it's used,
			// thereby copying the new values. Storing references was too dangerous.
			(*scribeIter)->SetInitialized(false);
		}
	}

	////////////////////////////////////////////////////////////////////////////////

#ifdef API_NAMESPACE
}
#endif
