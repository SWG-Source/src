// CentralServerConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTransferServer.h"
#include "CentralServerConnection.h"
#include "ConsoleManager.h"
#include "TransferServer.h"
#include "serverNetworkMessages/CharacterTransferStatusMessage.h"
#include "serverNetworkMessages/DownloadCharacterMessage.h"
#include "serverNetworkMessages/ToggleAvatarLoginStatus.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferReplyCharacterList.h"
#include "serverNetworkMessages/TransferReplyMoveValidation.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"
#include "sharedLog/Log.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "UnicodeUtils.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

namespace CentralServerConnectionNamespace
{
	std::map<std::string, CentralServerConnection *>  s_centralServerConnections;
}

using namespace CentralServerConnectionNamespace;


//-----------------------------------------------------------------------

CentralServerConnection::CentralServerConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t),
m_galaxyName("")
{
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
	std::map<std::string, CentralServerConnection *>::iterator f = s_centralServerConnections.find(m_galaxyName);
	if(f != s_centralServerConnections.end())
	{
		s_centralServerConnections.erase(f);
	}
}

//-----------------------------------------------------------------------

CentralServerConnection * CentralServerConnection::getCentralServerConnectionForGalaxy(const std::string & galaxyName)
{
	CentralServerConnection * result = 0;
	std::map<std::string, CentralServerConnection *>::iterator f = s_centralServerConnections.find(galaxyName);
	if(f != s_centralServerConnections.end())
	{
		result = f->second;
	}
	return result;
}

//-----------------------------------------------------------------------

