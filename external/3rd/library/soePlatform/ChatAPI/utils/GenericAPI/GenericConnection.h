#if !defined (GENERICCONNECTION_H_)
#define GENERICCONNECTION_H_

#include <Base/Archive.h>

#ifdef USE_TCP_LIBRARY
    #include <TcpLibrary/TcpManager.h>
    #include <TcpLibrary/TcpConnection.h>
    #include <TcpLibrary/TcpHandlers.h>
#else //default to UDP_LIBRARY
    #include <UdpLibrary/UdpLibrary.h>
    #include <UdpLibrary/UdpHandler.h>
#endif


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace GenericAPI 
{

	enum eConState
	{
		CON_DISCONNECT,
		CON_NEGOTIATE,
		CON_CONNECT
	};

#ifdef USE_TCP_LIBRARY
    class GenericConnection : public TcpConnectionHandler
#else //default to UDP_LIBRARY
	class GenericConnection : public UdpLibrary::UdpConnectionHandler
#endif
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
                          unsigned maxRecvMessageSizeInKB = 0,
						  unsigned holdTime = 0);

		virtual ~GenericConnection();

#ifdef USE_TCP_LIBRARY
		virtual void OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
		virtual void OnTerminated(TcpConnection *con);
#else //default to UDP_LIBRARY
		virtual void OnRoutePacket(UdpLibrary::UdpConnection *con, const unsigned char *data, int dataLen);
		virtual void OnTerminated(UdpLibrary::UdpConnection *con);
#endif
#ifdef USE_SERIALIZE_LIB
        void Send(const unsigned char *data, int dataLen);
#else
		void Send(Base::ByteStream &msg);
#endif
        
		
		void changeHostPort(const char *host, short port);
		bool isConnected() { return m_bConnected; }
		void disconnect();
		void process(bool giveTime = true);

		static unsigned	ms_crcBytes;
	private:
		bool m_bConnected;
		GenericAPICore *m_apiCore;
#ifdef USE_TCP_LIBRARY
        TcpManager *m_manager;
        TcpConnection *m_con;
#else //default to UDP_LIBRARY
		UdpLibrary::UdpManager *m_manager;
		UdpLibrary::UdpConnection *m_con;
#endif
		std::string m_host;
		std::string m_nextHost;
		short m_port;
		short m_nextPort;

		unsigned m_lastTrack;
		
		eConState m_conState;
		time_t m_conTimeout;
		unsigned m_reconnectTimeout;
	};

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
