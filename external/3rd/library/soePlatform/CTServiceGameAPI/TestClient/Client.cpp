#include "Client.h"
#include <stdio.h>
#include <string>
#include <map>

using namespace CTService;
using namespace Plat_Unicode;

extern unsigned						openRequests;
extern std::map<unsigned, unsigned>	m_tests;
 
//-------------------------------------------
Client::Client(const char *hostName, const char *game)
: CTServiceAPI(hostName, game), m_connected(false)
//-------------------------------------------
{
    printf("\nClient::Client()");
}

//-------------------------------------------
Client::~Client()
//-------------------------------------------
{
    printf("\nClient::~Client()");
}


//-------------------------------------------
void Client::onConnect(const char *host, short port, short current, short max)
//-------------------------------------------
{
	printf("\nConnect to %s:%d (%d of %d)", host, port, current, max);
    m_connected = true;
}

//-------------------------------------------
void Client::onDisconnect(const char *host, short port, short current, short max)
//-------------------------------------------
{
	printf("\nDisconnect from %s:%d (%d of %d)", host, port, current, max);
    m_connected = false;
}
/*
//-------------------------------------------
void Client::onTest(const unsigned track, const int resultCode, const unsigned value, void *user)
//-------------------------------------------
{
    printf("\n[onTest] track(%d) resultCode(%s) value(%d)", track, ResultString[resultCode], value);
	openRequests--;
}
*/
//-------------------------------------------
void Client::onServerTest(unsigned server_track, const char *game, const char *param)
//-------------------------------------------
{
    printf("\n[onServerTest] server_track(%d), game(%s) param(%s)", server_track, game, param);
#if TEST_MULTIPLE_SERVERS == 1
	m_tests.insert(std::pair<unsigned, unsigned>(server_track, (unsigned)(time(0) + 3)));	// 35 for fail
	printf("\nQueuing send for server track = %d", server_track);
	//replyTest(server_track, server_track * 100, "Test");
    //openRequests++;
#endif
}
/*
//-------------------------------------------
void Client::onReplyTest(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyTest] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}
*/
//-------------------------------------------
void Client::onRequestMoveStatus(unsigned server_track, const char *language, const unsigned transactionID)
//-------------------------------------------
{
    printf("\n[onRequestMoveStatus] server_track(%d), language(%s), transID(%d)", server_track, language, transactionID);
	unsigned gameStatus = CT_STATUS_UNKNOWN;
	unsigned ctStatus = CT_RESULT_TIMEOUT;
	Plat_Unicode::String reason = narrowToWide("Reason String Text");

	if (transactionID == 7000)
	{
		gameStatus = CT_STATUS_COMPLETE;
		ctStatus = CT_RESULT_SUCCESS;
	}
	else if (transactionID == 6000)
	{
		gameStatus = CT_STATUS_UNKNOWN;
		ctStatus = CT_RESULT_SUCCESS;
	}
	else if (transactionID == 5000)
	{
		gameStatus = CT_STATUS_INPROGRESS;
		ctStatus = CT_RESULT_SUCCESS;
	}
	replyMoveStatus(server_track, CT_STATUS_UNKNOWN, CT_RESULT_SUCCESS, reason.c_str(),(void *)"test");
    openRequests++;
}

