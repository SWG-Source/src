#include "FirstLogServer.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "LoggingServerApi.h"

LoggingServerApi::LoggingServerApi(LoggingServerHandler *handler, int queueSize)
{
	mHandler = handler;
	mQueueSize = queueSize;
	mQueue = new QueueEntry[mQueueSize];
	mQueuePosition = 0;
	mQueueCount = 0;
	mLoginSent = 0;

	mAuthenticated = false;
	mLoginName[0] = 0;
	mPassword[0] = 0;
	mDefaultDirectory[0] = 0;
	mConnection = nullptr;
	mUdpManager = nullptr;
	mTransaction = nullptr;
	mSessionId = int(time(nullptr));
	mSessionSequence = 1;
}

LoggingServerApi::~LoggingServerApi()
{
	if (mTransaction != nullptr)
		StopTransaction();

	for (int i = 0; i < mQueueCount; i++)
	{
		int spot = (mQueuePosition + i) % mQueueSize;
		mQueue[spot].packet->Release();
	}
	delete[] mQueue;

	Disconnect();
}

void LoggingServerApi::Connect(const char *address, int port, const char *loginName, const char *password, const char *defaultDirectory)
{
	Disconnect();

	strcpy(mLoginName, loginName);
	strcpy(mPassword, password);
	strcpy(mDefaultDirectory, defaultDirectory);

	UdpManager::Params params;
	params.clockSyncDelay = 0;
	params.crcBytes = 2;
	params.hashTableSize = 10;
	params.incomingBufferSize = 16 * 1024;
	params.keepAliveDelay = 30000;
	params.portAliveDelay = 0;
	params.noDataTimeout = 91000;
	params.maxConnections = 3;
	params.maxRawPacketSize = 1460;
	params.maxDataHoldTime = 0;

	params.outgoingBufferSize = 128 * 1024;
	params.packetHistoryMax = 3;
	params.port = 0;
	params.pooledPacketMax = 1000;
	params.reliable[0].maxInstandingPackets = 512;
	params.reliable[0].maxOutstandingBytes = 128 * 1024;
	params.reliable[0].congestionWindowMinimum = 50000;
	params.reliable[0].maxOutstandingPackets = 512;
	params.reliable[0].processOnSend = false;
	mUdpManager = new UdpManager(&params);

	mConnection = mUdpManager->EstablishConnection(address, port, 30000);
	if (mConnection != nullptr)
		mConnection->SetHandler(this);
	mLoginSent = false;
}

void LoggingServerApi::Disconnect()
{
	mAuthenticated = false;
	mLoginName[0] = 0;
	mPassword[0] = 0;
	mDefaultDirectory[0] = 0;

	if (mConnection != nullptr)
	{
		mConnection->Disconnect();
		mConnection->Release();
		mConnection = nullptr;
	}

	if (mUdpManager != nullptr)
	{
		mUdpManager->Release();
		mUdpManager = nullptr;
	}
}

void LoggingServerApi::Flush(int timeout)
{
	UdpMisc::ClockStamp startTime = UdpMisc::Clock();
	while (GetStatus() != cStatusDisconnected && UdpMisc::ClockElapsed(startTime) < timeout)
	{
		if (mConnection->TotalPendingBytes() == 0 && mQueueCount == 0)
			break;
		GiveTime();

		UdpMisc::Sleep(10);
	}
	UdpMisc::Sleep(20);
}

LoggingServerApi::Status LoggingServerApi::GetStatus() const
{
	if (mConnection != nullptr)
	{
		switch (mConnection->GetStatus())
		{
		case UdpConnection::cStatusConnected:
			if (mAuthenticated)
				return(cStatusConnected);
			else
				return(cStatusAuthenticating);
			break;
		case UdpConnection::cStatusNegotiating:
			return(cStatusNegotiating);
		default:
			break;
		}
	}
	return(cStatusDisconnected);
}

void LoggingServerApi::OnRoutePacket(UdpConnection *, const uchar *data, int)
{
	switch (data[0])
	{
	case cS2CPacketLoginConfirm:
	{
		mAuthenticated = data[1] ? true : false;
		if (mAuthenticated)
		{
			// flush the queue on authentication.  This sends not only any data that was queued before connecting
			// but also any data that was sent in the last 3 minutes, just to ensure that it didn't get lost on the
			// server side of things
			FlushQueue();
		}

		if (mHandler != nullptr)
			mHandler->LshOnLoginConfirm(mAuthenticated);
		break;
	}
	case cS2CPacketMonitor:
	{
		char *ptr = (char *)(data + 1);
		int sessionId = UdpMisc::GetValue32(ptr);
		ptr += 4;
		int sequenceNumber = UdpMisc::GetValue32(ptr);
		ptr += 4;
		int typeCode = UdpMisc::GetValue32(ptr);
		ptr += 4;
		char *name = ptr;
		ptr += strlen(ptr) + 1;
		char *filename = ptr;
		ptr += strlen(ptr) + 1;
		char *message = ptr;
		if (mHandler != nullptr)
			mHandler->LshOnMonitor(sessionId, sequenceNumber, name, filename, typeCode, message);
		break;
	}
	case cS2CPacketFileList:
	{
		if (mHandler != nullptr)
			mHandler->LshOnFileList((char *)(data + 1));
		break;
	}
	default:
		break;
	}
}

