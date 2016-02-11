#include <conio.h>
#include <time.h>
#include <stdio.h>
#include <map>
#include "CTServiceGameAPI/CTServiceAPI.h"

namespace CTService 
{
	class Client : public CTServiceAPI
	{
		public:
			Client(const char *hostList, const char *game);
			virtual ~Client();

			virtual void onConnect(const char *host, const short port, const short current, const short max);
			virtual void onDisconnect(const char *host, const short port, const short current, const short max);

			virtual void onTest(const unsigned track, const int resultCode, const unsigned value, void *user);
			virtual void onReplyTest(const unsigned track, const int resultCode, void *user);
			virtual void onReplyMoveStatus(const unsigned track, const int resultCode, void *user);
			virtual void onReplyValidateMove(const unsigned track, const int resultCode, void *user);
			virtual void onReplyMove(const unsigned track, const int resultCode, void *user);
			virtual void onReplyCharacterList(const unsigned track, const int resultCode, void *user);
			virtual void onReplyServerList(const unsigned track, const int resultCode, void *user);
			virtual void onReplyDestinationServerList(const unsigned track, const int resultCode, void *user);

			virtual void onServerTest(const unsigned server_track, const char *game, const char *param);
			virtual void onRequestMoveStatus(const unsigned server_track, const char *language, 
												const unsigned transactionID);
			virtual void onRequestValidateMove(const unsigned server_track, const char *language, 
												const CTUnicodeChar *sourceServer,
												const CTUnicodeChar *destServer, 
												const CTUnicodeChar *sourceCharacter,
												const CTUnicodeChar *destCharacter, const unsigned uid, 
												const unsigned destuid, bool withItems);
			virtual void onRequestMove(const unsigned server_track, const char *language, 
												const CTUnicodeChar *sourceServer,
												const CTUnicodeChar *destServer, 
												const CTUnicodeChar *sourceCharacter,
												const CTUnicodeChar *destCharacter, const unsigned uid, 
												const unsigned destuid, const unsigned transactionID, bool withItems);
			virtual void onRequestCharacterList(const unsigned server_track, const char *language, 
												const CTUnicodeChar *server, const unsigned uid);
			virtual void onRequestServerList(const unsigned server_track, const char *language);
			virtual void onRequestDestinationServerList(const unsigned server_track, const char *language,
												const CTUnicodeChar *character, const CTUnicodeChar *server);

			std::map<unsigned, unsigned> mTransactionMap;
	};

	Client::Client(const char *hostList, const char *game) :
		CTServiceAPI(hostList, game),
		mTransactionMap()
	{
	}

	Client::~Client()
	{
	}

	void Client::onConnect(const char *host, 
		const short port, 
		const short current, 
		const short max)
	{
		printf("onConnect(%s, %u, %d, %d)\n",host,port,current,max);
	}

	void Client::onDisconnect(const char *host, 
		const short port, 
		const short current, 
		const short max)
	{
		printf("onDisconnect(%s, %u, %d, %d)\n",host,port,current,max);
	}

	void Client::onTest(const unsigned track, 
		const int resultCode, 
		const unsigned value, 
		void *user)
	{
		printf("onTest(%u, %d, %u, 0x%x)\n", track, resultCode, value, user);
	}

