#include "GenericApiCore.h"
#include "GenericConnection.h"
#include "GenericMessage.h"

//----------------------------------------
//
// WARNING: These files are NOT standard generic API files
// They have been modified for this project.
// Do NOT replace them with generic API files
//
//----------------------------------------

using namespace std;

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif

//----------------------------------------
ServerTrackObject::ServerTrackObject(unsigned mapped_track, unsigned real_track, GenericConnection *con)
: m_mappedTrack(mapped_track), m_realTrack(real_track), m_connection(con)
//----------------------------------------
{
}

//----------------------------------------
GenericAPICore::GenericAPICore(const char *host, 
									short port, 
									unsigned reqTimeout, 
									unsigned reconnectTimeout, 
									unsigned noDataTimeoutSecs, 
									unsigned noAckTimeoutSecs, 
									unsigned incomingBufSizeInKB, 
									unsigned outgoingBufSizeInKB, 
									unsigned keepAlive,
                                    unsigned maxRecvMessageSizeInKB)
: m_currTrack(0),
  m_reconnectTimeout(0),
  m_outCount(0),
  m_pendingCount(0),
  m_requestTimeout(reqTimeout),
  m_currentConnections(0), m_maxConnections(0),
  m_suspended(false),
  m_nextConnectionIndex(0)
//----------------------------------------
{
	GenericConnection *con = new GenericConnection(host, port, this, reconnectTimeout, noDataTimeoutSecs, noAckTimeoutSecs, incomingBufSizeInKB, outgoingBufSizeInKB, keepAlive, maxRecvMessageSizeInKB);
	m_serverConnections.push_back(con);
}

//----------------------------------------
GenericAPICore::GenericAPICore(const char *game, const char *hosts[],
								   const short port[],
                                   unsigned arraySize,
								   unsigned reqTimeout,
								   unsigned reconnectTimeout,
								   unsigned noDataTimeoutSecs,
								   unsigned noAckTimeoutSecs,
								   unsigned incomingBufSizeInKB,
								   unsigned outgoingBufSizeInKB,
								   unsigned keepAlive,
                                   unsigned maxRecvMessageSizeInKB)
: m_currTrack(0),
  m_reconnectTimeout(0),
  m_outCount(0),
  m_pendingCount(0),
  m_requestTimeout(reqTimeout),
  m_currentConnections(0), m_maxConnections(0),
  m_suspended(false),
  m_nextConnectionIndex(0),
  m_game(game)
//----------------------------------------
{
    for (unsigned i=0; i<arraySize; i++)
    {
		GenericConnection *con = new GenericConnection(hosts[i], port[i], this, reconnectTimeout, noDataTimeoutSecs, noAckTimeoutSecs, incomingBufSizeInKB, outgoingBufSizeInKB, keepAlive, maxRecvMessageSizeInKB);
		m_serverConnections.push_back(con);
    }
}


//----------------------------------------
GenericAPICore::~GenericAPICore()
//----------------------------------------
{
    for (std::vector<GenericConnection *>::iterator conIter = m_serverConnections.begin(); conIter != m_serverConnections.end(); ++conIter)
    {
        GenericConnection *con = *conIter;
        delete con;
    }
    m_serverConnections.clear();

	for(map<unsigned, GenericResponse *>::iterator iter = m_pending.begin(); iter != m_pending.end(); ++iter)
	{
		delete (*iter).second;
	}

	while(m_outCount > 0)
	{
		delete m_outboundQueue.front().second;
		delete m_outboundQueue.front().first;
		m_outboundQueue.pop();
		--m_outCount;
	}
}

//----------------------------------------
unsigned GenericAPICore::submitRequest(GenericRequest *req, GenericResponse *res)
//----------------------------------------
{
	++m_outCount;
	if(m_currTrack == 0)
	{
		m_currTrack++;
	}
	req->setTrack(m_currTrack);
	res->setTrack(m_currTrack);
	time_t timeout = time(nullptr) + m_requestTimeout;

	req->setTimeout(timeout);
	res->setTimeout(timeout);

	m_outboundQueue.push(pair<GenericRequest *, GenericResponse *>(req, res));
	return(m_currTrack++);
}

