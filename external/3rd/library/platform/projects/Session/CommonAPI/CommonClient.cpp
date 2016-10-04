#pragma warning (disable : 4786)


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "CommonClient.h"
#include "CommonMessages.h"
#include "Base/Platform.h"
#include <algorithm>


using namespace std;
using namespace Base;


const int CONNECTION_ATTEMPT_TIMEOUT = 4;   //  connection timeout in seconds
const int CONNECTION_DELAY = 5;             //  pause between connection attempts in seconds
const int KEEP_ALIVE_DELAY = 15;            //  connection keep-alive timeout in seconds
const int PORT_ALIVE_DELAY = 0;             //  port-alive delay in milliseconds
const int NO_DATA_TIMEOUT = 0;              //  no-data timeout in seconds
const int UNACKNOWLEDGED_DATA_TIMEOUT = 15; //  unacknowledged-data timeout in seconds   

struct RandFunction : std::unary_function<int,int> 
{
	int operator()(int n) { return rand()%n; }
};
	  
////////////////////////////////////////////////////////////////////////////////


CConnectionHandler::CConnectionHandler(CConnectionManager & parent, UdpConnection * connection, unsigned index) :
    mParent(parent),
    mConnection(connection),
    mIndex(index)
{
    if (mConnection)
        mConnection->SetHandler(this);
}

CConnectionHandler::~CConnectionHandler()
{
    if (mConnection)
    {
        mConnection->SetHandler(nullptr);
        mConnection->Disconnect();
        mConnection->Release();
    }
}

void CConnectionHandler::Send(const Base::ByteStream & stream)
{
    if (mConnection && mConnection->GetStatus() == UdpConnection::cStatusConnected)
    {
        mConnection->Send(cUdpChannelReliable1, stream.getBuffer(), stream.getSize());
    }
}

void CConnectionHandler::OnConnectComplete(UdpConnection *)
{
    if (mConnection->GetStatus() == UdpConnection::cStatusConnected)
    {
        Message::Connect message;
        message.SetVersion(mParent.GetCore().GetVersion());
        message.SetDescription(mParent.GetCore().GetDescription());
        Base::ByteStream stream;
        message.pack(stream);
        Send(stream);
    }
    else
    {
        OnTerminated(mConnection);
    }
}

void CConnectionHandler::OnTerminated(UdpConnection *)
{
    mParent.OnConnectionClosed(mIndex);

    if (mConnection)
    {
        mConnection->SetHandler(nullptr);
        mConnection->Release();
        mConnection = 0;
    }
}

void CConnectionHandler::OnRoutePacket(UdpConnection *, const uchar *data, int dataLen)
{
    unsigned short  itemCount;
    unsigned short  messageId = 0;
    unsigned        trackingNumber = 0;
    
	if (dataLen < 8)
		return;

    Base::ByteStream stream(data,dataLen);
    Base::ByteStream::ReadIterator streamIterator = stream.begin();
    
    Base::get(streamIterator, itemCount);
    Base::get(streamIterator, messageId);
    Base::get(streamIterator, trackingNumber);

    if (messageId == Message::MESSAGE_CONNECT_REPLY)
    {
        streamIterator = stream.begin();
        Message::ConnectReply message(streamIterator);
        if (message.GetResult() == RESULT_SUCCESS)
            mParent.OnConnectionOpened(mIndex);
        else
            OnTerminated(mConnection);
    }

    mParent.GetCore().QueueTrackedCallback(stream);
}


////////////////////////////////////////////////////////////////////////////////


