#include "GenericApiCore.h"
#include "GenericConnection.h"
#include "GenericMessage.h"


#ifdef USE_SERIALIZE_LIB
    #include <Base/serialize.h>
#endif

using namespace std;

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace GenericAPI 
{


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
  m_suspended(false),
  m_nextConnectionIndex(0)
{
	m_serverConnections.push_back(new GenericConnection(host, port, this, reconnectTimeout, noDataTimeoutSecs, noAckTimeoutSecs, incomingBufSizeInKB, outgoingBufSizeInKB, keepAlive, maxRecvMessageSizeInKB));
}

GenericAPICore::GenericAPICore(const char *hosts[],
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
  m_suspended(false),
  m_nextConnectionIndex(0)
{
    for (unsigned i=0; i<arraySize; i++)
    {
        m_serverConnections.push_back(new GenericConnection(hosts[i], port[i], this, reconnectTimeout, noDataTimeoutSecs, noAckTimeoutSecs, incomingBufSizeInKB, outgoingBufSizeInKB, keepAlive, maxRecvMessageSizeInKB));
    }
}




GenericAPICore::~GenericAPICore()
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

void GenericAPICore::changeHostPort(unsigned connectionIndex, const char *host, short port)
{
	if (connectionIndex <= m_serverConnections.size() - 1)
	{
		m_serverConnections[connectionIndex]->changeHostPort(host, port);
	}
}

unsigned GenericAPICore::submitRequest(GenericRequest *req, GenericResponse *res)
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

unsigned GenericAPICore::submitRequest(GenericRequest *req, GenericResponse *res, unsigned reqTimeout)
{
	++m_outCount;
	if(m_currTrack == 0)
	{
		m_currTrack++;
	}
	req->setTrack(m_currTrack);
	res->setTrack(m_currTrack);
	time_t timeout = time(nullptr) + reqTimeout;

	req->setTimeout(timeout);
	res->setTimeout(timeout);

	m_outboundQueue.push(pair<GenericRequest *, GenericResponse *>(req, res));
	return(m_currTrack++);
}

void GenericAPICore::process()
{
	GenericRequest *req;
	GenericResponse *res;
	
	// Process timeout on pending requests - regardless of whether processing is suspended or not
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

	if (!m_suspended)
	{
		while(m_outCount > 0)
		{
            GenericConnection *con = getNextActiveConnection();
            if (con != nullptr)
            {
				pair<GenericRequest *, GenericResponse *> out_pair = m_outboundQueue.front();

                req = out_pair.first;
                res = out_pair.second;
                if (!req->isInputValid())
                {
                    res->setResult(req->getInputError());
                    --m_outCount;
			        m_outboundQueue.pop();

			        responseCallback(res);
			        delete res;
			        delete req;
                }
                else
                {
#ifdef USE_SERIALIZE_LIB
                    const unsigned char *msgBuf = nullptr;
                    unsigned msgSize = 0;
                    msgBuf = req->pack(msgSize);
                    con->Send(msgBuf, msgSize);
#else
				    Base::ByteStream msg;
				    req->pack(msg);
				    con->Send(msg);
#endif
				    m_pending.insert(pair<unsigned, GenericResponse *>(res->getTrack(), res));
				    --m_outCount;
				    ++m_pendingCount;
				    m_outboundQueue.pop();
				    delete req;
                }
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


GenericConnection *GenericAPICore::getNextActiveConnection()
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


};
#ifdef EXTERNAL_DISTRO
};
#endif
