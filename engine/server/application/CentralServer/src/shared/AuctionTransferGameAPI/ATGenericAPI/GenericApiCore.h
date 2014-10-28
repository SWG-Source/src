#if !defined (GENERICAPICORE_H_)
#define GENERICAPICORE_H_

//----------------------------------------
//
// WARNING: These files are NOT standard generic API files
// They have been modified for this project.
// Do NOT replace them with generic API files
//
//----------------------------------------

#pragma warning (disable: 4786)

#include <map>
#include <queue>
#include <time.h>
#include "Base/Archive.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

class GenericRequest;
class GenericResponse;
class GenericConnection;

//----------------------------------------------
class ServerTrackObject 
//----------------------------------------------
{
public:
	ServerTrackObject(unsigned mapped_track, unsigned real_track, GenericConnection *con);
	~ServerTrackObject() { }

	inline GenericConnection	*getConnection()		{ return m_connection; }
	inline unsigned				getRealServerTrack()	{ return m_realTrack; }
	inline unsigned				getMappedServerTrack()	{ return m_mappedTrack; }

private:
	unsigned			m_mappedTrack;
	unsigned			m_realTrack;
	GenericConnection	*m_connection;
};

//----------------------------------------------
class GenericAPICore
//----------------------------------------------
{
public:
	friend class GenericConnection;

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
    GenericAPICore(const char *game, const char *hosts[],
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

	void				process();
	virtual void		responseCallback(short type, Base::ByteStream::ReadIterator &iter, GenericConnection *con) = 0;
	virtual void		responseCallback(GenericResponse *R) = 0;

	virtual void		OnDisconnect(GenericConnection *con) = 0;
	virtual void		OnConnect(GenericConnection *con) = 0;

	void				countOpenConnections();
	ServerTrackObject	*findServer(unsigned server_track);


	void				suspendProcessing() { m_suspended = true; }
	void				resumeProcessing()	{ m_suspended = false; }
	unsigned			submitRequest(GenericRequest *req, GenericResponse *res);
    GenericConnection	*getNextActiveConnection();
	std::string			&getGameCode()		{ return m_game; }
	void				addIdentifier(const std::string &id) { m_gameIdentifiers.push_back(id);}

protected:	
    std::vector<GenericConnection *>		m_serverConnections;
	std::map<unsigned, GenericResponse *>	m_pending;
	std::queue<std::pair<GenericRequest *, GenericResponse *> > m_outboundQueue;
	unsigned	m_currTrack;
	time_t		m_reconnectTimeout;
	unsigned	m_outCount;
	unsigned	m_pendingCount;
	unsigned	m_requestTimeout;
	unsigned	m_currentConnections;		// number currently connected
	unsigned	m_maxConnections;			// number that should be connected
	std::map<unsigned, ServerTrackObject *> m_serverTracks;
	std::vector<std::string> m_gameIdentifiers;

private:
	bool		m_suspended;
    unsigned	m_nextConnectionIndex;
	std::string m_game;
};

#ifdef EXTERNAL_DISTRO
};
#endif
#endif
