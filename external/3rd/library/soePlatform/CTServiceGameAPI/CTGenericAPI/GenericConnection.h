#if !defined (GENERICCONNECTION_H_)
#define GENERICCONNECTION_H_

//----------------------------------------
//
// WARNING: These files are NOT standard generic API files
// They have been modified for this project.
// Do NOT replace them with generic API files
//
//----------------------------------------

#include <Base/Archive.h>
#include <TcpLibrary/TcpManager.h>
#include <TcpLibrary/TcpConnection.h>
#include <TcpLibrary/TcpHandlers.h>


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

static const unsigned API_VERSION_CODE	= 1;

enum eConState
{
	CON_DISCONNECT,
	CON_NEGOTIATE,
	CON_CONNECT
};
enum eConnectStatus
{
	CON_NONE,
	CON_CONNECTED,
	CON_IDENTIFIED
};

class GenericConnection : public TcpConnectionHandler
	{
	public:
		GenericConnection(const char *host,
						  short port,
						  GenericAPICore *apiCore,
						  unsigned reconnectTimeout,
						  unsigned noDataTimeoutSecs = 5,
						  unsigned noAckTimeoutSecs = 5,
						  unsigned incomingBufSizeInKB = 32,
						  unsigned outgoingBufSizeInKB = 32,
						  unsigned keepAlive = 1,
                          unsigned maxRecvMessageSizeInKB = 0);

		virtual ~GenericConnection();

		virtual void		OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
		virtual void		OnTerminated(TcpConnection *con);
		void				Send(Base::ByteStream &msg);
		inline const char	*getHost()	const	{ return m_host.c_str(); }
		inline const short	getPort()	const	{ return m_port; }
        
		inline eConnectStatus isConnected()		{ return m_bConnected; }
		void				disconnect();
		void				process();

private:
		eConnectStatus	m_bConnected;
		GenericAPICore	*m_apiCore;
        TcpManager		*m_manager;
        TcpConnection	*m_con;
		std::string		m_host;
		short			m_port;
		unsigned		m_lastTrack;
		eConState		m_conState;
		time_t			m_conTimeout;
		unsigned		m_reconnectTimeout;
	};

#ifdef EXTERNAL_DISTRO
};
#endif

#endif
