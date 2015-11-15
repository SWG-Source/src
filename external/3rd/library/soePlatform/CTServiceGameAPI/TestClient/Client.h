#ifndef CLIENT_H
#define CLIENT_H

#include "CTServiceGameAPI/CTServiceAPI.h"
#include <Base/Platform.h>
#include <Unicode/UnicodeUtils.h>
#include "CTServiceGameAPI/CTCommon/RequestStrings.h"

using namespace CTService;

#define	TEST_MULTIPLE_SERVERS	0
#define TEST_FUNCTIONS			0
#define ACT_AS_SERVER			1

#define GING_TEST_STATUS		1		// test OnRequestMoveStatus
#define GING_TEST_VALIDATE		2		// test OnRequestValidateMove
#define GING_TEST_MOVE			3		// test OnRequestMove
#define GING_TEST_CHARACTER		4		// test OnRequestCharacterList
#define GING_TEST_SERVER		5		// test OnRequestServerList
#define GING_TEST_DESTSERVER	6		// test OnRequestDestinationServerList
#define GING_TEST_DELETE		7		// test OnRequestDelete
#define GING_TEST_RESTORE		8		// test OnRequestRestore
#define GING_TEST_TRANSFER_ACCOUNT 9	// test OnRequestRestore

//----------------------------------------
class Client : public CTServiceAPI
//----------------------------------------
{
public:
	Client(const char *hostName, const char *game);
	virtual ~Client();

	void onConnect(const char *host, const short port, const short current, const short max);
	void onDisconnect(const char *host, const short port, const short current, const short max);

    bool m_connected;
	
	//callbacks

//	void onTest(const unsigned track, const int resultCode, const unsigned value, void *user);
//	void onReplyTest(const unsigned track, const int resultCode, void *user);
	void onReplyMoveStatus(const unsigned track, const int resultCode, void *user);
	void onReplyValidateMove(const unsigned track, const int resultCode, void *user);
	void onReplyMove(const unsigned track, const int resultCode, void *user);
	void onReplyDelete(const unsigned track, const int resultCode, void *user);
	void onReplyRestore(const unsigned track, const int resultCode, void *user);
	void onReplyTransferAccount(const unsigned track, const int resultCode, void *user);
	void onReplyCharacterList(const unsigned track, const int resultCode, void *user);
	void onReplyServerList(const unsigned track, const int resultCode, void *user);
	void onReplyDestinationServerList(const unsigned track, const int resultCode, void *user);

	void onServerTest(const unsigned server_track, const char *game, const char *param);
	void onRequestMoveStatus(const unsigned server_track, const char *language, 
										const unsigned transactionID);
	void onRequestValidateMove(const unsigned server_track, const char *language, 
										const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, 
										const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, bool withItems, bool override);
	void onRequestMove(const unsigned server_track, const char *language, 
										const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, 
										const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, const unsigned transactionID, bool withItems, bool override);
	void onRequestDelete(const unsigned server_track, const char *language, 
										const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, 
										const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, const unsigned transactionID, bool withItems, bool override);
	void onRequestRestore(const unsigned server_track, const char *language, 
										const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, 
										const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, const unsigned transactionID, bool withItems, bool override);
	void onRequestTransferAccount(const unsigned server_track,
										const unsigned uid, 
										const unsigned destuid, 
										const unsigned transactionID);
	void onRequestCharacterList(const unsigned server_track, const char *language, 
										const CTUnicodeChar *server, const unsigned uid);
	void onRequestServerList(const unsigned server_track, const char *language);
	void onRequestDestinationServerList(const unsigned server_track, const char *language,
										const CTUnicodeChar *character, const CTUnicodeChar *server);
};


#endif

