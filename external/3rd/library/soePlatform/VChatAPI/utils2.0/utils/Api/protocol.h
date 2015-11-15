#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#ifdef _USE_TCP

#include "TcpLibrary/TcpConnection.h"
#include "TcpLibrary/TcpManager.h"
#define PlatConnectionHandler	TcpConnectionHandler
#define PlatConnection			TcpConnection
#define PlatManager				TcpManager
#define PlatManagerHandler		TcpManagerHandler
#define PlatParams				TcpManager::TcpParams

#else
#include "UdpLibrary/UdpLibrary.hpp"
#define PlatConnectionHandler	UdpConnectionHandler
#define PlatConnection			UdpConnection
#define PlatManager				UdpManager
#define PlatParams				UdpManager::Params

#endif

#endif