
//This code is not used by anything intentionally
//The Connection class was getting mistakenly linked in
//instead of the SWG Connection class that is used for
//all of our connections. Removing the whole thing
//to prevent the issue from coming up again.
#if 0

#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include <stdio.h>
#include "TcpConnection.h"
#include "TcpListener.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

////////////////////////////////////////////////////////////////////////////////    

Listener::QueueNode::QueueNode() :
    connection(0),
    request(0)
{
}

Listener::QueueNode::QueueNode(Connection * con, RequestBase * req) :
    connection(con),
    request(req)
{
}

Listener::Listener() :
	mParams(),
    mTcpManager(0),
    mConnections(),
    mConnectionCount(0),
	mClosedConnections(),
    mQueuedRequests(),
    mActiveRequests(),
    mActiveCount(0),
    mActiveMax(0),
	mAcceptingNewConnections(true)
{
    //printf("ctor 0x%x Listener\n",this);
}

Listener::~Listener()
{
    std::set<Connection *>::iterator iterator;
    for (iterator = mConnections.begin(); iterator != mConnections.end(); iterator++)
    {
        Connection * connection = *iterator;
        delete connection;
    }
    mConnections.clear();
    
    if (mTcpManager != 0)
    {
        mTcpManager->Release();
        mTcpManager = 0;
    }

    //printf("dtor 0x%x Listener\n",this);
}

void Listener::RequestSleep(RequestBase * request)
{
	mSleepingRequests.insert(request);
}

void Listener::RequestWake(RequestBase * request)
{
	if (mSleepingRequests.erase(request))
	{
        QueueNode node(request->mConnection, request);
		mActiveRequests.push_front(node);
	}
}

void Listener::OnConnectRequest(TcpConnection * connection)
{
	if (IsAcceptingNewConnections())
	{
		Connection * connectionObject = new Connection(*this, connection);

		mConnections.insert(connectionObject);
		mConnectionCount++;

		OnConnectionOpened(connectionObject);
	}
}

void Listener::QueueRequest(Connection * connection, RequestBase * request)
{
    if (connection)
    {
        // normal request, internal requests have no connection
	    connection->NotifyQueuedRequest(request);
    }
    mQueuedRequests.push_back(QueueNode(connection,request));
}

bool Listener::IsIdle() const
{
    return (!IsActive() && !mTcpManager && mQueuedRequests.empty() && !mActiveCount);
}

bool Listener::IsAcceptingNewConnections() const
{
	return (IsActive() && mAcceptingNewConnections);
}

unsigned Listener::Process()
{
	//Profile profile("Listener::Process");
    ////////////////////////////////////////
    //  handle inactive state (with UdpManager)
    if (!IsActive() && mTcpManager)
    {
		//	check all connections to see if they are idle
		std::set<Connection *>::iterator iterator;
		for (iterator = mConnections.begin(); iterator != mConnections.end(); iterator++)
		{
			Connection * connection = *iterator;
			if (connection->IsConnected())
				connection->Disconnect();
		}
		//	close the UdpManager if all the connections are closed
		if (!mConnectionCount)
		{
			mTcpManager->Release();
			mTcpManager = 0;
			OnShutdown();
		}
    }
    ////////////////////////////////////////
    //  handle active state (without UdpManager)
    else if (IsActive() && !mTcpManager)
    {
		mParams = GetConnectionParams();
        mActiveMax = GetActiveRequestMax();
        mTcpManager = new TcpManager(mParams);
		mTcpManager->SetHandler(this);
		if (mTcpManager->BindAsServer()){
            OnStartup();
        }else{
            OnFailedStartup();
            return 0;
        }
    }

    ////////////////////////////////////////
    //  process the TcpManager
    if (mTcpManager)
    {
		//Profile subProfile("TcpManager::GiveTime()");
        mTcpManager->GiveTime();
    }

	//	check all closed connections to see if they are idle
	std::list<Connection *>::iterator closedIterator = mClosedConnections.begin();
	while (closedIterator != mClosedConnections.end())
	{
		//Profile profile("Listener::Process (cleanup connection)");
		std::list<Connection *>::iterator current = closedIterator++;
		Connection * connection = *current;
		if (!connection->GetActiveRequests() && 
			!connection->GetQueuedRequests())
		{
			mClosedConnections.erase(current);
			mConnections.erase(connection);
			mConnectionCount--;
			OnConnectionDestroyed(connection);
			delete connection;
		}
	}

    ////////////////////////////////////////
    //  process request queue
    while (!mQueuedRequests.empty() && (!mActiveMax || mActiveCount < mActiveMax))
    {
		//Profile profile("Listener::Process (activate queued request)");
        QueueNode & node = mQueuedRequests.front();
        if (!IsActive())
        {
            //  If not active, discard queued request
            if (node.connection)
            {
                // normal request, internal requests have no connection
                node.connection->NotifyDiscardRequest(node.request);
            }
            DestroyRequest(node.request);
        }
        else
        {
            //  Move request to active list
            if (node.connection)
            {
                // normal request, internal requests have no connection
                node.connection->NotifyBeginRequest(node.request);
            }
            mActiveRequests.push_back(node);
			mActiveCount++;
        }
        mQueuedRequests.pop_front();
    }

    ////////////////////////////////////////
    //  Process active requests
    unsigned requestsProcessed = 0;
    std::list<QueueNode>::iterator iterator = mActiveRequests.begin();
    while (iterator != mActiveRequests.end())
    {
		//Profile profile("Listener::Process (process request)");
        std::list<QueueNode>::iterator current = iterator++;
        RequestBase * request = current->request;
        Connection * connection = current->connection;
    
        if (request->Process())
        {
            if (connection)
            {
                // normal request, internal requests have no connection
                connection->NotifyEndRequest(request);
            }
            DestroyRequest(request);
            mActiveRequests.erase(current);
			mActiveCount--;
        }
		else if (mSleepingRequests.find(request) != mSleepingRequests.end())
		{
            mActiveRequests.erase(current);
		}
	    requestsProcessed++;
    }
    return requestsProcessed;
}