CConnectionManager::CConnectionManager(apiCore & parent, std::vector<std::string> & serverList, unsigned maxConnections) :
    mParent(parent),
    mServerArray(),
    mServerCount(0),
    mMaxConnections(maxConnections),
    mParams(),
    mUdpManager(0),
    mConnectionArray(),
    mConnectedSet(),
    mPendingSet(),
    mDisconnectedQueue(),
    mSendQueue(),
    mConnectionCount(0),
    mActiveCount(0),
    mConnectionDelay(0)
{
    for (unsigned serverIndex = 0; serverIndex < serverList.size(); serverIndex++)
    {
        std::string address = serverList[serverIndex];
        unsigned portIndex = address.find(':');
        if (portIndex != string::npos)
        {
            std::string host = address.substr(0, portIndex);
            std::string port = address.substr(portIndex+1);

			if (!host.length() || !port.length())
				continue;

            portIndex = port.find('-');
            if (portIndex == string::npos)
            {
				if (atoi(port.c_str()) == 0)
					continue;
				mServerArray.push_back(address);
				mServerCount++;
            }
            else
            {
                int first = atoi(port.substr(0,portIndex).c_str());
                int last  = atoi(port.substr(portIndex+1).c_str());

				if (!first || !last)
					continue;

                char buffer[64];
                for (int rangeIndex=first; rangeIndex<=last; rangeIndex++)
                {
                    sprintf(buffer,"%s:%d",host.c_str(),rangeIndex);
                    mServerArray.push_back(std::string(buffer));
                    mServerCount++;
                }
            }
        }
    }
    
    if (mMaxConnections > mServerCount)
        mMaxConnections = mServerCount;
    if (mMaxConnections == 0)
        mMaxConnections = 1;

    //mParams.handler = this;
    mParams.keepAliveDelay = parent.GetKeepAliveDelay() * 1000;
    mParams.portAliveDelay = parent.GetPortAliveDelay();
    mParams.incomingBufferSize = 256 * 1024;
    mParams.outgoingBufferSize = 256 * 1024;
    mParams.maxConnections = mMaxConnections;
    mParams.hashTableSize = mMaxConnections*10;
    mParams.port = 0;

    mUdpManager = new UdpManager(&mParams);

	srand((int)Base::getTimer()+(int)time(0));
	RandFunction myRand;
    random_shuffle(mServerArray.begin(), mServerArray.end(), myRand);
    mConnectionArray.resize(mServerCount,0);
    for (unsigned i=0; i<mServerCount; i++)
    {
        mDisconnectedQueue.push_back(i);
    }
}

CConnectionManager::~CConnectionManager()
{
    for (unsigned i=0; i<mConnectionArray.size(); i++)
    {
        if (mConnectionArray[i] != 0)
        {
            delete mConnectionArray[i];
            mConnectionArray[i] = 0;
        }
    }

    mUdpManager->Release();
}

void CConnectionManager::Process()
{
    mUdpManager->GiveTime();

    ////////////////////////////////////////
    //  try a new connection if:
    //  1. active connection count (pending + connected) < max connections
    //  2. (and) the set of disconnect servers is not empty
    //  3. (and) connection delay has expired or there are no active connections
    if (mActiveCount < mMaxConnections && !mDisconnectedQueue.empty())
    {
        if (!mConnectionCount || mConnectionDelay < (unsigned)time(0))
        {
            Connect();
        }
    }

    for (unsigned i=0; i<mServerCount; i++)
    {
        CConnectionHandler * handler = mConnectionArray[i];
        if (!handler)
            continue;

        if (!handler->mConnection)
        {
            std::set<unsigned>::iterator iterator;

            // remove from connected set
            if ((iterator = mConnectedSet.find(i)) != mConnectedSet.end())
            {
                mConnectedSet.erase(iterator);
                mActiveCount--;
                mConnectionCount--;
            }

            // remove from pending set
            if ((iterator = mPendingSet.find(i)) != mPendingSet.end())
            {
                mPendingSet.erase(iterator);
                mActiveCount--;
            }

            // add to disconnected set
            mDisconnectedQueue.push_back(i);            

            delete handler;
            mConnectionArray[i] = 0;
        }
        else
        {
            UdpConnection::ChannelStatus cs;
            handler->mConnection->GetChannelStatus(cUdpChannelReliable1,&cs);
            if (handler->mConnection->GetStatus() == UdpConnection::cStatusDisconnected || cs.oldestUnacknowledgedAge > mParent.GetUnacknowledgedDataTimeout()*1000)        // 30 seconds, fairly liberal, odds are the connection is dead after 10 seconds
            {
                handler->OnTerminated(handler->mConnection);
            }
        }
    }
}