const std::map<std::string, CentralServerConnection *> & CentralServerConnection::getGalaxies()
{
	return s_centralServerConnections;
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionClosed()
{
	LOG("CustomerService", ("CharacterTransfer: Connection with CentralServer running Galaxy %s has closed\n", m_galaxyName.c_str()));
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{
	LOG("CustomerService", ("CharacterTransfer: A connection with a CentralServer has opened\n"));
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();
	
	const uint32 messageType = msg.getType();
	
	switch(messageType) {
		case constcrc("CentralGalaxyName") :
		{
			const GenericValueTypeMessage<std::string> cgn(ri);
			setGalaxyName(cgn.getValue());
			LOG("CustomerService", ("CharacterTransfer: Received CentralGalaxyName for %s", cgn.getValue().c_str()));
			break;
		}
		case constcrc("UploadCharacterMessage") :
		{
			// receiving character data
			const UploadCharacterMessage ucm(ri);
			CharacterTransferData data;
			data.stationId = ucm.getStationId();
			data.fromGalaxy = m_galaxyName;
			data.packedData = ucm.getPackedCharacterData();
			bool result = TransferServer::uploadCharacterTransferData(data, ucm.getIsAdmin());
			if(result)
			{
				CharacterTransferStatusMessage uploadResult(ucm.getFromGameServerId(), ucm.getFromCharacterId(), "@base_player:upload_success");
				send(uploadResult, true);
			}
			else
			{
				CharacterTransferStatusMessage uploadResult(ucm.getFromGameServerId(), ucm.getFromCharacterId(), "@base_player:upload_fail");
				send(uploadResult, true);
			}
			break;
		}
		case constcrc("DownloadCharacterMessage") :
		{
			// retreive packed data from transfer server and
			// send it to the requesting central server
			const DownloadCharacterMessage dcm(ri);
			const CharacterTransferData * data = TransferServer::getCharacterTransferData(dcm.getStationId(), m_galaxyName, dcm.getIsAdmin());
			if(data)
			{
				const UploadCharacterMessage ucm(data->stationId, data->packedData, dcm.getGameServerId(), dcm.getToCharacterId(), true);
				send(ucm, true);
				LOG("CustomerService", ("CharacterTransfer: Character data for SUID %d downloaded\n", data->stationId));
			}
			else
			{
				CharacterTransferStatusMessage downloadResult(dcm.getGameServerId(), dcm.getToCharacterId(), "@base_player:download_fail");
				send(downloadResult, true);
			}
			break;

		}
		case constcrc("AuthorizeDownload") :
		{
			const GenericValueTypeMessage<unsigned int> auth(ri);
			TransferServer::authorizeDownload(auth.getValue());
			break;
		}
		case constcrc("UnauthorizeDownload") :
		{
			const GenericValueTypeMessage<unsigned int> auth(ri);
			TransferServer::unauthorizeDownload(auth.getValue());
			break;
		}
		case constcrc("TransferReplyCharacterList") :
		{
			const TransferReplyCharacterList reply(ri);
			
			// TESTING!
			LOG("CustomerService", ("CharacterTransfer: Received character list for station id %d\n", reply.getStationId()));
			const std::vector<AvatarRecord> & avatarList = reply.getAvatarList();
			std::vector<AvatarRecord>::const_iterator i;
			LOG("CustomerService", ("CharacterTransfer: Enumerating %d characters", avatarList.size()));
			for(i = avatarList.begin() ; i != avatarList.end(); ++i)
			{
				LOG("CustomerService", ("CharacterTransfer: m_name = \"%s\" m_networkId = %s\n", Unicode::wideToNarrow(i->m_name).c_str(), i->m_networkId.getValueString().c_str()));
			}
			
			// TESTING END
			TransferServer::replyCharacterList(reply);
			break;
		}
		case constcrc("TransferReplyMoveValidation") :
		{
			const TransferReplyMoveValidation reply(ri);
			if(reply.getResult() == TransferReplyMoveValidation::TRMVR_can_create_regular_character)
			{
				// save off the source character's template crc so it can be used in
				// subsequent TransferRequestMoveValidation queries, so the LoginServer
				// doesn't have to continually query the DB to fetch the same information
				// over and over again, because the character's template crc
				// (i.e. the character's species) should never, ever change
				TransferServer::cacheSourceCharacterTemplateCrc(reply.getSourceStationId(), reply.getSourceGalaxy(), reply.getSourceCharacter(), reply.getSourceCharacterTemplateId());

				TransferCharacterData requestData(reply.getTransferRequestSource());
				requestData.setTrack(reply.getTrack());
				requestData.setCustomerLocalizedLanguage(reply.getCustomerLocalizedLanguage());
				requestData.setSourceGalaxy(reply.getSourceGalaxy());
				requestData.setDestinationGalaxy(reply.getDestinationGalaxy());
				requestData.setSourceCharacterName(reply.getSourceCharacter());
				requestData.setObjectTemplateCrc(reply.getSourceCharacterTemplateId());
				requestData.setDestinationCharacterName(reply.getDestinationCharacter());
				requestData.setSourceStationId(reply.getSourceStationId());
				requestData.setDestinationStationId(reply.getDestinationStationId());
				const GenericValueTypeMessage<TransferCharacterData> requestNameValidation("TransferRequestNameValidation", requestData);
				CentralServerConnection * destinationConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(reply.getDestinationGalaxy());
				
				LOG("CustomerService", ("CharacterTransfer: Received TransferReplyMoveValidation for %s on %s to %s on %s from CentralServer. Sending TransferRequestNameValidation to destination CentralServer.", reply.getSourceCharacter().c_str(), reply.getSourceGalaxy().c_str(), reply.getDestinationCharacter().c_str(), reply.getDestinationGalaxy().c_str()));
				
				const GenericValueTypeMessage<std::pair<unsigned int, unsigned int> > kick("TransferRequestKickConnectedClients", std::make_pair(requestData.getSourceStationId(), requestData.getDestinationStationId()));
				if(destinationConnection)
				{
					destinationConnection->send(requestNameValidation, true);
					destinationConnection->send(kick, true);
				}

				CentralServerConnection * sourceConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(reply.getSourceGalaxy());
				if(sourceConnection)
				{
					sourceConnection->send(kick, true);
				}
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: TransferReplyMoveValidation failed for %s on %s to %s on %s from CentralServer because the account cannot create a new character on the destination server", reply.getSourceCharacter().c_str(), reply.getSourceGalaxy().c_str(), reply.getDestinationCharacter().c_str(), reply.getDestinationGalaxy().c_str()));
				TransferServer::failedToValidateTransfer(reply);
			}
			break;
		}
		case constcrc("TransferReplyNameValidation") :
		{
			const GenericValueTypeMessage<std::map<std::string, TransferCharacterData> > replyNameValidation(ri);
			auto i = replyNameValidation.getValue().begin();

			if (i == replyNameValidation.getValue().end()) {
				break;
			}

			if(!i->second.getIsMoveRequest())
			{
				LOG("CustomerService", ("CharacterTransfer: Received replyNameValidation for move validation request. (%s) %s", i->first.c_str(), i->second.toString().c_str()));
				TransferServer::replyValidateMove(i->second);
			}
			else
			{
				if(TransferServer::isRename(i->second))
				{
					LOG("CustomerService", ("CharacterTransfer: Received replyNameValidation for rename request, starting character rename protocol. (%s) %s", i->first.c_str(), i->second.toString().c_str()));
					const GenericValueTypeMessage<TransferCharacterData> renameCharacter("TransferRenameCharacter", i->second);
					CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(i->second.getSourceGalaxy());
					if(centralServerConnection)
					{
						centralServerConnection->send(renameCharacter, true);
					}
					else
					{
						TransferServer::transferCreateCharacterFailed(i->second);					}
				}
				else
				{
					DEBUG_FATAL(true, ("Received TransferReplyNameValidation for a move request that is not a rename!"));
				}
			}
			break;
		}
		case constcrc("TransferReplyCharacterDataFromLoginServer") :
		{
			const GenericValueTypeMessage<TransferCharacterData> reply(ri);
			LOG("CustomerService", ("CharacterTransfer: Got TransferReplyCharacterDataFromLoginServer, characterId=%s\n", reply.getValue().getCharacterId().getValueString().c_str()));
			
			// is this a transfer or a rename?
			if(TransferServer::isRename(reply.getValue()))
			{
				LOG("CustomerService", ("CharacterTransfer: Sending TransferRequestNameValidation for character rename request: %s", reply.getValue().toString().c_str()));
				// send a name validation request
				const GenericValueTypeMessage<TransferCharacterData> requestNameValidation("TransferRequestNameValidation", reply.getValue());
				CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(reply.getValue().getSourceGalaxy());
				if(centralServerConnection)
				{
					centralServerConnection->send(requestNameValidation, true);
				}
			}
			else
			{
				// this is a transfer
				TransferServer::getLoginLocationData(reply.getValue());
			}
			break;
		}
		case constcrc("TransferReplyLoginLocationData") :
		{
			const GenericValueTypeMessage<TransferCharacterData> reply(ri);
			LOG("CustomerService", ("CharacterTransfer: Got TransferReplyLoginLocationData from CentralServer. TransferCharacterData=%s\n", reply.getValue().toString().c_str()));
			
			// send character to ConnectionServer for login to a game server
			const GenericValueTypeMessage<TransferCharacterData> login("TransferLoginCharacterToSourceServer", reply.getValue());
			LOG("CustomerService", ("CharacterTransfer: Sending TransferLoginCharacterToSourceServer to CentralServer : %s", login.getValue().toString().c_str()));
			send(login, true);
			break;
		}
		case constcrc("TransferReceiveDataFromGameServer") :
		{
			const GenericValueTypeMessage<TransferCharacterData> transferReply(ri);
			LOG("CustomerService", ("CharacterTransfer: Got TransferReceiveDataFromGameServer: %s", transferReply.getValue().toString().c_str()));
			REPORT_LOG(true, ("Got TransferReceiveDataFromGameServer: %s\n", transferReply.getValue().toString().c_str()));
			
			// find a central server connection for the target cluster
			CentralServerConnection * destinationCentralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(transferReply.getValue().getDestinationGalaxy());
			if(destinationCentralConnection)
			{
				// send character to ConnectionServer for creation on the destination server
				GenericValueTypeMessage<TransferCharacterData> login("TransferLoginCharacterToDestinationServer", transferReply.getValue());
				destinationCentralConnection->send(login, true);
				LOG("CustomerService", ("CharacterTransfer: Sending TransferLoginCharacterToDestinationServer to CentralServer (%s) for (%s)", transferReply.getValue().getDestinationGalaxy().c_str(), login.getValue().toString().c_str()));
			}
			else
			{
				//@todo: report error to CTS API
				LOG("CustomerService", ("CharacterTransfer: Failed to get a CentralServer connection for %s", transferReply.getValue().getDestinationGalaxy().c_str()));
				TransferServer::transferCreateCharacterFailed(transferReply.getValue());
			}
			break;
		}
		case constcrc("ApplyTransferDataSuccess") :
		{
			const GenericValueTypeMessage<TransferCharacterData> success(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferDataSuccess from target galaxy. %s", success.getValue().toString().c_str()));
			
			CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(success.getValue().getSourceGalaxy());
			if(centralServerConnection)
			{
				LOG("CustomerService", ("CharacterTransfer: Sending ToggleAvatarLoginStatus request to source central server"));
				const ToggleAvatarLoginStatus toggleLoginStatus(success.getValue().getSourceGalaxy(), success.getValue().getSourceStationId(), success.getValue().getCharacterId(), false);
				centralServerConnection->send(toggleLoginStatus, true);
				TransferServer::replyMoveSuccess(success.getValue());
				LOG("CustomerService", ("CharacterTransfer: TRANSFER SUCCESSFUL for transactionId=%u", success.getValue().getTransactionId()));
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: Transfer failed on target galaxy : %s", success.getValue().toString().c_str()));
				TransferServer::transferCreateCharacterFailed(success.getValue());
			}
			break;
		}
		case constcrc("ApplyTransferDataFail") :
		{
			const GenericValueTypeMessage<TransferCharacterData> fail(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferDataFail from target galaxy. %s", fail.getValue().toString().c_str()));
			
			CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(fail.getValue().getDestinationGalaxy());
			if(centralServerConnection)
			{
				LOG("CustomerService", ("CharacterTransfer: failed to apply transfer data. Deleting character on destination server. %s", fail.getValue().toString().c_str()));
				const GenericValueTypeMessage<TransferCharacterData> deleteCharacter("DeleteFailedTransfer", fail.getValue());
				centralServerConnection->send(deleteCharacter, true);
				TransferServer::failedToApplyTransferData(fail.getValue());
			}
			break;
		}
		case constcrc("TransferCreateCharacterFailed") :
		{
			const GenericValueTypeMessage<TransferCharacterData> fail(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferCreateCharacterFailed, sending response to CTS API. %s", fail.getValue().toString().c_str()));
			TransferServer::transferCreateCharacterFailed(fail.getValue());
			break;
		}
		case constcrc("TransferRenameCharacterReply") :
		{
			const GenericValueTypeMessage<TransferCharacterData> reply(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferRenameCharacterReply from Central Server : %s", reply.getValue().toString().c_str()));
			if(reply.getValue().getIsValidName())
			{
				TransferServer::replyMoveSuccess(reply.getValue());
			}
			else
			{
				TransferServer::transferCreateCharacterFailed(reply.getValue());
			}
			break;
		}
		case constcrc("TransferAccountReplySuccessTransferServer") :
		{
			const GenericValueTypeMessage<TransferAccountData> reply(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountReplySuccessTransferServer from station Id %d to station Id %d", reply.getValue().getSourceStationId(), reply.getValue().getDestinationStationId()));
			TransferServer::replyTransferAccountSuccess(reply.getValue());
			break;
		}
		case constcrc("TransferAccountFailedToUpdateGameDatabase") :
		{
			const GenericValueTypeMessage<TransferAccountData> reply(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountFailedToUpdateGameDatabase for transfer from station Id %d to station Id %d", reply.getValue().getSourceStationId(), reply.getValue().getDestinationStationId()));
			TransferServer::failedToTransferAccountNoCentralConnection(reply.getValue());
			break;
		}
		case constcrc("TransferAccountFailedDestinationNotEmpty") :
		{
			const GenericValueTypeMessage<TransferAccountData> reply(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountFailedDestinationNotEmpty for transfer from station Id %d to station Id %d", reply.getValue().getSourceStationId(), reply.getValue().getDestinationStationId()));
			TransferServer::failedToTransferAccountDestinationNotEmpty(reply.getValue());
			break;
		}
		case constcrc("ReplyTransferDataFail") :
		{
			const GenericValueTypeMessage<TransferCharacterData> reply(ri);
			TransferServer::failedToRetrieveTransferData(reply.getValue());
			break;
		}
		case constcrc("TransferFailGameServerClosedConnectionWithConnectionServer") :
		{
			const GenericValueTypeMessage<TransferCharacterData> fail(ri);
			TransferServer::failedToTransferCharacterGameConnectionClosed(fail.getValue());
			break;
		}
		case constcrc("TransferFailConnectionServerClosedConnectionWithCentralServer") :
		{
			const GenericValueTypeMessage<unsigned int> fail(ri);
			TransferServer::failedToTransferCharacterConnectionServerConnectionClosed(fail.getValue());
			break;
		}
		case constcrc("ConGenericMessage") :
		{
			const ConGenericMessage cm(ri);
			std::string result;
			ConsoleManager::processString(cm.getMsg(), static_cast<const NetworkId::NetworkIdType>(cm.getMsgId()), result);
			const ConGenericMessage response(result, cm.getMsgId());
			send(response, true);
			break;
		}
	}
}

//-----------------------------------------------------------------------

void CentralServerConnection::setGalaxyName(const std::string & galaxyName)
{
	s_centralServerConnections[galaxyName] = this;
	m_galaxyName = galaxyName;
	REPORT_LOG("CentralServerConnection", ("Galaxy %s is handled by CentralServerConnection with %s:%d\n", m_galaxyName.c_str(), getRemoteAddress().c_str(), getRemotePort()));
}

//-----------------------------------------------------------------------
