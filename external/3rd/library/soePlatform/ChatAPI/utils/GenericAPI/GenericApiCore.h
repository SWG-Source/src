#if !defined (GENERICAPICORE_H_)
#define GENERICAPICORE_H_

#pragma warning (disable: 4786)

#include <map>
#include <queue>

#include <time.h>

#include <Base/Archive.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace GenericAPI 
{

class GenericRequest;
class GenericResponse;
class GenericConnection;
class GenericAPI;

class GenericAPICore
{
public:
	friend class GenericConnection;
	friend class GenericAPI;

	GenericAPICore(const char *host,
								   short port,
								   unsigned reqTimeout,
								   unsigned reconnectTimeout,
								   unsigned noDataTimeoutSecs = 5,
								   unsigned noAckTimeoutSecs = 5,
								   unsigned incomingBufSizeInKB = 32,
								   unsigned outgoingBufSizeInKB = 32,
								   unsigned keepAlive = 1,
                                   unsigned maxRecvMessageSizeInKB = 0);

    /**
     * NOTE:  arraySize must be actual size of host and port arrays.
     *   ALSO: cannot specify a 0 array size.
     */
    GenericAPICore(const char *hosts[],
								   const short port[],
                                   unsigned arraySize,
								   unsigned reqTimeout,
								   unsigned reconnectTimeout,
								   unsigned noDataTimeoutSecs = 5,
								   unsigned noAckTimeoutSecs = 5,
								   unsigned incomingBufSizeInKB = 32,
								   unsigned outgoingBufSizeInKB = 32,
								   unsigned keepAlive = 1,
                                   unsigned maxRecvMessageSizeInKB = 0);

	virtual ~GenericAPICore();

	void process();
#ifdef USE_SERIALIZE_LIB
    virtual void responseCallback(short type, const unsigned char *data, unsigned dataLen) = 0;
#else
	virtual void responseCallback(short type, Base::ByteStream::ReadIterator &iter) = 0;
#endif
	virtual void responseCallback(GenericResponse *R) = 0;

	virtual void OnDisconnect(const char *host, short port) = 0;
	virtual void OnConnect(const char *host, short port) = 0;

	void suspendProcessing() { m_suspended = true; }
	void resumeProcessing() { m_suspended = false; }

	// change the host and port for a connection object, will take effect
	// on connection's next CON_DISCONNECT state.
	void changeHostPort(unsigned connectionIndex, const char *host, short port);

	unsigned submitRequest(GenericRequest *req, GenericResponse *res);
	unsigned submitRequest(GenericRequest *req, GenericResponse *res, unsigned reqTimeout);

protected:	
    std::vector<GenericConnection *> m_serverConnections;
	std::map<unsigned, GenericResponse *> m_pending;
	std::queue<std::pair<GenericRequest *, GenericResponse *> > m_outboundQueue;
	unsigned m_currTrack;
	time_t m_reconnectTimeout;
	unsigned m_outCount;
	unsigned m_pendingCount;
	unsigned m_requestTimeout;
private:
	bool m_suspended;
    unsigned m_nextConnectionIndex;

    GenericConnection *getNextActiveConnection();
};

};
#ifdef EXTERNAL_DISTRO
};
#endif
#endif