void LoggingServerApi::OnTerminated(UdpConnection *con)
{
	if (mHandler != nullptr)
	{
		mHandler->LshOnTerminated(con->GetDisconnectReason());
	}
}

void LoggingServerApi::GiveTime()
{
	if (mConnection != nullptr && mConnection->GetStatus() == UdpConnection::cStatusConnected)
	{
		if (!mLoginSent)
		{
			mLoginSent = true;

			// send login packet
			char buf[1024];
			char *ptr = buf;
			*ptr++ = cC2SPacketLogin;
			strcpy(ptr, mLoginName);
			ptr += strlen(mLoginName) + 1;
			strcpy(ptr, mPassword);
			ptr += strlen(mPassword) + 1;
			strcpy(ptr, mDefaultDirectory);
			ptr += strlen(mDefaultDirectory) + 1;
			ptr += UdpMisc::PutValue32(ptr, mSessionId);
			mConnection->Send(cUdpChannelReliable1, buf, ptr - buf);
		}
	}

	while (GetStatus() == cStatusConnected && mQueueCount > 0)
	{
		int spot = mQueuePosition % mQueueSize;

		// if entry was actually sent and it has been sitting in the queue long enough to be deleted
		if (mQueue[spot].sentTime != 0 && UdpMisc::ClockElapsed(mQueue[spot].sentTime) > cSafetyQueueTime)
		{
			mQueue[spot].packet->Release();
			mQueue[spot].packet = nullptr;
			mQueuePosition++;
			mQueueCount--;
		}
		else
		{
			break;
		}
	}

	if (mUdpManager != nullptr)
	{
		mUdpManager->GiveTime();
	}
}

void LoggingServerApi::StartTransaction()
{
	if (mTransaction == nullptr)
		mTransaction = new GroupLogicalPacket();
}

void LoggingServerApi::StopTransaction()
{
	if (mTransaction != nullptr)
	{
		PacketSend(mTransaction);
		mTransaction->Release();
		mTransaction = nullptr;
	}
}

void LoggingServerApi::PacketSend(LogicalPacket *lp)
{
	// make room in queue
	if (mQueueCount == mQueueSize)
	{
		int spot = mQueuePosition % mQueueSize;
		mQueue[spot].packet->Release();
		mQueue[spot].packet = nullptr;
		mQueuePosition++;
		mQueueCount--;
	}

	// queue it
	int spot = (mQueuePosition + mQueueCount) % mQueueSize;
	lp->AddRef();
	mQueue[spot].packet = lp;
	mQueue[spot].sentTime = 0;
	mQueueCount++;

	// if possible, send it
	if (GetStatus() == cStatusConnected)
	{
		mQueue[spot].sentTime = UdpMisc::Clock();
		mConnection->Send(cUdpChannelReliable1, lp);
	}
}

void LoggingServerApi::Log16(const char *filename, int typeCode, const unsigned short *ucs2String)
{
	char buffer[32768];
	char * const ptr_end_buffer = &buffer[sizeof(buffer)];
	char *ptr = buffer;
	*ptr++ = cC2SPacketLog16;
	ptr += UdpMisc::PutValue32(ptr, typeCode);
	strcpy(ptr, filename);
	ptr += strlen(ptr) + 1;

	const unsigned short *rawPtr = ucs2String;
	char *startPtr = ptr;
	for (;;)
	{
		if (*rawPtr == '\n' || *rawPtr == 0)
		{
			if (ptr != startPtr)
			{
				ptr += UdpMisc::PutValue16(ptr, 0);		// terminating zero
				ptr += UdpMisc::PutValue32(ptr, mSessionSequence++);
				LogPacket(buffer, ptr - buffer);
				ptr = startPtr;
			}

			if (*rawPtr == 0)
				break;
		}
		else
		{
			// catch buffer overflow; we need to reserve 6 bytes
			// at the end of the buffer for the terminating 0
			// and the sequence number (see above)
			if ((ptr + 8) <= ptr_end_buffer)
				ptr += UdpMisc::PutValue16(ptr, *rawPtr);
		}

		rawPtr++;
	}
}