	void Client::onReplyTest(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyTest(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onReplyMoveStatus(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyMoveStatus(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onReplyValidateMove(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyValidateMove(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onReplyMove(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyMove(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onReplyCharacterList(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyCharacterList(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onReplyServerList(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyServerList(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onReplyDestinationServerList(const unsigned track, 
		const int resultCode, 
		void *user)
	{
		printf("onReplyDestinationServerList(%u, %d, 0x%x)\n", track, resultCode, user);
	}

	void Client::onServerTest(const unsigned server_track, 
		const char *game, 
		const char *param)
	{
		printf("onServerTest(%u, %s, %s)\n", server_track, game ? game : "(nullptr)", param ? param : "(nullptr)");
		replyTest(server_track, 999, 0);
	}

	void Client::onRequestMoveStatus(const unsigned server_track, 
		const char *language, 
		const unsigned transactionID)
	{
		unsigned status;
		unsigned result;
		unsigned short * reason = L"makes me horny it does";

		printf("#%x onRequestMoveStatus(%s, #%u)\n", 
			server_track,
			language,
			transactionID);

		if (mTransactionMap.find(transactionID) != mTransactionMap.end())
		{
			status = CT_STATUS_COMPLETE;
			result = mTransactionMap[transactionID];
		}
		else
		{
			status = CT_STATUS_UNKNOWN;
			result = CT_GAMERESULT_SUCCESS;
		}

		replyMoveStatus(server_track, status, result, reason, 0);
	}

	void Client::onRequestValidateMove(const unsigned server_track, 
		const char *language, 
		const CTUnicodeChar *sourceServer,
		const CTUnicodeChar *destServer, 
		const CTUnicodeChar *sourceCharacter,
		const CTUnicodeChar *destCharacter, 
		const unsigned uid, 
		const unsigned destuid, bool withItems)
	{
		static resultArray[4] = {CT_GAMERESULT_SUCCESS, CT_GAMERESULT_SOFTERROR, CT_GAMERESULT_HARDERROR, CT_GAMERESULT_INVALID_NAME};
		static resultIndex = 0;

		printf("#%x onRequestValidateMove(%s): server(%S > %S) character (%S > %S) user(%u > %u) %s\n", 
			server_track,
			language,
			sourceServer,
			destServer,
			sourceCharacter,
			destCharacter,
			uid,
			destuid,
			withItems ? "/w items" : "/wo items");

		int result = resultArray[(resultIndex++)%4];
		unsigned short * reason = L"makes me horny it does";
		unsigned short * suggested = L"yoda";

		replyValidateMove(server_track, result, reason, suggested, 0);
	}

	void Client::onRequestMove(const unsigned server_track, 
		const char *language, 
		const CTUnicodeChar *sourceServer,
		const CTUnicodeChar *destServer, 
		const CTUnicodeChar *sourceCharacter,
		const CTUnicodeChar *destCharacter, 
		const unsigned uid, 
		const unsigned destuid, 
		const unsigned transactionID, bool withItems)
	{
		static resultArray[4] = {CT_GAMERESULT_SUCCESS, CT_GAMERESULT_SOFTERROR, CT_GAMERESULT_HARDERROR, CT_GAMERESULT_INVALID_NAME};
		static resultIndex = 0;

		printf("#%x onRequestMove(%s, #%u): server(%S > %S) character (%S > %S) user(%u > %u) %s\n", 
			server_track,
			language,
			transactionID,
			sourceServer,
			destServer,
			sourceCharacter,
			destCharacter,
			uid,
			destuid,
			withItems ? "/w items" : "/wo items");

		int result = resultArray[(resultIndex++)%4];
		unsigned short * reason = L"makes me horny it does";
		unsigned short * suggested = L"yoda";
		mTransactionMap[transactionID] = result;
		replyMove(server_track, result, reason, 0);
	}

	void Client::onRequestCharacterList(const unsigned server_track, 
		const char *language, 
		const CTUnicodeChar *server, 
		const unsigned uid)
	{
		CTServiceCharacter characters[4] = 
		{
			L"wussup1",
			L"wussup2",
			L"wussup3",
			L"wussup4"
		};

		printf("#%x onRequestCharacterList(%s, %S, %u)\n", server_track, language, server,uid);
		replyCharacterList(server_track, CT_GAMERESULT_SUCCESS, 4, characters, 0);
	}

	void Client::onRequestServerList(const unsigned server_track, 
		const char *language)
	{
		CTServiceServer servers[4] = 
		{
			L"crib1",
			L"crib2",
			L"crib3",
			L"crib4"
		};

		printf("#%x onRequestServerList(%s)\n", server_track, language);
		replyServerList(server_track, CT_GAMERESULT_SUCCESS, 4, servers, 0);
	}

	void Client::onRequestDestinationServerList(const unsigned server_track, 
		const char *language,
		const CTUnicodeChar *character, 
		const CTUnicodeChar *server)
	{
		CTServiceServer servers[2] = 
		{
			L"crib5",
			L"crib6"
		};

		printf("#%x onRequestDestinationServerList(%s)\n", server_track, language);
		replyDestinationServerList(server_track, CT_GAMERESULT_SUCCESS, 2, servers, 0);
	}

}

int main()
{
	char * hostList = "sdplatdev2:2000";
	char * game = "SWG";
	CTService::Client client(hostList, game);
	while (!kbhit())
	{
		client.process();
	}

	return 0;
}