//-------------------------------------------
void Client::onReplyMoveStatus(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyMoveStatus] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestValidateMove(unsigned server_track, const char *language, const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, bool withItems, bool override)
//-------------------------------------------
{
	Plat_Unicode::String sServer = sourceServer;
	Plat_Unicode::String dServer = destServer;
	Plat_Unicode::String sCharacter = sourceCharacter;
	Plat_Unicode::String dCharacter = destCharacter;
	printf("\n[onRequestValidateMove] server_track(%d), language(%s), sourceServer(%s), destServer(%s), sourceChar(%s), destChar(%s), uid(%d), destuid(%d), withItems(%d), override(%d)", 
			server_track, language, wideToNarrow(sServer).c_str(), wideToNarrow(dServer).c_str(),
			wideToNarrow(sCharacter).c_str(), wideToNarrow(dCharacter).c_str(), uid, destuid, withItems, override);

	Plat_Unicode::String reason = narrowToWide("Player has a corpse");
	Plat_Unicode::String reason2 = narrowToWide("SUCCESS");
	Plat_Unicode::String reason3 = narrowToWide("Name Taken");
	Plat_Unicode::String reason4 = narrowToWide("Game Server down for routine maitenence");
	Plat_Unicode::String suggestedName = narrowToWide("Fippyxxx");
	Plat_Unicode::String suggestedName2 = narrowToWide("");

	if (!strcmp(wideToNarrow(sCharacter).c_str() , "Homer"))
	{
		replyValidateMove(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), suggestedName2.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(dCharacter).c_str() , "Fippy"))
	{
		replyValidateMove(server_track, CT_GAMERESULT_NAME_ALREADY_TAKE, reason3.c_str(), suggestedName.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "Bart"))
	{
		replyValidateMove(server_track, CT_GAMERESULT_SOFTERROR, reason4.c_str(), suggestedName2.c_str(), (void *)"Test");
	}
	else
		replyValidateMove(server_track, CT_GAMERESULT_SUCCESS, reason2.c_str(), suggestedName2.c_str(), (void *)"Test");


    openRequests++;
}

//-------------------------------------------
void Client::onReplyValidateMove(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyValidateMove] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestMove(unsigned server_track, const char *language, const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, const unsigned transactionID, bool withItems, bool override)
//-------------------------------------------
{
	Plat_Unicode::String sServer = sourceServer;
	Plat_Unicode::String dServer = destServer;
	Plat_Unicode::String sCharacter = sourceCharacter;
	Plat_Unicode::String dCharacter = destCharacter;
	printf("\n[onRequestMove] server_track(%d), language(%s), sourceServer(%s), destServer(%s), sourceChar(%s), destChar(%s), uid(%d), destuid(%d), transID(%d) withItems(%d) override(%d)", 
			server_track, language, wideToNarrow(sServer).c_str(), wideToNarrow(dServer).c_str(),
			wideToNarrow(sCharacter).c_str(), wideToNarrow(dCharacter).c_str(), uid, destuid, transactionID, withItems, override);

//	Plat_Unicode::String reason = narrowToWide("Character does not exist");
 //   replyMove(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), "Test");
	
	if (!strcmp(wideToNarrow(sCharacter).c_str() , "HomerX"))
	{
		Plat_Unicode::String reason = narrowToWide("");
		replyMove(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "MargeX"))
	{
		Plat_Unicode::String reason = narrowToWide("Player has a corpse");
		replyMove(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "BartX"))
	{
		Plat_Unicode::String reason = narrowToWide("Game Server down for routine maitenence");
		replyMove(server_track, CT_GAMERESULT_SOFTERROR, reason.c_str(), (void *)"Test");
	}
	else
	{
		Plat_Unicode::String reason = narrowToWide("Order completed successfully");
		replyMove(server_track, CT_GAMERESULT_SUCCESS, reason.c_str(), (void *)"Test");
	}
//	else
//	{
//		Plat_Unicode::String reason = narrowToWide("Player has a corpse");
//		replyMove(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), "Test");
//	}
	
//	Plat_Unicode::String reason = narrowToWide("World server is down");
//	replyMove(server_track, CT_GAMERESULT_SOFTERROR, reason.c_str(), "Test");

    openRequests++;
}

