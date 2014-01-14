// Connection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#ifndef	INCLUDED_Connection_H
#define	INCLUDED_Connection_H

//-----------------------------------------------------------------------


#include "sharedNetwork/NetworkHandler.h"

#include "sharedFoundation/Watcher.h"
#include <string>
#include <vector>

namespace Archive
{
	class ByteStream;
}

class ConnectionHandler;
class DeferredSend;
class NetworkSetupData;
class TcpClient;
class UdpConnectionMT;

//-----------------------------------------------------------------------

class Connection : public NetworkHandler
{
public:
	Connection   (const std::string & remoteAddress, const unsigned short remotePort, const NetworkSetupData & setup);
	explicit Connection   (UdpConnectionMT * newConnection, TcpClient * t = 0);

	virtual   ~Connection  ();
	void                  disconnect               ();
	void                  describeConnection       (const std::string &);
	const std::string &   getConnectionDescription () const;
	const std::string &   getRemoteAddress         () const;
	const unsigned short  getRemotePort            () const;
	const UdpConnectionMT * getUdpConnection         () const;
	UdpConnectionMT *       getUdpConnection         ();
	WatchedByList        &getWatchedByList         () const;
	uint16                getServerSyncStampShort  () const;
	uint32                getServerSyncStampLong   () const;
	Service *             getService               ();
	int                   getPendingBytes          () const;
	int                   getOldestUnacknowledgedAge() const;
	int                   getCongestionWindowSize  () const;
	int                   getAckAveragePing        () const;
	int                   getLastSend              () const;
	int                   getLastReceive           () const;
	TcpClient *           getTcpClient             ();
	void                  setTcpClientPendingSendAllocatedSizeLimit(unsigned int limit);

	virtual void          onConnectionClosed       () = 0;
	virtual void          onConnectionOpened       () = 0;
	virtual void          onConnectionOverflowing  (const unsigned int bytesPending);
	virtual void          onConnectionStalled      (const unsigned long stallTimeMs);
	virtual void          reportReceive            (const Archive::ByteStream & bs);
	virtual void          reportSend               (const Archive::ByteStream & bs);
	virtual void          onReceive                (const Archive::ByteStream & bs) = 0;
	void                  receive                  (const Archive::ByteStream & bs);
	virtual void          send                     (const Archive::ByteStream & data, const bool reliable);
	void                  sendSharedPacket         (const LogicalPacket * packet, const bool reliable);
	void                  setNoDataTimeout         (const int timeout);
	void                  setOverflowLimit         (const int newLimit);
	void                  setDisconnecting         ();
	void                  setDisconnectReason      (char const *fmt, ...);
	std::string const &   getDisconnectReason      () const;

	static void           install                  ();
	static void           remove                   ();

protected:
	friend class Service;
	friend class NetworkHandler;
	friend class TcpClient;
	virtual void  onConnectionClosed       (Connection *);
	void          onConnectionOpened       (UdpConnectionMT *);
	int           flushAndConfirmAllData   ();
	void          setService               (Service * s);
	void          receive                  (const unsigned char * const buffer, int length);
	static void   update                   ();

	virtual bool  isNetLogConnection       () const;
	void	      setRawTCP 	       ( bool bNewValue );

private:
	Connection & operator = (const Connection & rhs);
	Connection(const Connection & source);

	void  checkOverflow  (unsigned int bytesPending);
	void  flush          ();
	void  reportSend     (const int sendSize);
	void  reportDeferredMessages() const;

protected:
	UdpConnectionMT *  udpConnection;
	std::vector<std::pair<std::string, int> >  m_pendingPackets;
	uint64                       m_currentFrame;
	bool			     m_rawTCP;

private:
	std::string                  m_remoteAddress;
	unsigned short               m_remotePort;
	std::vector<DeferredSend *>  m_deferredMessages;
	int                          m_deferredDataSize;
	mutable WatchedByList        m_watchedByList;
	int                          m_overflowLimit;
	int                          m_bytesReceived;
	int                          m_bytesSent;
	int                          m_sendPeakBytesPerSecond;
	int                          m_sendAverageBytesPerSecond;
	unsigned long                m_connectionStartTime;
	int                          m_recvPeakBytesPerSecond;
	int                          m_recvAverageBytesPerSecond;
	unsigned long                m_lastRecvTime;
	unsigned long                m_lastSendTime;
	unsigned long                m_lastRecvReportTime;
	unsigned long                m_lastSendReportTime;
	unsigned long                m_recvBytesReportInterval;
	unsigned long                m_sendBytesReportInterval;
	Service *                    m_service;
	std::string                  m_description;
	ConnectionHandler *          m_connectionHandler;
	TcpClient *                  m_tcpClient;
	Archive::ByteStream *        m_tcpHeader;
	Archive::ByteStream *        m_tcpInput;
	bool                         m_disconnecting;
	std::string                  m_disconnectReason;
};

inline WatchedByList &Connection::getWatchedByList() const
{
	return m_watchedByList;
}


//-----------------------------------------------------------------------

#endif	// _INCLUDED_Connection_H
