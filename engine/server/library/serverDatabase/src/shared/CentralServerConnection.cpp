// ======================================================================
//
// CentralServerConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "Archive/Archive.h"
#include "ConsoleManager.h"
#include "serverDatabase/CentralServerConnection.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/Persister.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

// ======================================================================

CentralServerConnection::CentralServerConnection(const std::string & a, const unsigned short p) :
		ServerConnection(a, p, NetworkSetupData()),
		MessageDispatch::Receiver()
{
}

// ----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
	DEBUG_REPORT_LOG(true, ("Deleting central server connection\n"));
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("CentralConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{
	static MessageConnectionCallback m("CentralConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();
	
	if(msg.isType("TransferRenameCharacter"))
	{
		const GenericValueTypeMessage<TransferCharacterData> request(ri);
		const TransferCharacterData & requestData = request.getValue();
		LOG("CustomerService", ("CharacterTransfer: Received TransferRenameCharacter, starting game database name change. %s", requestData.toString().c_str()));
		Persister::getInstance().renameCharacter(0, static_cast<int8>(RenameCharacterMessageEx::RCMS_cts_rename), request.getValue().getSourceStationId(), request.getValue().getCharacterId(), Unicode::narrowToWide(request.getValue().getDestinationCharacterName()), Unicode::narrowToWide(request.getValue().getSourceCharacterName()), false, NetworkId::cms_invalid, &requestData);
	}
	else if(msg.isType("ConGenericMessage"))
	{
		const ConGenericMessage cm(ri);
		std::string result;
		ConsoleManager::processString(cm.getMsg(), cm.getMsgId(), result);
		const ConGenericMessage response(result, cm.getMsgId());
		send(response, true);
	}
	else if(msg.isType("TransferAccountRequestCentralDatabase"))
	{
		const GenericValueTypeMessage<TransferAccountData> request(ri);
		const TransferAccountData & requestData = request.getValue();
		LOG("CustomerService", ("CharacterTransfer: Received TransferAccountRequestCentralDatabase, starting game database change from station ID %d to station ID %d", request.getValue().getSourceStationId(), request.getValue().getDestinationStationId()));
		Persister::getInstance().changeStationId(&requestData);
	}
	else
	{
		ServerConnection::onReceive(message);
	}
}

//-----------------------------------------------------------------------

void CentralServerConnection::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
	UNREF(message);
}

// ======================================================================
