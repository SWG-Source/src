// CentralServerConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "Archive/ByteStream.h"
#include "CentralServerConnection.h"
#include "ChatServer.h"
#include "ChatInterface.h"
#include "ConfigChatServer.h"
#include "serverNetworkMessages/EnumerateServers.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

CentralServerConnection::CentralServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
	ChatServer::fileLog(true, "CentralServerConnection", "Connection created...listening on (%s:%d)", a.c_str(), static_cast<int>(p));
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionClosed()
{
	ChatServer::fileLog(true, "CentralServerConnection", "onConnectionClosed()");

	ChatServer::onCentralServerConnectionClosed();
	ChatServer::quit();
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{
	ChatServer::fileLog(true, "CentralServerConnection", "onConnectionOpened()");
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);

	ri = message.begin();

	if(m.isType("EnumerateServers"))
	{
		static MessageDispatch::Transceiver<const EnumerateServers &> emitter;

		EnumerateServers e(ri);
		emitter.emitMessage(e);
	}
	else if (m.isType("CustomerServiceServerChatServerServiceAddress"))
	{
		GenericValueTypeMessage<std::pair<std::string, unsigned short> > msg(ri);

		ChatServer::instance().connectToCustomerServiceServer(msg.getValue().first, msg.getValue().second);
	}
	else if(m.isType("RequestChatTransferAvatar"))
	{
		GenericValueTypeMessage<TransferCharacterData> request(ri);
		LOG("CustomerService", ("CharacterTransfer: Received RequestChatTransferAvatar from CentralServer. Forwarding request to ChatAPI. %s", request.getValue().toString().c_str()));
		ChatServer::requestTransferAvatar(request.getValue());
	}
	else if (m.isType("RenameCharacterMessageEx"))
	{
		RenameCharacterMessageEx const msg(ri);

		// if the player requested the rename, also rename the chat
		// avatar, so mail, friends list, and ignore list will migrate
		if ((msg.getRenameCharacterMessageSource() == RenameCharacterMessageEx::RCMS_player_request) && !msg.getLastNameChangeOnly() && !ConfigFile::getKeyBool("CharacterRename", "disableRenameChatAvatar", false))
		{
			std::string const oldName(Unicode::toLower(Unicode::wideToNarrow(msg.getOldName())));
			std::string const oldNameNormalized(oldName, 0, oldName.find(' '));
			ChatAvatarId const oldAvatar("SWG", ConfigChatServer::getClusterName(), oldNameNormalized);

			std::string const newName(Unicode::toLower(Unicode::wideToNarrow(msg.getNewName())));
			std::string const newNameNormalized(newName, 0, newName.find(' '));
			ChatAvatarId const newAvatar("SWG", ConfigChatServer::getClusterName(), newNameNormalized);

			IGNORE_RETURN(ChatServer::getChatInterface()->RequestTransferAvatar(msg.getStationId(), oldAvatar.getName(), oldAvatar.getAPIAddress(), msg.getStationId(), newAvatar.getName(), newAvatar.getAPIAddress(), true, nullptr));
		}
	}
	else if (m.isType("ChatDestroyAvatar"))
	{
		GenericValueTypeMessage<std::string> const msg(ri);
		ChatServer::getChatInterface()->DestroyAvatar(msg.getValue());
	}
	else if(m.isType("AllCluserGlobalChannel"))
	{
		typedef std::pair<std::pair<std::string,std::string>, bool> PayloadType;
		GenericValueTypeMessage<PayloadType> msg(ri);

		PayloadType const & payload = msg.getValue();
		std::string const & channelName = payload.first.first;
		std::string const & messageText = payload.first.second;
		bool const & isRemove = payload.second;

		LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer got AllCluserGlobalChannel on CentralServerConnection chan(%s) text(%s) remove(%d)",
			channelName.c_str(), messageText.c_str(), (isRemove?1:0)));
		ChatServer::requestBroadcastChannelMessage(channelName, messageText, isRemove);
	}
}

//-----------------------------------------------------------------------