void LoggingServerApi::Log(const char *filename, int typeCode, const char *message, ...)
{
	char buffer[16384];
	char * const ptr_end_buffer = &buffer[sizeof(buffer)];
	char *ptr = buffer;
	*ptr++ = cC2SPacketLog;
	ptr += UdpMisc::PutValue32(ptr, typeCode);
	strcpy(ptr, filename);
	ptr += strlen(ptr) + 1;

	char rawString[16384];
	va_list marker;
	va_start(marker, message);
	_vsnprintf(rawString, sizeof(rawString), message, marker);
	va_end(marker);
	rawString[sizeof(rawString) - 1] = 0;

	char *rawPtr = rawString;
	char *startPtr = ptr;
	for (;;)
	{
		if (*rawPtr == '\n' || *rawPtr == 0)
		{
			if (ptr != startPtr)
			{
				*ptr++ = 0;
				ptr += UdpMisc::PutValue32(ptr, mSessionSequence++);
				LogPacket(buffer, ptr - buffer);
				ptr = startPtr;
			}

			if (*rawPtr == 0)
				break;
		}
		else
		{
			// catch buffer overflow; we need to reserve 5 bytes
			// at the end of the buffer for the terminating 0
			// and the sequence number (see above)
			if ((ptr + 6) <= ptr_end_buffer)
				*ptr++ = *rawPtr;
		}

		rawPtr++;
	}
}

void LoggingServerApi::LogPacket(char *data, int len)
{
	if (mTransaction != nullptr)
	{
		// add it to the transaction
		mTransaction->AddPacket(data, len);
	}
	else
	{
		LogicalPacket *lp = CreatePacket(data, len);
		PacketSend(lp);
		lp->Release();
	}
}

LogicalPacket *LoggingServerApi::CreatePacket(char *data, int dataLen)
{
	if (mUdpManager != nullptr)
	{
		return(mUdpManager->CreatePacket(data, dataLen));
	}

	return(UdpMisc::CreateQuickLogicalPacket(data, dataLen));
}

void LoggingServerApi::FlushQueue()
{
	if (GetStatus() == cStatusConnected)
	{
		for (int i = 0; i < mQueueCount; i++)
		{
			int spot = (mQueuePosition + i) % mQueueSize;
			mQueue[spot].sentTime = UdpMisc::Clock();
			mConnection->Send(cUdpChannelReliable1, mQueue[spot].packet);
		}
	}
}

void LoggingServerApi::RequestMonitor(const char *filename, bool turnOn)
{
	if (GetStatus() == cStatusConnected)
	{
		char buffer[2048];
		char *ptr = buffer;
		*ptr++ = cC2SPacketMonitor;
		*ptr++ = turnOn;
		strcpy(ptr, filename);
		ptr += strlen(ptr) + 1;
		mConnection->Send(cUdpChannelReliable1, buffer, ptr - buffer);
	}
}

void LoggingServerApi::RequestFileList()
{
	if (GetStatus() == cStatusConnected)
	{
		char buffer[10];
		char *ptr = buffer;
		*ptr++ = cC2SPacketFileList;
		mConnection->Send(cUdpChannelReliable1, buffer, ptr - buffer);
	}
}

void LoggingServerApi::GetStatistics(LoggingServerApiStatistics *stats)
{
	memset(stats, 0, sizeof(*stats));
	UdpConnectionStatistics udpConnectionStats;
	memset(&udpConnectionStats, 0, sizeof(udpConnectionStats));
	if (mConnection != nullptr)
	{
		mConnection->GetStats(&udpConnectionStats);
		stats->applicationPacketsReceived = udpConnectionStats.applicationPacketsReceived;
		stats->applicationPacketsSent = udpConnectionStats.applicationPacketsSent;
		stats->totalBytesReceived = udpConnectionStats.totalBytesReceived;
		stats->totalBytesSent = udpConnectionStats.totalBytesSent;
		stats->totalPacketsReceived = udpConnectionStats.totalPacketsReceived;
		stats->totalPacketsSent = udpConnectionStats.totalPacketsSent;

		stats->totalTime = mConnection->ConnectionAge();
	}

	UdpManagerStatistics managerStats;
	if (mUdpManager != nullptr)
	{
		mUdpManager->GetStats(&managerStats);
		udp_int64 iterations = managerStats.iterations;
		int elapseTime = managerStats.elapsedTime;

		if (elapseTime != 0)
		{
			stats->iterationsPerSecond = (double)((iterations * 1000) / (double)elapseTime);
		}
		mUdpManager->ResetStats();
	}
}

void LoggingServerHandler::LshOnLoginConfirm(bool)
{
}

void LoggingServerHandler::LshOnMonitor(int, int, const char * const, const char * const, int, const char * const)
{
}

void LoggingServerHandler::LshOnFileList(const char * const)
{
}

void LoggingServerHandler::LshOnTerminated(UdpConnection::DisconnectReason)
{
}