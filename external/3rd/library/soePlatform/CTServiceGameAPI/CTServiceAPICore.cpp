#include "CTServiceAPICore.h"
#include "CTServiceAPI.h"
#include "Response.h"
#include <Unicode/UnicodeUtils.h>

namespace CTService 
{

using namespace Base;
using namespace Plat_Unicode;

//----------------------------------------
CTServiceAPICore::CTServiceAPICore(const char *host[], const short port[], int count, CTServiceAPI *api, const char *game)
: GenericAPICore(game, host, port, count, 45, 5, 0, 90, 32, 32), 
  m_api(api), m_mappedServerTrack(1000)
//----------------------------------------
{
}

//----------------------------------------
CTServiceAPICore::~CTServiceAPICore()
//----------------------------------------
{
}

//----------------------------------------
void CTServiceAPICore::OnConnect(GenericConnection *con)
//----------------------------------------
{
	countOpenConnections();
	m_api->onConnect(con->getHost(), con->getPort(), m_currentConnections, m_maxConnections);
}

//----------------------------------------
void CTServiceAPICore::OnDisconnect(GenericConnection *con)
//----------------------------------------
{
	countOpenConnections();
	m_api->onDisconnect(con->getHost(), con->getPort(), m_currentConnections, m_maxConnections);
}

//----------------------------------------
void CTServiceAPICore::responseCallback(short type, Base::ByteStream::ReadIterator &iter, GenericConnection *con)
// Received a request/notification from the server
//----------------------------------------
{
	unsigned real_server_track;
	get(iter, real_server_track);
	ServerTrackObject *stobj = new ServerTrackObject(++m_mappedServerTrack, real_server_track, con);
	m_serverTracks.insert(std::pair<unsigned, ServerTrackObject *>(m_mappedServerTrack, stobj));
	//printf("\nMapping %d to %d", real_server_track, m_mappedServerTrack);	//debug

	switch (type)
	{
		case CTGAME_SERVER_TEST:
			{
					std::string game, param;
					get(iter, game);
					get(iter, param);
					m_api->onServerTest(m_mappedServerTrack, game.c_str(), param.c_str());
					break;
			}
		case CTGAME_SERVER_MOVESTATUS:
			{
					std::string lang;
					unsigned transactionID;
					get(iter, lang);
					get(iter, transactionID);
					m_api->onRequestMoveStatus(m_mappedServerTrack, lang.c_str(), transactionID);
					break;
			}
		case CTGAME_SERVER_VALIDATEMOVE:
			{
					std::string lang;
					Plat_Unicode::String sourceServer, destServer, sourceCharacter, destCharacter;
					unsigned UID, destUID;
					bool withItems;
					bool override;
					get(iter, lang);
					get(iter, sourceServer);
					get(iter, destServer);
					get(iter, sourceCharacter);
					get(iter, destCharacter);
					get(iter, UID);
					get(iter, destUID);
					get(iter, withItems);
					get(iter, override);
					m_api->onRequestValidateMove(m_mappedServerTrack, lang.c_str(), sourceServer.c_str(), destServer.c_str(),
								sourceCharacter.c_str(), destCharacter.c_str(), UID, destUID, withItems, override);
					break;
			}
		case CTGAME_SERVER_MOVE:
			{
					std::string lang;
					Plat_Unicode::String sourceServer, destServer, sourceCharacter, destCharacter;
					unsigned UID, destUID, transactionID;
					bool withItems;
					bool override;
					get(iter, lang);
					get(iter, sourceServer);
					get(iter, destServer);
					get(iter, sourceCharacter);
					get(iter, destCharacter);
					get(iter, UID);
					get(iter, destUID);
					get(iter, transactionID);
					get(iter, withItems);
					get(iter, override);
					m_api->onRequestMove(m_mappedServerTrack, lang.c_str(), sourceServer.c_str(), destServer.c_str(),
								sourceCharacter.c_str(), destCharacter.c_str(), UID, destUID, transactionID, withItems, override);
					break;
			}
		case CTGAME_SERVER_DELETE:
			{
					std::string lang;
					Plat_Unicode::String sourceServer, destServer, sourceCharacter, destCharacter;
					unsigned UID, destUID, transactionID;
					bool withItems;
					bool override;
					get(iter, lang);
					get(iter, sourceServer);
					get(iter, destServer);
					get(iter, sourceCharacter);
					get(iter, destCharacter);
					get(iter, UID);
					get(iter, destUID);
					get(iter, transactionID);
					get(iter, withItems);
					get(iter, override);
					m_api->onRequestDelete(m_mappedServerTrack, lang.c_str(), sourceServer.c_str(), destServer.c_str(),
								sourceCharacter.c_str(), destCharacter.c_str(), UID, destUID, transactionID, withItems, override);
					break;
			}
		case CTGAME_SERVER_RESTORE:
			{
					std::string lang;
					Plat_Unicode::String sourceServer, destServer, sourceCharacter, destCharacter;
					unsigned UID, destUID, transactionID;
					bool withItems;
					bool override;
					get(iter, lang);
					get(iter, sourceServer);
					get(iter, destServer);
					get(iter, sourceCharacter);
					get(iter, destCharacter);
					get(iter, UID);
					get(iter, destUID);
					get(iter, transactionID);
					get(iter, withItems);
					get(iter, override);
					m_api->onRequestRestore(m_mappedServerTrack, lang.c_str(), sourceServer.c_str(), destServer.c_str(),
								sourceCharacter.c_str(), destCharacter.c_str(), UID, destUID, transactionID, withItems, override);
					break;
			}
		case CTGAME_SERVER_TRANSFER_ACCOUNT:
			{
					unsigned UID, destUID, transactionID;
					get(iter, UID);
					get(iter, destUID);
					get(iter, transactionID);
					m_api->onRequestTransferAccount(m_mappedServerTrack, UID, destUID, transactionID);
					break;
			}
		case CTGAME_SERVER_CHARACTERLIST:
			{
					std::string lang;
					Plat_Unicode::String server;
					unsigned uid;
					get(iter, lang);
					get(iter, server);
					get(iter, uid);
					m_api->onRequestCharacterList(m_mappedServerTrack, lang.c_str(), server.c_str(), uid);
					break;
			}
		case CTGAME_SERVER_SERVERLIST:
			{
					std::string lang;
					get(iter, lang);
					m_api->onRequestServerList(m_mappedServerTrack, lang.c_str());
					break;
			}
		case CTGAME_SERVER_DESTSERVERLIST:
			{
					std::string lang;
					Plat_Unicode::String character, server;
					get(iter, lang);
					get(iter, character);
					get(iter, server);
					m_api->onRequestDestinationServerList(m_mappedServerTrack, lang.c_str(), character.c_str(), server.c_str());
					break;
			}
	}
}

//----------------------------------------
void CTServiceAPICore::responseCallback(GenericResponse *res)
//----------------------------------------
{
	switch(res->getType())
	{
		/*case CTGAME_REQUEST_TEST:
							m_api->onTest(((ResTest *)res)->getTrack(), ((ResTest *)res)->getResult(), 
								((ResTest *)res)->getValue(), ((ResTest *)res)->getUser());
							break;
		case CTGAME_REPLY_TEST:
							m_api->onReplyTest(((ResTest *)res)->getTrack(), ((ResTest *)res)->getResult(), 
								((ResTest *)res)->getUser());
							break;*/
		case CTGAME_REPLY_MOVESTATUS:
							m_api->onReplyMoveStatus(((ResReplyMoveStatus *)res)->getTrack(),
								((ResReplyMoveStatus *)res)->getResult(), 
								((ResReplyMoveStatus *)res)->getUser());
							break;
		case CTGAME_REPLY_VALIDATEMOVE:
							m_api->onReplyValidateMove(((ResReplyValidateMove *)res)->getTrack(),
								((ResReplyValidateMove *)res)->getResult(), 
								((ResReplyValidateMove *)res)->getUser());
							break;
		case CTGAME_REPLY_MOVE:
							m_api->onReplyMove(((ResReplyMove *)res)->getTrack(),
								((ResReplyMove *)res)->getResult(), 
								((ResReplyMove *)res)->getUser());
							break;
		case CTGAME_REPLY_DELETE:
							m_api->onReplyDelete(((ResReplyDelete *)res)->getTrack(),
								((ResReplyDelete *)res)->getResult(), 
								((ResReplyDelete *)res)->getUser());
							break;
		case CTGAME_REPLY_RESTORE:
							m_api->onReplyRestore(((ResReplyRestore *)res)->getTrack(),
								((ResReplyRestore *)res)->getResult(), 
								((ResReplyRestore *)res)->getUser());
							break;
		case CTGAME_REPLY_TRANSFER_ACCOUNT:
							m_api->onReplyTransferAccount(((ResReplyTransferAccount *)res)->getTrack(),
								((ResReplyTransferAccount *)res)->getResult(), 
								((ResReplyTransferAccount *)res)->getUser());
							break;
		case CTGAME_REPLY_CHARACTERLIST:
							m_api->onReplyCharacterList(((ResReplyCharacterList *)res)->getTrack(),
								((ResReplyCharacterList *)res)->getResult(), 
								((ResReplyCharacterList *)res)->getUser());
							break;
		case CTGAME_REPLY_SERVERLIST:
							m_api->onReplyServerList(((ResReplyServerList *)res)->getTrack(),
								((ResReplyServerList *)res)->getResult(), 
								((ResReplyServerList *)res)->getUser());
							break;
		case CTGAME_REPLY_DESTSERVERLIST:
							m_api->onReplyDestinationServerList(((ResReplyDestinationServerList *)res)->getTrack(),
								((ResReplyDestinationServerList *)res)->getResult(), 
								((ResReplyDestinationServerList *)res)->getUser());
							break;
		default:
							break;
	}
}

}; // namespace
