// ======================================================================
//
// UdpManagerHandlerMT.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _UdpManagerHandlerMT_H_
#define _UdpManagerHandlerMT_H_

// ======================================================================

class UdpConnectionMT;
class UdpManagerHandlerInternal;

// ======================================================================

class UdpManagerHandlerMT
{
public:
	UdpManagerHandlerMT();

	void AddRef();
	void Release();

	virtual void OnConnectRequest(UdpConnectionMT *con) = 0;
	virtual int OnUserSuppliedEncrypt(UdpConnectionMT *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);
	virtual int OnUserSuppliedDecrypt(UdpConnectionMT *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);
	virtual int OnUserSuppliedEncrypt2(UdpConnectionMT *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);
	virtual int OnUserSuppliedDecrypt2(UdpConnectionMT *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);

	UdpManagerHandler *getManagerHandler();
	void update();

private:
	UdpManagerHandlerMT(UdpManagerHandlerMT const &);
	UdpManagerHandlerMT &operator=(UdpManagerHandlerMT const &);

protected:
	virtual ~UdpManagerHandlerMT();

	friend class UdpManagerHandlerInternal;
	void queueConnectRequest(UdpConnection *con);

private:
	int m_refCount;
	UdpManagerHandlerInternal *m_managerHandler;
};

// ======================================================================

#endif // _UdpManagerHandlerMT_H_