void CConnectionManager::Connect()
{
    unsigned index = mDisconnectedQueue.front();
    mDisconnectedQueue.pop_front();

    std::string address;
    int port;

    if (!ParseAddress(mServerArray[index],address,port))
    {
        mDisconnectedQueue.push_back(index);
        return;
    }

    UdpConnection * connection = mUdpManager->EstablishConnection(address.c_str(), port, CONNECTION_ATTEMPT_TIMEOUT*1000);
    if (connection)
    {
        mConnectionArray[index] = new CConnectionHandler(*this, connection, index);

        mPendingSet.insert(index);
        mActiveCount++;
    }
    else
    {
        mDisconnectedQueue.push_back(index);
        mParent.OnConnectionFailed(mServerArray[index],mConnectionCount);
    }
}

void CConnectionManager::Send(const Base::ByteStream & stream)
{
    if (CanSend())
    {
        unsigned index;
        CConnectionHandler * handler = 0;
        while (!handler && !mSendQueue.empty())
        {
            index = mSendQueue.front();
            mSendQueue.pop_front();

            handler = mConnectionArray[index];
        }

        if (handler)
        {
            mSendQueue.push_back(index);
            handler->Send(stream);
        }
    }
}

bool CConnectionManager::CanSend()
{
    return (mConnectionCount > 0);
}

void CConnectionManager::SetConnectionDelay()
{
    mConnectionDelay = (int)time(0) + CONNECTION_DELAY;
}

void CConnectionManager::OnConnectionOpened(unsigned index)
{
    mPendingSet.erase(index);
    mConnectedSet.insert(index);
    mSendQueue.push_back(index);
    mConnectionCount++;

    mParent.OnConnectionOpened(mServerArray[index],mConnectionCount);
}

void CConnectionManager::OnConnectionClosed(unsigned index)
{
    if (mPendingSet.find(index) != mPendingSet.end())
    {
        mParent.OnConnectionFailed(mServerArray[index],mConnectionCount ? mConnectionCount-1 : mConnectionCount);
    }
    else
    {
        mSendQueue.remove(index);
        mParent.OnConnectionClosed(mServerArray[index],mConnectionCount);
    }
}

bool CConnectionManager::ParseAddress(const std::string & full, std::string & address, int & port)
{
    unsigned index = full.find(':');
    if (full.length() == 0 || index == string::npos)
        return false;

    address = full.substr(0,index);
    port = atoi(full.substr(index+1).c_str());

    return true;
}


////////////////////////////////////////////////////////////////////////////////


apiClient::apiClient() :
    mClientCore(0)
{
}

apiClient::~apiClient()
{
}

void apiClient::Process()
{
    if (mClientCore)
        static_cast<apiCore *>(mClientCore)->Process();
}

void apiClient::OnConnectionOpened(const char *, unsigned)
{
}

void apiClient::OnConnectionClosed(const char *, unsigned)
{
}

void apiClient::OnConnectionFailed(const char *, unsigned)
{
}


////////////////////////////////////////////////////////////////////////////////


apiCore::apiCore(apiClient * parent, const char * version, const char * serverList, const char * description, unsigned maxConnections) :
    mParent(parent),
    mVersion(version),
    mDescription(description),
    mServerArray(),
    mMaxConnections(maxConnections),
    mConnectionManager(0),
    mTrackingIndex(1),
    mTrackedMessages(),
    mRequestMap(),
    mCallbackQueue(),
    mTrackedCallbackQueue(),
    mTimeoutQueue(),
    mRequestQueueSize(0),
    mExpirationTimeout(0),
    mInCallback(false)
{
    std::string list = serverList;
    unsigned offset=0;
    unsigned index=0;

    while ((index = list.find(' ',offset)) != std::string::npos)
    {
        mServerArray.push_back(list.substr(offset, index-offset));
        offset = index + 1;
    }

    mServerArray.push_back(list.substr(offset));

    if (mDescription.length() > 64)
        mDescription = mDescription.substr(0,64);
}

apiCore::apiCore(apiClient * parent, const char * version, const char ** serverList, unsigned serverCount, const char * description, unsigned maxConnections) :
    mParent(parent),
    mVersion(version),
    mDescription(description),
    mServerArray(),
    mMaxConnections(maxConnections),
    mConnectionManager(0),
    mTrackingIndex(1),
    mTrackedMessages(),
    mRequestMap(),
    mCallbackQueue(),
    mTrackedCallbackQueue(),
    mTimeoutQueue(),
    mRequestQueueSize(0),
    mExpirationTimeout(0),
    mInCallback(false)
{
    for (unsigned i=0; i<serverCount; i++)
    {
        mServerArray.push_back(serverList[i]);
    }

    if (mDescription.length() > 64)
        mDescription = mDescription.substr(0,64);
}

