#include <stdio.h>
#include <string>
//#include "Clock.h"
#include "Base/cmdLine.h"
#include "Base/timer.h"
#include "VChatClient.h"
#include <time.h>
#include "Base/stringutils.h"

//----------------------------------------------------------------------
//-- this is a SWG hack to work with our modified STLPORT
#ifdef WIN32
enum MemoryManagerNotALeak
{
	MM_notALeak
};

void * __cdecl operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}
#endif

#define HELP_SWITCH			"-?"
#define HOST_SWITCH			"-h"
#define PORT_SWITCH			"-p"
#define USER_SWITCH			"-u"
#define TIMEOUT_SWITCH		"-t"
#define INTERACTIVE_SWITCH	"-i"
#define TEST_INIT_SWITCH	"-test"

void printUsage(const char *executableName)
{
	printf("Usage : %s [" HELP_SWITCH "] "
		HOST_SWITCH " hostlist "
		"[" TIMEOUT_SWITCH " timeout_in_seconds] "
		"[" INTERACTIVE_SWITCH " waits_for_user_to_exit ] "
		"[" TEST_INIT_SWITCH " runs_Init_test ]"
		"[" USER_SWITCH " used_with -test valid_user_id ]\n",
		executableName);
}

int main(int argc, char **argv){

	CmdLine cmdLine;

	cmdLine.SplitLine(argc, argv);

	std::string hostList = cmdLine.GetSafeArgument(HOST_SWITCH, 0, "localhost:9100");
	bool showUsage = cmdLine.HasSwitch(HELP_SWITCH) || !cmdLine.HasSwitch(HOST_SWITCH);
	unsigned timeout_in_seconds  = atoi(cmdLine.GetSafeArgument(TIMEOUT_SWITCH, 0, "10").c_str());

	if (showUsage) {
		printUsage(argv[0]);
		return 0;
	}

	srand((unsigned)time(0));
	int x = rand();
	int y = rand();

	y *= x;

	unsigned count = 0;
	
	char c;
	do 
	{
		VChatClient api(hostList.c_str());
		printf("Connecting to %s...\n", hostList.c_str());

		while(!api.IsConnected() && !api.HasFailed())
		{
			api.Process();
			soe::Sleep(10);
		}

		y++;
		unsigned result = 0;
		char numBuff[32];
		sprintf(numBuff, "RandomName%d", y);

		std::string name = numBuff;
		std::string game = "EQ2";
		std::string server = "Butcherblock";

		std::string lowerName = soe::tolowercase(name);
		std::string lowerServer = soe::tolowercase(server);
		std::string lowerGame = soe::tolowercase(game);

		printf("Login Request: %s\n", name.c_str());

		// first one adds
		printf("GetAccountEx -- user: %s\n", name.c_str());

		result = api.GetAccountEx(name, game, server, 1111, 9999);
		
		if (!api.m_previousUser.empty())
		{
			std::string tmpName, tmpServer;

			GetUserNameComponents(api.m_previousUser, tmpServer, tmpName);
			
			printf("GetAccountEx -- user: %s\n", tmpName.c_str());

			// second one gets.
			result = api.GetAccountEx(tmpName, game, tmpServer, 1111, 9999);
		}

		
		printf("GetAccountEx -- user: %s\n", name.c_str());

		// second one gets.
		result = api.GetAccountEx(name, game, server, 1111, 9999);
	
// 		
// 		printf("GetAccountEx -- user: %s\n", lowerName.c_str());
// 
// 		// gets with all lower
// 		result = api.GetAccountEx(lowerName, lowerGame, lowerServer, 1111, 9999);
// 		
// 		printf("GetAccountEx -- user: %s\n", lowerName.c_str());
// 
// 
// 		// gets with lowerName
// 		result = api.GetAccountEx(lowerName, game, server, 1111, 9999);
// 
// 		
// 		printf("GetAccountEx -- user: %s\n", name.c_str());
// 
// 		// gets with lowerGamer.
// 		result = api.GetAccountEx(name, lowerGame, server, 1111, 9999);
// 
// 		sprintf(numBuff, "Random Name%d", y);
// 		
// 		
// 
// 		// creates account with space.
// 		result = api.GetAccountEx(numBuff, game, lowerServer, 1111, 9999);

// 		
// 
// 		// gets account with space.
// 		result = api.GetAccountEx(numBuff, game, lowerServer, 1111, 9999);

// 		
// 
// 		sprintf(numBuff, "Random's Name%d", y);
// 
// 		// creates account with space and apostrophe.
// 		result = api.GetAccountEx(numBuff, game, lowerServer, 1111, 9999);

		

		std::string guild = "guild.";
		guild += name;

		

// 		result = api.GetChannelInfoEx(guild, game, server);
// 
// 		
// 
// 		result = api.GetChannelEx(guild, game, server, "Description space", "", 3000, 0);
// 		
// 		
// 
// 		result = api.GetChannelEx(guild, game, server, "Description space", "", 3000, 0);

		
		std::string guild2 = "guild2.";
		guild2 += name;


		result = api.GetChannelV2Ex(guild2, game, server, "Description space", "firewall", 100, 0);
// 
// 		result = api.GetChannelV2Ex(guild2, game, server, "Description space", "firewall", 100, 0);
// 
// 
// 		result = api.ChangePasswordEx(guild, game, server, "sony1234");
// 		
// 		
// 
// 		result = api.GetChannelInfoEx(guild, game, server);
// 
// 		

// 		std::string space = "Ga illed.";
// 		space += name;
// 		result = api.GetChannelEx(space, game, server, "Description's apostrophe", "", 100, 0);
// 		
// 		
// 		
// 		result = api.GetChannelEx(space, game, server, "Description's apostrophe", "", 100, 0);
// 
// 		
// 
// 		result = api.DeleteChannelEx(space, game, server);
// 
// 		if (count < 100)
// 		{
// 			
// 
// 			std::string proximity = "prox-";
// 			proximity += name;
// 
// 			result = api.GetProximityChannelEx(proximity, game, server, "Description", "", 100, 0, 80, 10, 2.5, 1.5, 1);
// 
// 			
// 
// 			result = api.GetProximityChannelEx(proximity, game, server, "Description", "", 100, 0, 80, 10, 2.5, 1.5, 1);
// 
// 			
// 
// 			result = api.DeleteChannelEx(proximity, game, server);
// 
// 		}
// 		
// 		char userBuff[512];
// 		sprintf (userBuff, "%s.%s", server.c_str(), name.c_str());
// 		
// 		
// 		char channelBuff[512];
// 		sprintf(channelBuff, "%s.%s.guild.%s", game.c_str(), server.c_str(), name.c_str());
// 	
// 		
// 
// 		printf("ChannelCommand user(%s) channel(%s)\n", userBuff, channelBuff);
// 
// 		printf("COMMAND_ADD_MODERATOR  ");
// 		result = api.ChannelCommandEx(userBuff, userBuff, channelBuff, COMMAND_ADD_MODERATOR, 0); 
// 
// 		
// 
// 		printf("COMMAND_DELETE_MODERATOR  ");
// 		result = api.ChannelCommandEx(userBuff, userBuff, channelBuff, COMMAND_DELETE_MODERATOR, 0); 
// 
// 		
// 
// 		printf("COMMAND_ADD_ACL  ");
// 		result = api.ChannelCommandEx(userBuff, userBuff, channelBuff, COMMAND_ADD_ACL, 0); 
// 
// 
// 		
// 
// 		printf("COMMAND_DELETE_ACL  ");
// 		result = api.ChannelCommandEx(userBuff, userBuff, channelBuff, COMMAND_DELETE_ACL, 0); 
// 		
// 		
// 
// 		result = api.DeleteChannelEx(guild, game, server);
// 
// 
// 		printf ("\nExpect it return a new channel.\n;");
// 		result = api.GetChannelV2Ex(guild, game, server, "Description space", "", 100, 0);
// 
// 		// delete again
// 		result = api.DeleteChannelEx(guild, game, server);
// 
// 
// 		//result = api.DeactivateVoiceAccount(name, game, server); 
// 		
// 		
// 		sprintf(numBuff, "Deleter.%d", y);
// 
// 		result = api.GetChannelEx(numBuff, game, server, "My Description", "firewall", 400, 1);
// 
// 		
// 
// 		result = api.GetAllChannelsEx(); 
// 
// 		
// 
// 		printf("Deleting Channel %s\n", numBuff);
// 
// 		result = api.DeleteChannelEx(numBuff, game, server);
// 		
// 		
// 
// 		result = api.GetAllChannelsEx(); 
// 
// 
// 		printf("Login user: %s\n", name.c_str());
// 
// 		// first one adds
// 		result = api.GetAccountEx(name, game, server, 1111, 0);
// 
// 		
// 
// 		result = api.SetBanStatusEx(1111, BAN); 
// 
// 		

		printf("Login after ban -- user: %s\n", name.c_str());

		// first one adds
		result = api.GetAccountEx(name, game, server, 1111, 9999);

		

		result = api.SetBanStatusEx(1111, UNBAN); 

		

		printf("Login after UNban -- user: %s\n", name.c_str());

		// first one adds
		result = api.GetAccountEx(name, game, server, 1111, 9999);


		result = api.AddCharacterChannelEx(1111, 9999, name,  server, game, "Guild", 
										   "Guilds for Geldings", "sony1234", "addr", "en_US");


		result = api.GetCharacterChannelEx(1111, name, server, game);

// 		result = api.UpdateCharacterChannelEx(1111, 9999, name,  server, game, "Guild", 
// 			"Guilds for Geldings", api.m_channelID, "secret", api.m_channelURI, "en_US");
// 		
		result = api.AddCharacterChannelEx(1111, 9999, name,  server, game, "Guild", 
			"Guilds for Geldings", "secret",  "addr",  "en_US");

		result = api.GetCharacterChannelEx(1111, name, server, game);

// 		result = api.UpdateCharacterChannelEx(1111, 9999, name,  server, game, "PartyCentral", 
// 			"Wills for Weldings", api.m_channelID, "secret", api.m_channelURI, "en_US");
// 		
		result = api.AddCharacterChannelEx(1111, 9999, name,  server, game, "PartyCentral", 
			"Wills for Weldings", "secret",  "addr",  "en_US");

		result = api.GetCharacterChannelEx(1111, name, server, game);

		result = api.RemoveCharacterChannelEx(1111, 9999, name,  server, game, "Guild");

		result = api.GetCharacterChannelEx(1111, name, server, game);

		//////////////////////////////////////////////////////////////////////////////////////////////
		printf("Press Enter to go again, x to exit:");
		c = getchar();
		y++;

		count++;
	}  while (c != 'x');

	if (cmdLine.HasSwitch(INTERACTIVE_SWITCH))
	{
		printf("Press Enter to exit:");
		getchar();
	}
	return 0;
}