/*
void Listener::GetStats(UdpManagerStatistics & statsStruct)
{
	if (mTcpManager)
		mTcpManager->GetStats(&statsStruct);
}

void Listener::ResetStats()
{
	if (mTcpManager)
		mTcpManager->ResetStats();
}
*/

void Listener::OnStartup()
{
}

void Listener::OnShutdown()
{
}

void Listener::OnFailedStartup()
{
}

void Listener::OnConnectionOpened(Connection * connection)
{
}

void Listener::OnConnectionClosed(Connection * connection, const char * reason)
{
}

void Listener::OnConnectionDestroyed(Connection * connection)
{
}

void Listener::OnCrcReject(Connection *connection, const unsigned char * buffer, unsigned size)
{
}

/*
void Listener::OnPacketCorrupt(Connection *con, const unsigned char *data, int dataLen, UdpCorruptionReason reason)
{
}
*/

////////////////////////////////////////////////////////////////////////////////    


Connection::Connection(Listener & listener, TcpConnection * connection) :
	mListener(listener),
    mConnection(connection),
    mHost(),
	mHostIp(0),
	mDisconnectReason(0),
    mQueuedRequests(0),
    mActiveRequests(0)
{
	mConnection->AddRef();
	mConnection->SetHandler(this);

    char buffer[256];
    char addr[32];
	mHostIp = mConnection->GetDestinationIp().GetAddress();
    mConnection->GetDestinationIp().GetAddress(addr);
    snprintf(buffer, sizeof(buffer), "%s:%u", addr, mConnection->GetDestinationPort());
    mHost = buffer;

    //printf("ctor 0x%x connection\n",this);
}

Connection::~Connection()
{
	Disconnect();

    //printf("dtor 0x%x connection\n",this);
}

unsigned Connection::Send(const unsigned char * data, unsigned dataLen)
{
    if (mConnection)
    {
		mConnection->Send((const char*)data, dataLen);
        return dataLen;
    }
    return 0;
}

void Connection::Disconnect()
{
    if (mConnection)
        OnTerminated(mConnection);
}

bool Connection::IsConnected() const
{
    return mConnection != 0;
}

const std::string & Connection::GetHost() const 
{
    return mHost;
}

const unsigned Connection::GetHostIP() const
{
	return mHostIp;
}

unsigned Connection::GetQueuedRequests() const
{
    return mQueuedRequests;
}

unsigned Connection::GetActiveRequests() const
{
    return mActiveRequests;
}

void Connection::OnTerminated(TcpConnection *con)
{
    mConnection->SetHandler(0);
	mConnection->Disconnect();
	//TcpConnection::DisconnectReason disconnectReason = mConnection->GetDisconnectReason();
	mConnection->Release();
	mConnection = 0;
	mListener.mClosedConnections.push_back(this);
    mListener.OnConnectionClosed(this, "Test"/*TcpConnection::DisconnectReasonText(disconnectReason)*/);
}

void Connection::OnRoutePacket(TcpConnection *, const unsigned char * data, int dataLen)
{
    mListener.OnReceive(this, data, dataLen);
}

void Connection::OnCrcReject(TcpConnection *, const unsigned char * data, int dataLen)
{
    mListener.OnCrcReject(this, data, dataLen);
}

/*
void Connection::OnPacketCorrupt(TcpConnection *, const uchar *data, int dataLen, UdpCorruptionReason reason)
{
    mListener.OnPacketCorrupt(this, data, dataLen, reason);
}
*/

void Connection::NotifyQueuedRequest(RequestBase * request)
{
    mQueuedRequests++;
}

void Connection::NotifyDiscardRequest(RequestBase * request)
{
    mQueuedRequests--;
}

void Connection::NotifyBeginRequest(RequestBase * request)
{
    mQueuedRequests--;
    mActiveRequests++;
}

void Connection::NotifyEndRequest(RequestBase * request)
{
    mActiveRequests--;
}


////////////////////////////////////////////////////////////////////////////////    


RequestBase::RequestBase(Connection * connection, bool isInternal) :
    mConnection(connection),
    mProcessState(0),
    mIsInternal(isInternal)
{
}

RequestBase::~RequestBase()
{
}
#ifdef EXTERNAL_DISTRO
}
#endif

#endif