//-------------------------------------------
void Client::onReplyMove(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyMove] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestDelete(unsigned server_track, const char *language, const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, const unsigned transactionID, bool withItems, bool override)
//-------------------------------------------
{
	Plat_Unicode::String sServer = sourceServer;
	Plat_Unicode::String dServer = destServer;
	Plat_Unicode::String sCharacter = sourceCharacter;
	Plat_Unicode::String dCharacter = destCharacter;
	printf("\n[onRequestDelete] server_track(%d), language(%s), sourceServer(%s), destServer(%s), sourceChar(%s), destChar(%s), uid(%d), destuid(%d), transID(%d), withItems(%d), override(%d)", 
			server_track, language, wideToNarrow(sServer).c_str(), wideToNarrow(dServer).c_str(),
			wideToNarrow(sCharacter).c_str(), wideToNarrow(dCharacter).c_str(), uid, destuid, transactionID, withItems, override);

//	Plat_Unicode::String reason = narrowToWide("Character does not exist");
 //   replyMove(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), "Test");
	
	if (!strcmp(wideToNarrow(sCharacter).c_str() , "HomerX"))
	{
		Plat_Unicode::String reason = narrowToWide("");
		replyDelete(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "MargeX"))
	{
		Plat_Unicode::String reason = narrowToWide("Player has a corpse");
		replyDelete(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "BartX"))
	{
		Plat_Unicode::String reason = narrowToWide("Game Server down for routine maitenence");
		replyDelete(server_track, CT_GAMERESULT_SOFTERROR, reason.c_str(), (void *)"Test");
	}
	else
	{
		Plat_Unicode::String reason = narrowToWide("Order completed successfully");
		replyDelete(server_track, CT_GAMERESULT_SUCCESS, reason.c_str(), (void *)"Test");
	}
//	else
//	{
//		Plat_Unicode::String reason = narrowToWide("Player has a corpse");
//		replyDelete(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), "Test");
//	}
	
//	Plat_Unicode::String reason = narrowToWide("World server is down");
//	replyDelete(server_track, CT_GAMERESULT_SOFTERROR, reason.c_str(), "Test");

    openRequests++;
}

//-------------------------------------------
void Client::onReplyDelete(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyDelete] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestRestore(unsigned server_track, const char *language, const CTUnicodeChar *sourceServer,
										const CTUnicodeChar *destServer, const CTUnicodeChar *sourceCharacter,
										const CTUnicodeChar *destCharacter, const unsigned uid, 
										const unsigned destuid, const unsigned transactionID, bool withItems, bool override)
//-------------------------------------------
{
	Plat_Unicode::String sServer = sourceServer;
	Plat_Unicode::String dServer = destServer;
	Plat_Unicode::String sCharacter = sourceCharacter;
	Plat_Unicode::String dCharacter = destCharacter;
	printf("\n[onRequestRestore] server_track(%d), language(%s), sourceServer(%s), destServer(%s), sourceChar(%s), destChar(%s), uid(%d), destuid(%d), transID(%d), withItems(%d), override(%d)", 
			server_track, language, wideToNarrow(sServer).c_str(), wideToNarrow(dServer).c_str(),
			wideToNarrow(sCharacter).c_str(), wideToNarrow(dCharacter).c_str(), uid, destuid, transactionID, withItems, override);

//	Plat_Unicode::String reason = narrowToWide("Character does not exist");
 //   replyMove(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), "Test");
	
	if (!strcmp(wideToNarrow(sCharacter).c_str() , "HomerX"))
	{
		Plat_Unicode::String reason = narrowToWide("");
		replyRestore(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "MargeX"))
	{
		Plat_Unicode::String reason = narrowToWide("Player has a corpse");
		replyRestore(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), (void *)"Test");
	}
	else if (!strcmp(wideToNarrow(sCharacter).c_str() , "BartX"))
	{
		Plat_Unicode::String reason = narrowToWide("Game Server down for routine maitenence");
		replyRestore(server_track, CT_GAMERESULT_SOFTERROR, reason.c_str(), (void *)"Test");
	}
	else
	{
		Plat_Unicode::String reason = narrowToWide("Order completed successfully");
		replyRestore(server_track, CT_GAMERESULT_SUCCESS, reason.c_str(), (void *)"Test");
	}
//	else
//	{
//		Plat_Unicode::String reason = narrowToWide("Player has a corpse");
//		replyRestore(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), "Test");
//	}
	
//	Plat_Unicode::String reason = narrowToWide("World server is down");
//	replyRestore(server_track, CT_GAMERESULT_SOFTERROR, reason.c_str(), "Test");

    openRequests++;
}

//-------------------------------------------
void Client::onReplyRestore(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyRestore] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestTransferAccount(unsigned server_track,
										const unsigned uid, 
										const unsigned destuid, 
										const unsigned transactionID)