apiCore::~apiCore()
{
    delete mConnectionManager;
}

int apiCore::GetKeepAliveDelay() 
{ 
    return KEEP_ALIVE_DELAY; 
} 

int apiCore::GetPortAliveDelay() 
{ 
    return PORT_ALIVE_DELAY; 
} 

int apiCore::GetNoDataTimeout()
{
    return NO_DATA_TIMEOUT;
}

int apiCore::GetUnacknowledgedDataTimeout()
{
    return UNACKNOWLEDGED_DATA_TIMEOUT;
}

void apiCore::NotifyConnectionOpened(const char *address, unsigned connectionCount)
{
}

void apiCore::NotifyConnectionClosed(const char *address, unsigned connectionCount)
{
}

void apiCore::NotifyConnectionFailed(const char *address, unsigned connectionCount)
{
}

void apiCore::OnConnectionOpened(const std::string & address, unsigned connectionCount)
{
    NotifyConnectionOpened(address.c_str(),connectionCount);
    mParent->OnConnectionOpened(address.c_str(),connectionCount);
}

void apiCore::OnConnectionClosed(const std::string & address, unsigned connectionCount)
{
    NotifyConnectionClosed(address.c_str(),connectionCount);
    mParent->OnConnectionClosed(address.c_str(),connectionCount);
}

void apiCore::OnConnectionFailed(const std::string & address, unsigned connectionCount)
{
    NotifyConnectionFailed(address.c_str(),connectionCount);
    mParent->OnConnectionFailed(address.c_str(),connectionCount);
}

bool apiCore::IsTrackedMessage(const unsigned short messageId)
{
    return mTrackedMessages.find(messageId) != mTrackedMessages.end();
}

void apiCore::RegisterTrackedMessage(const unsigned short messageId)
{
    mTrackedMessages.insert(messageId);
}

////////////////////////////////////////
//  This function submits a request that expects no reply
void apiCore::SubmitRequest(Message::Basic & input)
{
    Base::ByteStream stream;
    input.pack(stream);

    if (mConnectionManager && mConnectionManager->CanSend())
        mConnectionManager->Send(stream);
}

////////////////////////////////////////
//  This function submits a request that expects a reply
apiTrackingNumber apiCore::SubmitRequest(Message::Tracked & input, Message::TrackedReply & output, const void * userData, unsigned timeout)
{
    ////////////////////////////////////////
    //  Set tracking number
    apiTrackingNumber trackingNumber = mTrackingIndex++;

    input.SetTrackingNumber(trackingNumber);
    output.SetTrackingNumber(trackingNumber);    

    mRequestMap.insert(pair<apiTrackingNumber,apiTrackedRequest>(trackingNumber, apiTrackedRequest(trackingNumber, output.GetMessageID(), userData, timeout)));

    ////////////////////////////////////////
    //  Verify that the request needs to be sent.
    //  If the default result is not RESULT_TIMEOUT, 
    //  trigger callback using the output
    if (!mConnectionManager || !mConnectionManager->CanSend() || output.GetResult() != RESULT_TIMEOUT)    
    {
        QueueTrackedCallback(output);
    }
    else
    {
        Base::ByteStream stream;
        input.pack(stream);

        mConnectionManager->Send(stream);
    }

    return input.GetTrackingNumber();
}

void apiCore::QueueCallback(const Base::ByteStream & stream)
{
    mCallbackQueue.push_back(stream);
}

void apiCore::QueueCallback(const Message::Basic & message)
{
    Base::ByteStream stream;
    message.pack(stream);

    mCallbackQueue.push_back(stream);
}

void apiCore::QueueTrackedCallback(const Base::ByteStream & stream)
{
    mTrackedCallbackQueue.push_back(stream);
}

void apiCore::QueueTrackedCallback(const Message::Basic & message)
{
    Base::ByteStream stream;
    message.pack(stream);

    mTrackedCallbackQueue.push_back(stream);
}

std::string & apiCore::GetVersion()
{
    return mVersion;
}

std::string & apiCore::GetDescription()
{
    return mDescription;
}

