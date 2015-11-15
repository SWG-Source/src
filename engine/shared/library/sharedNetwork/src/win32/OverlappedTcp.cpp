// OverlappedTcp.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "OverlappedTcp.h"
#include <vector>
#include "TcpClient.h"
#include "TcpServer.h"

//---------------------------------------------------------------------

OverlappedTcp::~OverlappedTcp()
{
	delete[] m_acceptData;
	delete[] m_recvBuf.buf;
}

//---------------------------------------------------------------------

struct OverlappedFreeList
{
	~OverlappedFreeList();
	std::vector<OverlappedTcp *>  allOverlapped;
	std::vector<OverlappedTcp *>  freeOverlapped;
};

//---------------------------------------------------------------------

OverlappedFreeList::~OverlappedFreeList()
{
	std::vector<OverlappedTcp *>::const_iterator i;
	for(i = allOverlapped.begin(); i != allOverlapped.end(); ++i)
	{
		OverlappedTcp * t = (*i);
		delete t;
	}
}

//---------------------------------------------------------------------

OverlappedFreeList overlappedFreeList;

//---------------------------------------------------------------------

OverlappedTcp * getFreeOverlapped()
{
	OverlappedTcp * result = NULL;

	if(! overlappedFreeList.freeOverlapped.empty())
	{
		result = overlappedFreeList.freeOverlapped.back();
		overlappedFreeList.freeOverlapped.pop_back();
	}
	else
	{
		result = new OverlappedTcp;
		result->m_bytes = 0;
		memset(&result->m_overlapped, 0, sizeof(OVERLAPPED));
		result->m_recvBuf.buf = new char[1024];
		result->m_recvBuf.len = 1024;
		result->m_tcpClient = 0;
		result->m_tcpServer = 0;
		result->m_acceptData = 0;
		overlappedFreeList.allOverlapped.push_back(result);
	}
	return result;
}

//---------------------------------------------------------------------

void releaseOverlapped(OverlappedTcp * o)
{
	o->m_bytes = 0;
	o->m_tcpClient = 0;
	o->m_tcpServer = 0;
	o->m_acceptData = 0;
	memset(&o->m_overlapped, 0, sizeof(OVERLAPPED));
	overlappedFreeList.freeOverlapped.push_back(o);
}

//-----------------------------------------------------------------------