//-------------------------------------------
{
	printf("\n[onRequestTransferAccount] server_track(%d), uid(%d), destuid(%d), transID(%d)", 
			server_track, uid, destuid, transactionID);

//	Plat_Unicode::String reason = narrowToWide("Character does not exist");
 //   replyMove(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), "Test");
	
	if (uid == 521281339)
	{
		Plat_Unicode::String reason = narrowToWide("Tavish is too cool to move");
		replyTransferAccount(server_track, CT_GAMERESULT_HARDERROR, reason.c_str(), (void *)"Test");
	}
	else if (destuid == 549981672)
	{
		Plat_Unicode::String reason = narrowToWide("devula as a dest ain't cool, there is a corpse to be rezzed");
		replyTransferAccount(server_track, CT_GAMERESULT_HAS_CORPSE, reason.c_str(), (void *)"Test");
	}
	else
	{
		Plat_Unicode::String reason = narrowToWide("Order completed successfully");
		replyTransferAccount(server_track, CT_GAMERESULT_SUCCESS, reason.c_str(), (void *)"Test");
	}

    openRequests++;
}

//-------------------------------------------
void Client::onReplyTransferAccount(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyTransferAccount] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestCharacterList(unsigned server_track, const char *language, const CTUnicodeChar *server, const unsigned uid)
//-------------------------------------------
{
	Plat_Unicode::String sServer = server;
	printf("\n[onRequestCharacterList] server_track(%d), language(%s), server(%s), uid(%d)", 
			server_track, language, wideToNarrow(sServer).c_str(), uid);

	if (!strcmp(wideToNarrow(sServer).c_str() , "Tribunal"))
	{
		CTServiceCharacter chars[1];
		replyCharacterList(server_track, CT_RESULT_FAILURE, 0, chars, (void *)"Test");
		
	}
	else if (!strcmp(wideToNarrow(sServer).c_str() , "Hoth"))
	{
		CTServiceCharacter chars[1];
		replyCharacterList(server_track, CT_RESULT_TIMEOUT, 0, chars, (void *)"Test");
	}
	else
	{
		CTServiceCharacter chars[3];
		chars[0].SetCanRename(true);
		chars[0].SetCanMove(true);
		chars[0].SetCanTransfer(true);
		chars[0].SetCharacter(narrowToWide("Fippy").c_str());
		chars[0].SetRenameReason(narrowToWide("Rename Reason 1").c_str());
		chars[0].SetMoveReason(narrowToWide("Move Reason 1").c_str());
		chars[0].SetTransferReason(narrowToWide("Transfer Reason 1").c_str());

		chars[1].SetCanRename(false);
		chars[1].SetCanMove(false);
		chars[1].SetCanTransfer(false);
		chars[1].SetCharacter(narrowToWide("Melyssa").c_str());
		chars[1].SetRenameReason(narrowToWide("Rename Reason 2").c_str());
		chars[1].SetMoveReason(narrowToWide("Move Reason 2").c_str());
		chars[1].SetTransferReason(narrowToWide("Transfer Reason 2").c_str());

		chars[2].SetCanRename(true);
		chars[2].SetCanMove(false);
		chars[2].SetCanTransfer(true);
		chars[2].SetCharacter(narrowToWide("Bald Eagle").c_str());
		chars[2].SetRenameReason(narrowToWide("Rename Reason 3").c_str());
		chars[2].SetMoveReason(narrowToWide("Move Reason 3").c_str());
		chars[2].SetTransferReason(narrowToWide("Transfer Reason 3").c_str());

		replyCharacterList(server_track, CT_RESULT_SUCCESS, 3, chars, (void *)"Test");
	}
    openRequests++;
}