void apiCore::Process()
{
    if (mInCallback)
        return;
    mInCallback = true;

    unsigned currentTime = (unsigned)time(0);

    ////////////////////////////////////////
    //  Process connection manager
    if (mConnectionManager)
        mConnectionManager->Process();
    else
	{
        mConnectionManager = new CConnectionManager(*this, mServerArray, mMaxConnections);
	}

    ////////////////////////////////////////
    //  Check timeout in Request Map
    if (mExpirationTimeout <= currentTime)
    {
        map<unsigned,apiTrackedRequest>::iterator mapIterator = mRequestMap.begin();
        while (mapIterator != mRequestMap.end())
        {
            apiTrackedRequest & request = (*mapIterator).second;
            mapIterator++;

            if (request.Expired())
            {
                mTimeoutQueue.push_back(request.GetTrackingNumber());
            }
        }
        mExpirationTimeout = currentTime + 3;
    }

    ////////////////////////////////////////
    //  Process Timeout Queue
    while (!mTimeoutQueue.empty())
    {   
        apiTrackingNumber trackingNumber = mTimeoutQueue.front();
        
        map<unsigned,apiTrackedRequest>::iterator mapIterator;
        mapIterator = mRequestMap.find(trackingNumber);
        if (mapIterator != mRequestMap.end())
        {
            apiTrackedRequest & request = mapIterator->second;
            Timeout(request.GetMessageId(), trackingNumber, (void *)request.GetUserData());
            mRequestMap.erase(mapIterator);
        }
        mTimeoutQueue.pop_front();
    }

	/*
    ////////////////////////////////////////
    //  Process Callback Queue
    while (!mCallbackQueue.empty())
    {   
        Base::ByteStream & stream = mCallbackQueue.front();
        Callback(stream,0);
        mCallbackQueue.pop_front();
    }
	*/

    ////////////////////////////////////////
    //  Process Tracked Callback Queue
    while (!mTrackedCallbackQueue.empty())
    {
		unsigned short  itemCount;
		unsigned short  messageId = 0;

        Base::ByteStream & stream = mTrackedCallbackQueue.front();
        Base::ByteStream::ReadIterator streamIterator = stream.begin();
		Base::get(streamIterator, itemCount);
		Base::get(streamIterator, messageId);

		if (!IsTrackedMessage(messageId))
		{
			Callback(stream,0);
		}
		else
		{
			streamIterator = stream.begin();
			Message::Tracked message(streamIterator);

			map<apiTrackingNumber,apiTrackedRequest>::iterator mapIterator = mRequestMap.find(message.GetTrackingNumber());
			if (mapIterator != mRequestMap.end())
			{
				apiTrackedRequest & request = (*mapIterator).second;
				Callback(stream,(void *)request.GetUserData());
				mRequestMap.erase(mapIterator);
			}
		}
        mTrackedCallbackQueue.pop_front();
    }
    
    mInCallback = false;
}


////////////////////////////////////////////////////////////////////////////////


const int DEFAULT_TIMEOUT   = 20;


apiTrackedRequest::apiTrackedRequest(apiTrackingNumber trackingNumber, const unsigned short messageId, const void * userData, unsigned duration) :
    mTrackingNumber(trackingNumber),
    mMessageId(messageId),
    mUserData(userData),
    mTimeout(0),
    mDuration(duration)
{
    if (mDuration == 0)
        mDuration = DEFAULT_TIMEOUT;

    mTimeout = (int)time(0) + mDuration;
}

apiTrackedRequest::apiTrackedRequest(const apiTrackedRequest & copy) :
    mTrackingNumber(copy.mTrackingNumber),
    mMessageId(copy.mMessageId),
    mUserData(copy.mUserData),
    mTimeout(copy.mTimeout),
    mDuration(copy.mDuration)
{
}

apiTrackedRequest::~apiTrackedRequest()
{
}

apiTrackingNumber apiTrackedRequest::GetTrackingNumber()
{
    return mTrackingNumber;
}

unsigned short apiTrackedRequest::GetMessageId()
{
    return mMessageId;
}

const void * apiTrackedRequest::GetUserData()
{
    return mUserData;
}

bool apiTrackedRequest::Expired()
{
    return mTimeout < (unsigned)time(0);
}


////////////////////////////////////////////////////////////////////////////////


