// ======================================================================
//
// ManagerHandler.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _ManagerHandler_H_
#define _ManagerHandler_H_

// ======================================================================

class UdpConnectionMT;
class ZlibCompressor;

// ======================================================================

#include "sharedNetwork/UdpHandlerMT.h"

// ======================================================================

class ManagerHandler: public UdpManagerHandlerMT
{
public:
	ManagerHandler(NetworkHandler *owner);

	static int   getRecvTotalCompressedByteCount    ();
	static int   getRecvTotalUncompressedByteCount  ();
	static int   getSendTotalCompressedByteCount    ();
	static int   getSendTotalUncompressedByteCount  ();
	static float getTotalCompressionRatio           ();

	void         setOwner                           (NetworkHandler *owner);
	int          getRecvCompressedByteCount         () const;
	int          getRecvUncompressedByteCount       () const;
	int          getSendCompressedByteCount         () const;
	int          getSendUncompressedByteCount       () const;
	float        getCompressionRatio                () const;
	
	virtual void OnConnectRequest(UdpConnectionMT *con);
	virtual int  OnUserSuppliedEncrypt(UdpConnectionMT *con, uchar *destData, const uchar *sourceData, int sourceLen);
	virtual int  OnUserSuppliedDecrypt(UdpConnectionMT *con, uchar *destData, const uchar *sourceData, int sourceLen);

private:
	ManagerHandler(ManagerHandler const &);
	ManagerHandler &operator=(ManagerHandler const &);

	virtual ~ManagerHandler();

private:
	NetworkHandler *   m_owner;
	ZlibCompressor *   m_compressor;
	int                m_recvCompressedBytes;
	int                m_recvUncompressedBytes;
	int                m_sendCompressedBytes;
	int                m_sendUncompressedBytes;
};

//-----------------------------------------------------------------------

#endif // _ManagerHandler_H_

