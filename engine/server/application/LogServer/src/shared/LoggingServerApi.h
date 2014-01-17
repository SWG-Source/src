#ifndef LOGGINGSERVERAPI_H
#define LOGGINGSERVERAPI_H

#include "UdpLibrary.hpp"

struct LoggingServerApiStatistics
{
	//These are all statistics from the UdpConnection object.
	udp_int64 totalBytesSent;
	udp_int64 totalBytesReceived;
	udp_int64 totalPacketsSent;			// total packets we have sent
	udp_int64 totalPacketsReceived;		// total packets we have received
    udp_int64 applicationPacketsSent;
    udp_int64 applicationPacketsReceived;
	double iterationsPerSecond;
	int totalTime;
};

class LoggingServerHandler;

class LoggingServerApi : public UdpConnectionHandler
{
	public:
		enum { cDefaultPort = 9877 };

		enum Status { cStatusDisconnected, cStatusConnected, cStatusNegotiating, cStatusAuthenticating };

		LoggingServerApi(LoggingServerHandler *handler, int queueSize = 4096);
		virtual ~LoggingServerApi();

		void Connect(const char *address, int port, const char *loginName, const char *password, const char *defaultDirectory);
		void Disconnect();
		
		void Flush(int timeout);		// blocking call to flush the log (timeout is how long to try, in ms)

		Status GetStatus() const;
		void GiveTime();
		void Log16(const char *filename, int typeCode, const unsigned short *ucs2String);		// no formatting support, application will have to do that on it's own
		void Log(const char *filename, int typeCode, const char *message, ...);		// printf style formatting
		
		void StartTransaction();
		void StopTransaction();

		void RequestMonitor(const char *filename, bool turnOn);
		void RequestFileList();
		void GetStatistics(LoggingServerApiStatistics *stats);

	protected:
		enum { cSafetyQueueTime = 3 * 60 * 1000 };	// 3 minutes

		LoggingServerHandler *mHandler;
		UdpManager *mUdpManager;
		bool mAuthenticated;
		bool mLoginSent;
		UdpConnection *mConnection;
		char mLoginName[256];
		char mPassword[256];
		char mDefaultDirectory[256];
		GroupLogicalPacket *mTransaction;
		int mSessionId;
		int mSessionSequence;

			// queue system
		struct QueueEntry
		{
			LogicalPacket *packet;
			UdpMisc::ClockStamp sentTime;
		};

		QueueEntry *mQueue;
		int mQueueSize;
		int mQueueCount;
		int mQueuePosition;


		void FlushQueue();
		void PacketSend(LogicalPacket *lp);
		void LogPacket(char *data, int len);
		LogicalPacket *CreatePacket(char *data, int dataLen);
	public:
			// these are public so the server-application can see them
		enum C2SPacket { cC2SPacketNotUsed, cC2SPacketLogin, cC2SPacketLog, cC2SPacketMonitor, cC2SPacketFileList, cC2SPacketLog16 };
		enum S2CPacket { cS2CPacketNotUsed, cS2CPacketLoginConfirm, cS2CPacketMonitor, cS2CPacketFileList };

		virtual void OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen);
		virtual void OnTerminated(UdpConnection *con);
};

class LoggingServerHandler
{
	public:
		virtual void LshOnLoginConfirm(bool authenticated);
		virtual void LshOnMonitor(int sessionId, int sequenceNumber, const char * const name, const char * const filename, int typeCode, const char * const message);
		virtual void LshOnFileList(const char * const fileList);
		virtual void LshOnTerminated(UdpConnection::DisconnectReason disconnect);
};






#endif

