// ======================================================================
//
// ManagerHandler.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedCompression/ZlibCompressor.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/ManagerHandler.h"
#include "sharedNetwork/UdpConnectionMT.h"

// ======================================================================

namespace ManagerHandlerNamespace
{
	int gs_recvTotalCompressedBytes = 0;
	int gs_recvTotalUncompressedBytes = 0;
	int gs_sendTotalCompressedBytes = 0;
	int gs_sendTotalUncompressedBytes = 0;
}

using namespace ManagerHandlerNamespace;

// ======================================================================

ManagerHandler::ManagerHandler(NetworkHandler *owner) :
	UdpManagerHandlerMT(),
	m_owner(owner),
	m_compressor(new ZlibCompressor()),
	m_recvCompressedBytes(0),
	m_recvUncompressedBytes(0),
	m_sendCompressedBytes(0),
	m_sendUncompressedBytes(0)
{
}

// ----------------------------------------------------------------------

ManagerHandler::~ManagerHandler()
{
	delete m_compressor;
	m_compressor = 0;
}

// ----------------------------------------------------------------------

void ManagerHandler::setOwner(NetworkHandler *owner)
{
	m_owner = owner;
}

// ----------------------------------------------------------------------

void ManagerHandler::OnConnectRequest(UdpConnectionMT * con)
{ 
	NetworkHandler::onConnect(m_owner, con); 
}

// ----------------------------------------------------------------------

int ManagerHandler::getRecvCompressedByteCount() const
{
	return m_recvCompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getRecvTotalCompressedByteCount()
{
	return gs_recvTotalCompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getRecvTotalUncompressedByteCount() 
{
	return gs_recvTotalUncompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getRecvUncompressedByteCount() const
{
	return m_recvUncompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getSendCompressedByteCount() const
{
	return m_sendCompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getSendTotalCompressedByteCount() 
{
	return gs_sendTotalCompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getSendTotalUncompressedByteCount()
{
	return gs_sendTotalUncompressedBytes;
}

// ----------------------------------------------------------------------

int ManagerHandler::getSendUncompressedByteCount() const
{
	return m_sendUncompressedBytes;
}

// ----------------------------------------------------------------------

float ManagerHandler::getCompressionRatio() const
{
	float result = 0.0;
	if(m_sendCompressedBytes + m_recvCompressedBytes > 0)
		result = static_cast<float>(static_cast<float>(m_sendUncompressedBytes + m_recvUncompressedBytes) / static_cast<float>(m_sendCompressedBytes + m_recvCompressedBytes));
	else
		result = 1.0f;
	return result;
}

// ----------------------------------------------------------------------

float ManagerHandler::getTotalCompressionRatio() 
{
	float result = 0.0;
	if(gs_sendTotalCompressedBytes + gs_recvTotalCompressedBytes > 0)
		result = static_cast<float>(static_cast<float>(gs_sendTotalUncompressedBytes + gs_recvTotalUncompressedBytes) / static_cast<float>(gs_sendTotalCompressedBytes + gs_recvTotalCompressedBytes));
	else
		result = 1.0f;
	return result;
}

// ----------------------------------------------------------------------

int ManagerHandler::OnUserSuppliedEncrypt(UdpConnectionMT *, uchar *destData, const uchar *sourceData, int sourceLen)
{
	m_sendUncompressedBytes += sourceLen;
	gs_sendTotalUncompressedBytes += sourceLen;

	int result = 0;
	FATAL(!m_compressor, ("No compressor is available"));
	result = m_compressor->compress(sourceData, sourceLen, destData, sourceLen);
	if(result < 0 || result > sourceLen)
	{
		memcpy(destData, sourceData, sourceLen);
		destData[sourceLen] = 0;
		result = sourceLen + 1;
	}
	else
	{
		destData[result] = 1;
		result++;
	}

	if(result > 0)
	{
		m_sendCompressedBytes += result;
		gs_sendTotalCompressedBytes += result;
	}
	else
	{
		m_sendCompressedBytes += sourceLen;
		gs_sendTotalCompressedBytes += sourceLen;
	}

	return result;
}

// ----------------------------------------------------------------------

int ManagerHandler::OnUserSuppliedDecrypt(UdpConnectionMT *, uchar *destData, const uchar *sourceData, int sourceLen)
{
	m_recvCompressedBytes += sourceLen;
	gs_recvTotalCompressedBytes += sourceLen;

	int result = -1;
	FATAL(!m_compressor, ("No compressor is available"));
	if(sourceData[sourceLen - 1] == 1)
	{
		static const int bufferSize = ConfigSharedNetwork::getMaxRawPacketSize();
		result = m_compressor->expand(sourceData, sourceLen - 1, destData, bufferSize);
		if (result < 0)
		{
			WARNING_STRICT_FATAL(result < 0, ("Failed to decompress a compressed buffer"));
			return -1;
		}

		if(result > 0)
		{
			m_recvUncompressedBytes += result;
			gs_recvTotalUncompressedBytes += result;
		}
		else
		{
			m_recvUncompressedBytes += sourceLen;
			gs_recvTotalUncompressedBytes += sourceLen;
		}
	}
	else if(sourceData[sourceLen - 1] == 0)
	{
		memcpy(destData, sourceData, sourceLen - 1);
		result = sourceLen - 1;
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("invalid compression bit in buffer"));
		return -1;
	}

	return result;
}

// ======================================================================

