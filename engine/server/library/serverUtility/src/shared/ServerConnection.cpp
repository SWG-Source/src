// ======================================================================
//
// ServerConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ServerConnection.h"

#include "Archive/ByteStream.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/GameGameServerMessages.h"
#include "serverNetworkMessages/ReloadDatatableMessage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "SystemAssignedProcessId.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <algorithm>

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

namespace ServerConnectionNamespace
{
	std::vector<unsigned long int> s_forwardableMessages;
}
using namespace ServerConnectionNamespace;

// ======================================================================

void ServerConnection::install() // static
{
	s_forwardableMessages.push_back(constcrc("AddGameServer"));                    // startup
	s_forwardableMessages.push_back(constcrc("DeltasMessage"));                    // deltas
	s_forwardableMessages.push_back(constcrc("SynchronizeScriptVarDeltasMessage"));// deltas
	s_forwardableMessages.push_back(constcrc("CreateObjectByCrcMessage"));         // baselines
	s_forwardableMessages.push_back(constcrc("UpdateObjectPositionMessage"));      // baselines
	s_forwardableMessages.push_back(constcrc("BaselinesMessage"));                 // baselines
	s_forwardableMessages.push_back(constcrc("SynchronizeScriptVarsMessage"));     // baselines
	s_forwardableMessages.push_back(constcrc("setDecayTime"));                     // baselines
	s_forwardableMessages.push_back(constcrc("AiCreatureStateMessage"));           // baselines
	s_forwardableMessages.push_back(constcrc("AiMovementMessage"));                // baselines
	s_forwardableMessages.push_back(constcrc("EndBaselinesMessage"));              // baselines
	s_forwardableMessages.push_back(constcrc("SlowDownEffectMessage"));            // baselines
	s_forwardableMessages.push_back(constcrc("AuthTransferClientMessage"));        // auth transfer
	s_forwardableMessages.push_back(constcrc("SetAuthoritativeMessage"));          // auth transfer
	s_forwardableMessages.push_back(constcrc("CreateSyncUiMessage"));              // auth transfer
	s_forwardableMessages.push_back(constcrc("PageChangeAuthority"));              // auth transfer
	s_forwardableMessages.push_back(constcrc("AuthTransferConfirmMessage"));       // auth transfer
	s_forwardableMessages.push_back(constcrc("PlayedTimeAccumMessage"));           // auth transfer
	s_forwardableMessages.push_back(constcrc("UnloadObjectMessage"));              // container transfer
	s_forwardableMessages.push_back(constcrc("UpdateContainmentMessage"));         // container transfer
	s_forwardableMessages.push_back(constcrc("ObjControllerMessage"));             // controller messages
	s_forwardableMessages.push_back(constcrc("TeleportMessage"));                  // teleport
	s_forwardableMessages.push_back(constcrc("PlayerSanityCheck"));                // player sanity checker
	s_forwardableMessages.push_back(constcrc("PlayerSanityCheckProxy"));           // player sanity checker
	s_forwardableMessages.push_back(constcrc("PlayerSanityCheckSuccess"));         // player sanity checker
	s_forwardableMessages.push_back(constcrc("PlayerSanityCheckProxyFail"));       // player sanity checker
	s_forwardableMessages.push_back(constcrc("UnloadPersistedCharacter"));         // player sanity checker
	s_forwardableMessages.push_back(constcrc("AddResourceTypeMessage"));           // universe data
	s_forwardableMessages.push_back(constcrc("AddImportedResourceType"));          // universe data
	s_forwardableMessages.push_back(constcrc("UniverseCompleteMessage"));          // universe data
	s_forwardableMessages.push_back(constcrc("SetTheaterMessage"));                // universe data
	s_forwardableMessages.push_back(constcrc("ClearTheaterMessage"));              // universe data
	s_forwardableMessages.push_back(constcrc("ManualDepleteResourceMessage"));     // universe data
	s_forwardableMessages.push_back(constcrc("AddAttribModName"));                 // universe data
	s_forwardableMessages.push_back(constcrc("AddAttribModNamesList"));            // universe data
	s_forwardableMessages.push_back(constcrc("CharacterNamesMessage"));            // universe data
	s_forwardableMessages.push_back(constcrc("MessageToMessage"));                 // messageTos
	s_forwardableMessages.push_back(constcrc("WhoHasMessage"));                    // messageTos
	s_forwardableMessages.push_back(constcrc("ObjectNotOnServerMessage"));         // messageTos
	s_forwardableMessages.push_back(constcrc("ObjectOnServerMessage"));            // messageTos
	s_forwardableMessages.push_back(constcrc("EnableNewJediTrackingMessage"));     // console commands
	s_forwardableMessages.push_back(constcrc("EnablePlayerSanityCheckerMessage")); // console commands
	s_forwardableMessages.push_back(constcrc("ReloadAdminTableMessage"));          // console commands
	s_forwardableMessages.push_back(constcrc("ReloadCommandTableMessage"));        // console commands
	s_forwardableMessages.push_back(Crc::calculate(ReloadDatatableMessage::ms_messageName)); // console commands
	s_forwardableMessages.push_back(constcrc("ReloadScriptMessage"));              // console commands
	s_forwardableMessages.push_back(constcrc("ReloadTemplateMessage"));            // console commands
	s_forwardableMessages.push_back(constcrc("SetOverrideAccountAgeMessage"));     // console commands
	s_forwardableMessages.push_back(constcrc("ClaimRewardsReplyMessage"));         // veteran reward system
	s_forwardableMessages.push_back(constcrc("UnloadProxyMessage"));               // object load
	s_forwardableMessages.push_back(constcrc("CSRSLReq"));                         // city (ClusterStartupResidenceStructureListRequest)
	s_forwardableMessages.push_back(constcrc("CSRSLRsp"));                         // city (ClusterStartupResidenceStructureListResponse)

	std::sort(s_forwardableMessages.begin(), s_forwardableMessages.end());
}