//-------------------------------------------
void Client::onReplyCharacterList(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyCharacterList] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestServerList(unsigned server_track, const char *language)
//-------------------------------------------
{
	printf("\n[onRequestServerList] server_track(%d), language(%s)", server_track, language);

	if (strcmp(language, "en"))
	{
		CTServiceServer s[1];
		replyServerList(server_track, CT_RESULT_FAILURE, 0, s, (void *)"Test");
	}
	else
	{
		CTServiceServer s[4];
		s[0].SetCanRename(true);
		s[0].SetCanMove(true);
		s[0].SetCanTransfer(true);
		s[0].SetServer(narrowToWide("Server1").c_str());
		s[0].SetRenameReason(narrowToWide("Server Rename Reason 1").c_str());
		s[0].SetMoveReason(narrowToWide("Server Move Reason 1").c_str());
		s[0].SetTransferReason(narrowToWide("Server Transfer Reason 1").c_str());

		s[1].SetCanRename(true);
		s[1].SetCanMove(true);
		s[1].SetCanTransfer(false);
		s[1].SetServer(narrowToWide("Server2").c_str());
		s[1].SetRenameReason(narrowToWide("Server Rename Reason 2").c_str());
		s[1].SetMoveReason(narrowToWide("Server Move Reason 2").c_str());
		s[1].SetTransferReason(narrowToWide("Server Transfer Reason 2").c_str());

		s[2].SetCanRename(true);
		s[2].SetCanMove(false);
		s[2].SetCanTransfer(true);
		s[2].SetServer(narrowToWide("Server3").c_str());
		s[2].SetRenameReason(narrowToWide("Server Rename Reason 3").c_str());
		s[2].SetMoveReason(narrowToWide("Server Move Reason 3").c_str());
		s[2].SetTransferReason(narrowToWide("Server Transfer Reason 3").c_str());

		s[3].SetCanRename(false);
		s[3].SetCanMove(true);
		s[3].SetCanTransfer(true);
		s[3].SetServer(narrowToWide("Server4").c_str());
		s[3].SetRenameReason(narrowToWide("Server Rename Reason 4").c_str());
		s[3].SetMoveReason(narrowToWide("Server Move Reason 4").c_str());
		s[3].SetTransferReason(narrowToWide("Server Transfer Reason 4").c_str());

		replyServerList(server_track, CT_RESULT_SUCCESS, 4, s, (void *)"Test");
	}
    openRequests++;
}

//-------------------------------------------
void Client::onReplyServerList(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyServerList] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}

//-------------------------------------------
void Client::onRequestDestinationServerList(unsigned server_track, const char *language, const CTUnicodeChar *character, const CTUnicodeChar *server)
//-------------------------------------------
{
	Plat_Unicode::String ss = server;
	Plat_Unicode::String c = character;

	printf("\n[onRequestDestinationServerList] server_track(%d), language(%s), character(%s), server(%s)", 
		server_track, language, wideToNarrow(c).c_str(), wideToNarrow(ss).c_str());

	if (strcmp(language, "en"))
	{
		CTServiceServer s[1];
		replyServerList(server_track, CT_RESULT_FAILURE, 0, s, (void *)"Test");
	}
	else
	{
		CTServiceServer s[2];
		s[0].SetCanRename(true);
		s[0].SetCanMove(true);
		s[0].SetCanTransfer(true);
		s[0].SetServer(narrowToWide("Server1").c_str());
		s[0].SetRenameReason(narrowToWide("Server Rename Reason 1").c_str());
		s[0].SetMoveReason(narrowToWide("Server Move Reason 1").c_str());
		s[0].SetTransferReason(narrowToWide("Server Transfer Reason 1").c_str());

		s[1].SetCanRename(true);
		s[1].SetCanMove(true);
		s[1].SetCanTransfer(false);
		s[1].SetServer(narrowToWide("Server2").c_str());
		s[1].SetRenameReason(narrowToWide("Server Rename Reason 2").c_str());
		s[1].SetMoveReason(narrowToWide("Server Move Reason 2").c_str());
		s[1].SetTransferReason(narrowToWide("Server Transfer Reason 2").c_str());

		replyDestinationServerList(server_track, CT_RESULT_SUCCESS, 2, s, (void *)"Test");
	}
    openRequests++;
}

//-------------------------------------------
void Client::onReplyDestinationServerList(const unsigned track, const int resultCode, void *user)
//-------------------------------------------
{
    printf("\n[onReplyDestinationServerList] track(%d) resultCode(%s)", track, ResultString[resultCode]);
	openRequests--;
}