//----------------------------------------
void GenericAPICore::process()
//----------------------------------------
{
	GenericRequest *req;
	GenericResponse *res;

	if (!m_suspended)
	{
		// Process timeout on pending requests
		while((m_outCount > 0) && ((req = m_outboundQueue.front().first)->getTimeout() <= time(nullptr)))
		{
			--m_outCount;
			res = m_outboundQueue.front().second;
			m_outboundQueue.pop();

			responseCallback(res);
			delete res;
			delete req;
		}

		// Process timeout on pending responses
		while((m_pendingCount > 0) && ((res = (*m_pending.begin()).second)->getTimeout() <= time(nullptr)))
		{
			--m_pendingCount;
			m_pending.erase(m_pending.begin());
			responseCallback(res);
			delete res;
		}

		while(m_outCount > 0)
		{
			pair<GenericRequest *, GenericResponse *> out_pair = m_outboundQueue.front();
            req = out_pair.first;
            res = out_pair.second;
			GenericConnection *con = nullptr;
			if (req->getMappedServerTrack() == 0)					// request has no originating "owner" server
			{
				con = getNextActiveConnection();					// it does not matter which server we send this to
			}
			else
			{
				ServerTrackObject *stobj = findServer(req->getMappedServerTrack());
				if (stobj)
				{
					con = stobj->getConnection();						// the server connection to respond to
					req->setServerTrack(stobj->getRealServerTrack());	// map server track back to REAL server track
					//printf("\nUnmapping %d to %d", stobj->getMappedServerTrack(), req->getMappedServerTrack());	//debug
					delete stobj;
				}
			}

            if (con != nullptr)
            {
				Base::ByteStream msg;
				req->pack(msg);
				con->Send(msg);
				m_pending.insert(pair<unsigned, GenericResponse *>(res->getTrack(), res));
				--m_outCount;
				++m_pendingCount;
				m_outboundQueue.pop();
				delete req;
            }
            else
            {
                //no active connections
                break; //from while loop
            }

		}
	}

   for (std::vector<GenericConnection *>::iterator conIter = m_serverConnections.begin(); conIter != m_serverConnections.end(); ++conIter)
    {
		GenericConnection *con = *conIter;
	    con->process();
    }
}


//----------------------------------------
GenericConnection *GenericAPICore::getNextActiveConnection()
//----------------------------------------
{
    unsigned startIndex = m_nextConnectionIndex;
    unsigned maxIndex = m_serverConnections.size() - 1;

    GenericConnection *con = nullptr;

    //loop until we find an active connection, or until we get back
    //              to where we started
    do
    {
        if (m_serverConnections[m_nextConnectionIndex]->isConnected())
        {
            con = m_serverConnections[m_nextConnectionIndex];
            if (m_nextConnectionIndex == maxIndex)
                m_nextConnectionIndex  = 0;
            else
                m_nextConnectionIndex++;
            
        }
        else if (++m_nextConnectionIndex > maxIndex)
        {
            //went past end of vector, start back at 0
            m_nextConnectionIndex = 0;
        }
    }while (con == nullptr && m_nextConnectionIndex != startIndex);

    return con;
}

//----------------------------------------
void GenericAPICore::countOpenConnections()
//----------------------------------------
{
	m_currentConnections = 0;
	m_maxConnections = m_serverConnections.size();

    for (std::vector<GenericConnection *>::iterator conIter = m_serverConnections.begin(); conIter != m_serverConnections.end(); ++conIter)
    {
		GenericConnection *con = *conIter;
		if (con->isConnected())
			++m_currentConnections;
    }
}

//----------------------------------------
ServerTrackObject *GenericAPICore::findServer(unsigned server_track)
//----------------------------------------
{
	std::map<unsigned, ServerTrackObject *>::iterator iter = m_serverTracks.find(server_track);
	if (iter == m_serverTracks.end())
		return nullptr;
	ServerTrackObject *stobj = (*iter).second;
	m_serverTracks.erase(server_track);
	return stobj;
}

#ifdef EXTERNAL_DISTRO
};
#endif