// ----------------------------------------------------------------------

const unsigned long ServerConnection::makeProcessId() // static
{
	static unsigned long pid = 0;
	return ++pid;
}

// ----------------------------------------------------------------------

bool ServerConnection::isMessageForwardable(unsigned long int type)  // static
{
	return std::binary_search(s_forwardableMessages.begin(), s_forwardableMessages.end(), type);
}

// ----------------------------------------------------------------------

ServerConnection::MessageConnectionCallback::MessageConnectionCallback(const char * const messageName) :
MessageDispatch::MessageBase(messageName)
{
}

// ----------------------------------------------------------------------

ServerConnection::MessageConnectionCallback::~MessageConnectionCallback()
{
}

// ----------------------------------------------------------------------

ServerConnection::MessageConnectionOverflowing::MessageConnectionOverflowing(const unsigned int newBytesPending) :
MessageDispatch::MessageBase("ConnectionOverflowing"),
bytesPending(newBytesPending)
{
}

// ----------------------------------------------------------------------

ServerConnection::MessageConnectionOverflowing::~MessageConnectionOverflowing()
{
}

// ----------------------------------------------------------------------

const unsigned int ServerConnection::MessageConnectionOverflowing::getBytesPending() const
{
	return bytesPending;
}

// ----------------------------------------------------------------------

ServerConnection::ServerConnection(const std::string & a, const unsigned short p, const NetworkSetupData & setup) :
Connection(a, p, setup),
MessageDispatch::Emitter(),
processId(0),
osProcessId(0)
{
	char desc[1024] = {"\0"};
	snprintf(desc, sizeof(desc), "%s:%d", Os::getProgramName(), Os::getProcessId());
	GenericValueTypeMessage<std::string> d("DescribeConnection", desc);
	send(d, true);
	SystemAssignedProcessId id(Os::getProcessId());
	send(id, true);
}

// ----------------------------------------------------------------------

ServerConnection::ServerConnection(UdpConnectionMT * u, TcpClient * t) :
Connection(u, t),
MessageDispatch::Emitter(),
processId(0),
osProcessId(0)
{
	char desc[1024] = {"\0"};
	snprintf(desc, sizeof(desc), "%s:%d", Os::getProgramName(), Os::getProcessId());
	GenericValueTypeMessage<std::string> d("DescribeConnection", desc);
	send(d, true);	
	SystemAssignedProcessId id(Os::getProcessId());
	send(id, true);
}

// ----------------------------------------------------------------------

ServerConnection::~ServerConnection()
{
}

// ----------------------------------------------------------------------

void ServerConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("ConnectionClosed");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void ServerConnection::onConnectionOpened()
{
	processId = makeProcessId();
	static MessageConnectionCallback m("ConnectionOpened");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void ServerConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	// not static -- bytesPending is different each time this is invoked
	MessageConnectionOverflowing m(bytesPending);
	emitMessage(m);
}

// ----------------------------------------------------------------------

void ServerConnection::onReceive(const Archive::ByteStream & message)
{
	try
	{
		Archive::ReadIterator r = message.begin();
		GameNetworkMessage m(r);
		r = message.begin();
		if(m.isType("CentralGameServerSetProcessId"))
		{
			CentralGameServerSetProcessId p(r);
			r = message.begin();
			processId = p.getProcessId();
		}
		else if(m.isType("GameGameServerConnect"))
		{
			GameGameServerConnect p(r);
			r = message.begin();
			processId = p.getProcessId();
		}
		else if(m.isType("SystemAssignedProcessId"))
		{
			SystemAssignedProcessId id(r);
			osProcessId = id.getId();
		}
		else if(m.isType("DescribeConnection"))
		{
			GenericValueTypeMessage<std::string> d(r);
			describeConnection(d.getValue());
		}

		emitMessage(m);
	}
	catch(const Archive::ReadException & readException)
	{
		WARNING(true, ("Archive::ReadException : %s\n\tDisconnecting from remote server...", readException.what()));
		disconnect();
		
	}
}

// ----------------------------------------------------------------------

void ServerConnection::reportReceive(const Archive::ByteStream & bs)
{
	Connection::reportReceive(bs);
	static Archive::ReadIterator ri;
	ri = bs.begin();
	GameNetworkMessage whatIsIt(ri);
	NetworkHandler::reportMessage("recv." + whatIsIt.getCmdName(), bs.getSize());
}

// ----------------------------------------------------------------------

void ServerConnection::reportSend(const Archive::ByteStream & bs)
{
	Connection::reportReceive(bs);
	static Archive::ReadIterator ri;
	ri = bs.begin();
	GameNetworkMessage whatIsIt(ri);
	NetworkHandler::reportMessage("send." + whatIsIt.getCmdName(), bs.getSize());

	m_pendingPackets.push_back(std::make_pair(whatIsIt.getCmdName(), bs.getSize()));
}

// ----------------------------------------------------------------------

void ServerConnection::send(const GameNetworkMessage & message, const bool reliable)
{
	static Archive::ByteStream a;
	a.clear();
	message.pack(a);
	Connection::send(a, reliable);
}

// ----------------------------------------------------------------------

void ServerConnection::onConnectionStalled(const unsigned long stallTimeMs)
{
	LOG("Network", ("Connection to %s, pid %d stalled with %d bytes pending for %d milliseconds", getRemoteAddress().c_str(), getOsProcessId(), getPendingBytes(), stallTimeMs));
}

// ----------------------------------------------------------------------

void ServerConnection::setProcessId(const uint32 newProcessId)
{
	processId = newProcessId;
}

// ======================================================================
