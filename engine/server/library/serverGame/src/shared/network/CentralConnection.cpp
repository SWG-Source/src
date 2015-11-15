// CentralServerConnection.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "Archive/ByteStream.h"
#include "serverGame/GameServer.h"
#include "serverGame/CentralConnection.h"
#include "serverGame/CentralCommandParserGame.h"
#include "serverGame/ConsoleCommandParserDefault.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverNetworkMessages/CharacterTransferStatusMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferReplyCharacterList.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/GameServerCSResponseMessage.h"
#include "serverNetworkMessages/VerifyAndLockName.h"
#include "serverUtility/FreeCtsDataTable.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

CentralServerConnection::CentralServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData()),
m_centralCommandParser(new ConsoleCommandParserDefault)
{
	m_centralCommandParser->addSubCommand(new CentralCommandParserGame);
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
	delete m_centralCommandParser;
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
	ServerConnection::onConnectionOpened();

	static MessageConnectionCallback m("CentralConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage msg(ri);
	ri = message.begin();

	if(msg.isType("ConGenericMessage"))
	{
		// dispatch to CentralCommandParser
		ConGenericMessage con(ri);
		Unicode::String wideResult;
		m_centralCommandParser->parse(NetworkId(static_cast<NetworkId::NetworkIdType>(con.getMsgId())), Unicode::narrowToWide(con.getMsg()), wideResult);

		ConGenericMessage response(Unicode::wideToNarrow(wideResult), con.getMsgId());
		send(response, true);
	}
	else if(msg.isType("CharacterTransferStatusMessage"))
	{
		CharacterTransferStatusMessage c(ri);
		CreatureObject * character = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(c.getToCharacterId()));
		if(character)
		{
			if(character->getClient())
			{
				character->receiveCharacterTransferStatusMessage(c.getStatusMessage());
			}
		}
	}
	else if(msg.isType("TransferRequestNameValidation"))
	{
		const GenericValueTypeMessage<TransferCharacterData> requestNameValidation(ri);
		LOG("CustomerService", ("CharacterTransfer: Received TransferRequestNameValidation from CentralServer : %s", requestNameValidation.getValue().toString().c_str()));

		std::string name = requestNameValidation.getValue().getDestinationCharacterName();
		if(name.empty())
		{
			name = requestNameValidation.getValue().getSourceCharacterName();
		}

		// call the same name validation code that gets called for normal character creation
		std::string templateName = requestNameValidation.getValue().getObjectTemplateName();
		if (templateName.empty())
		{
			uint32 const templateCrc = requestNameValidation.getValue().getObjectTemplateCrc();
			if (templateCrc != 0)
			{
				templateName = ObjectTemplateList::lookUp(templateCrc).getString();
			}
		}

		VerifyAndLockNameRequest const verifyRequest(requestNameValidation.getValue().getDestinationStationId(), NetworkId::cms_invalid, templateName, Unicode::narrowToWide(name), 0xFFFFFFFF);
		StringId const verifyResult = GameServer::getInstance().handleVerifyAndLockNameRequest(verifyRequest, false, false);

		TransferCharacterData replyData(requestNameValidation.getValue());
		replyData.setIsValidName(verifyResult == NameErrors::nameApproved);
		GenericValueTypeMessage<std::pair<std::string, TransferCharacterData> > reply("TransferReplyNameValidation", std::make_pair(std::string("@") + verifyResult.getCanonicalRepresentation(), replyData));
		send(reply, true);
	}
	else if(msg.isType("TransferReplyCharacterList"))
	{
		const TransferReplyCharacterList reply(ri);
		const AvatarList & oldAl = reply.getAvatarList();

		// remove any characters from the list that would qualify for free CTS; characters that
		// qualify for free CTS should/must use the in-game free CTS to transfer the character;
		AvatarList newAl;
		std::string const & clusterName = GameServer::getInstance().getClusterName();
		for (AvatarList::const_iterator iter = oldAl.begin(); iter != oldAl.end(); ++iter)
		{
			if (!FreeCtsDataTable::getFreeCtsInfoForCharacter(static_cast<time_t>(NameManager::getInstance().getPlayerCreateTime(iter->m_networkId)), clusterName, false))
				newAl.push_back(*iter);
			else
				LOG("CustomerService", ("CharacterTransfer: removing (%s, %s, %s) from paid CTS character list for station id %u because the character qualifies for free CTS", clusterName.c_str(), Unicode::wideToNarrow(iter->m_name).c_str(), iter->m_networkId.getValueString().c_str(), reply.getStationId()));
		}

		// send updated character list back to CentralServer
		const TransferReplyCharacterList updatedReply(reply.getTrack(), reply.getStationId(), newAl);
		send(updatedReply, true);
	}
	else if(msg.isType("CtsCompletedForcharacter"))
	{
		const GenericValueTypeMessage<std::pair<std::string, NetworkId> > msg(ri);
		MessageToQueue::getInstance().sendMessageToJava(msg.getValue().second, "ctsCompletedForCharacter", std::vector<int8>(), 0, false);
	}
	else if( msg.isType( "GameServerCSRequest" ) )
	{
		GameServerCSRequestMessage request( ri );
		GameServer::getInstance().handleCSRequest( request );
	}
	else if( msg.isType( "CSFindAuthObject" ) )
	{
		// see if we have the object.
		GenericValueTypeMessage< std::pair<NetworkId, unsigned int> > msg( ri );
		ServerObject * object = safe_cast<ServerObject *>( NetworkIdManager::getObjectById( msg.getValue().first ) );
		// return an appropriate response.
		if( object && object->isAuthoritative() )
		{
			GenericValueTypeMessage< std::pair< unsigned int, bool > > rmsg( "CSFindAuthObjectReply", std::make_pair( msg.getValue().second, true ) );
			GameServer::getInstance().sendToCentralServer( rmsg );
		}
		else
		{
			GenericValueTypeMessage< std::pair< unsigned int, bool > > rmsg( "CSFindAuthObjectReply", std::make_pair( msg.getValue().second, false ) );
			GameServer::getInstance().sendToCentralServer( rmsg );
		}
	}
	else
	{
		ServerConnection::onReceive(message);
	}
}

//-----------------------------------------------------------------------
