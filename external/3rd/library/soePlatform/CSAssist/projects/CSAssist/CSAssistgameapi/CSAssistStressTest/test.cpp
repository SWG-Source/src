#include "test.h"
#include "Base/Base.h"
#include "Unicode/UnicodeUtils.h"
#include "request.h"

#include <iostream>
#include <time.h>
#include <string.h>
#ifdef WIN32
#include <conio.h>
#endif
#include <queue>

//#define WARP_SPEED	true			// set this for BRIEF std::cout printing
#define AUTO_RUN	true

using namespace std;
using namespace CSAssist;
using namespace Plat_Unicode;

apiTest			*api;
int				received = 0;
int				submitted = 0;
unsigned		packetsRx = 0;
bool			ready = false;
bool			createdTicket = false;
bool			loginFailed = true;
unsigned		firstTicketID = 0;
unsigned		lastTicketID = 0;
unsigned		uid = 0;
unsigned		testUID = 623259672;//99709401;
unsigned		track(0);
int numberFunctionsToRun(0);
bool doBug(false);

std::queue<unsigned>	register_list;
Plat_Unicode::String	game;
Plat_Unicode::String	server;
Plat_Unicode::String	character;
Plat_Unicode::String	lang;
Plat_Unicode::String	comment;
Plat_Unicode::String	id;
Plat_Unicode::String	search;
Plat_Unicode::String	hierarchy = narrowToWide("test");

unsigned	numberFunctions = 16;
unsigned	functionCount[16];
unsigned	functionID[] = 
{
	CSASSIST_CALL_NEWTICKETACTIVITY,			// 2
	CSASSIST_CALL_REGISTERCHARACTER,			// 3
	CSASSIST_CALL_GETISSUEHIERARCHY,			// 6
	CSASSIST_CALL_CREATETICKET,					// 7
	CSASSIST_CALL_APPENDCOMMENT,				// 8
	CSASSIST_CALL_GETTICKETBYID,				// 9
	CSASSIST_CALL_GETTICKETCOMMENTS,			// 10
	CSASSIST_CALL_GETTICKET,					// 11
	CSASSIST_CALL_MARKREAD,						// 12
	CSASSIST_CALL_CANCELTICKET,					// 13
	CSASSIST_CALL_COMMENTCOUNT,					// 14
	CSASSIST_CALL_GETDOCUMENTLIST,				// 16
	CSASSIST_CALL_GETDOCUMENT,					// 17
	CSASSIST_CALL_GETTICKETXMLBLOCK,			// 18
	CSASSIST_CALL_GETKBARTICLE,					// 19
	CSASSIST_CALL_SEARCHKB						// 20
};
char *functionName[] = 
{
	"CSASSIST_CALL_NEWTICKETACTIVITY",			// 2
	"CSASSIST_CALL_REGISTERCHARACTER",			// 3
	"CSASSIST_CALL_GETISSUEHIERARCHY",			// 6
	"CSASSIST_CALL_CREATETICKET",				// 7
	"CSASSIST_CALL_APPENDCOMMENT",				// 8
	"CSASSIST_CALL_GETTICKETBYID",				// 9
	"CSASSIST_CALL_GETTICKETCOMMENTS",			// 10
	"CSASSIST_CALL_GETTICKET",					// 11
	"CSASSIST_CALL_MARKREAD",					// 12
	"CSASSIST_CALL_CANCELTICKET",				// 13
	"CSASSIST_CALL_COMMENTCOUNT",				// 14
	"CSASSIST_CALL_GETDOCUMENTLIST",			// 16
	"CSASSIST_CALL_GETDOCUMENT",				// 17
	"CSASSIST_CALL_GETTICKETXMLBLOCK",			// 18
	"CSASSIST_CALL_GETKBARTICLE",				// 19
	"CSASSIST_CALL_SEARCHKB"					// 20
};

void ResetGlobals()
{
	received = 0;
	submitted = 0;
	packetsRx = 0;
	ready = false;
	createdTicket = false;
	loginFailed = true;
	firstTicketID = 0;
	lastTicketID = 0;
	uid = 0;

	float max = 500.0;
	//srand(time(0));
	//numberFunctionsToRun = 2+(int) (max*rand()/(RAND_MAX+1.0));
	numberFunctionsToRun = 2000;
}

//---------------------------------------------
apiTest::apiTest(const char *host, const unsigned port, const unsigned timeout = 0, const unsigned flags = 0)
: CSAssistGameAPI(host, port, timeout, flags)
//---------------------------------------------
{
}

//---------------------------------------------
apiTest::~apiTest()
//---------------------------------------------
{
}

//---------------------------------------------
void apiTest::OnConnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nConnect: " << result;
#endif
	if (result == CSASSIST_RESULT_SUCCESS)
		ready = true;
	else
		loginFailed = true;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnDisconnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nDisConnect: " << result;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nRegister: " << result;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nUnRegister: " << result;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nMark Read: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnCancelTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nUCancel: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnGetTicketCommentsCount(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket, const unsigned count)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nComment Count: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnRequestGameLocation(const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID CSRUID)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nGame Location: " << uid;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnGetDocumentList(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIDocumentHeader *docList, const unsigned numberRead)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nDoc List: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnGetDocument(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistUnicodeChar *documentBody)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nDocument: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnGetTicketXMLBlock(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML XMLBody)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nGet XML Block: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnSearchKB(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPISearchResult *results, const unsigned numberRead)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nSearch KB: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnGetKBArticle(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML articleBody)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nGet KB Article: " << result;
#endif
	received++;
	packetsRx++;
}
//---------------------------------------------
void apiTest::OnIssueHierarchyChanged(const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nHierarchy Changed";
#endif
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nNewActivity: " << result << " " << HasTickets;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnGetIssueHierarchy(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML hierarchyBody, const unsigned modifyDate)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nHierarchy: " << result;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnCreateTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nCreate Ticket: " << result << " "<<flush;
#endif
	if (firstTicketID == 0)
		firstTicketID = ticket;
	if (ticket > lastTicketID)
		lastTicketID = ticket;
	received++;
	packetsRx++;
	createdTicket = true;
}
//---------------------------------------------
void apiTest::OnAppendTicketComment(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nAppend Comment: " << result;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnGetTicketByID(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicket *ticketBody)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nGet Ticket: " << result;
#endif
	received++;
	packetsRx++;
}

//---------------------------------------------
void apiTest::OnGetTicketComments(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketComment *comments, const unsigned numberRead)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nGet Ticket Comments: " << result;
	if (comments == 0)
		std::cout << "\nNo Comment data";
#endif	
	received++;
	packetsRx += numberRead;
}

//---------------------------------------------
void apiTest::OnGetTicketByCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NumberReturned, const unsigned TotalNumber, const CSAssistGameAPITicket *tickets)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nGet Ticket: " << result;
	if (tickets == 0)
		std::cout << "\nNo Ticket data";
#endif	
	received++;
	packetsRx += NumberReturned;
}

//---------------------------------------------
void apiTest::OnTicketChange(const CSAssistGameAPITicketID ticketID, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
//---------------------------------------------
{
#ifndef WARP_SPEED
	std::cout << "\nTicket Change: " << ticketID;
#endif
	packetsRx++;
}

//---------------------------------------------
void createTicket()
//---------------------------------------------
{
	CSAssistGameAPITicket t;
	t.setUID(testUID);
	t.setGame(game.c_str());
	t.setServer(server.c_str());
	t.setCharacter(character.c_str());
	t.setCategory(0,4000);
	t.setCategory(1,4001);
	t.setCategory(2,2);
	t.setCategory(3,1);
	if (doBug) t.setBug();
	Plat_Unicode::String details  = narrowToWide("Stress Test Ticket details here.");
	Plat_Unicode::String location = narrowToWide("Stress Test Location");
	Plat_Unicode::String xml	  = narrowToWide("Stress Test XML here");
	t.setDetails(details.c_str());
	t.setLocation(location.c_str());
	track = api->requestCreateTicket(nullptr, &t, nullptr, t.uid);
	//track = api->requestCreateTicket(nullptr, &t, xml.c_str(), t.uid);
	submitted++;

	printf("creating ticket\n");
	createdTicket = false;
	if (firstTicketID ==0)			// if no tickets created yet, wait for one
	{
		while (!createdTicket)		// wait for create ticket to return, so we know ticket number we created
		{
			Base::sleep(50);
			api->Update();
		}
	}
}

//---------------------------------------------
unsigned randomTicketID()
//---------------------------------------------
{
	if (firstTicketID == 0)
		createTicket();				// create at least one ticket first

	unsigned r = firstTicketID;
	if (r != lastTicketID)
		r += rand() % (lastTicketID - firstTicketID);
#ifndef WARP_SPEED
	printf(" (%d) ", r);
#endif
	return r;
}

//-----------------------------------------------
std::string GetHostName(unsigned port)
//-----------------------------------------------
{
	const short hostLen(512);
	char hostName[hostLen] = {0};
	char hostAndPort[hostLen+10] = {0};
	std::string serverID;
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(1,1), &wsaData);
#endif
	if (gethostname(hostName, hostLen) < 0)
	{
		// Bad error - the serverID must be unique between CSA servers.
		// Attempt to use IP Address ?. // TODO:rlsmith
		sprintf(hostName,"localhost-%u", port);
	}
//#endif
	std::string shortName = hostName;
	unsigned dot = shortName.find_first_of(".");
	if (dot != std::string::npos)
	{
		shortName = shortName.substr(0,dot);
	}
#ifdef WIN32
	_snprintf(hostAndPort, hostLen+10, "%s", shortName.c_str());
#else
	snprintf(hostAndPort, hostLen+10, "%s", shortName.c_str());
#endif
	return (std::string)hostAndPort;
}

//---------------------------------------------
int main(int argc, char **argv) 
//---------------------------------------------
{
	unsigned i = 0;
	unsigned tid = 0;

	// ----- validate and strip command line parameters -----

	char *serverhost;
	int serverport;
	char *gamep;
	char *gameserver;

#ifdef AUTO_RUN
	ResetGlobals();
	serverhost = "localhost";
	serverport = 3009;
	if (argc > 1) 
		gamep = argv[1];
	else
		gamep = "PS";
	if (argc> 2  && !strcmp(argv[2],"bug")) doBug=true;
	std::string gs = GetHostName(numberFunctionsToRun);
	gameserver = (char*)gs.c_str();
#else
	if (argc < 6)
	{
		std::cout << "CSAssistStressTest Game Version: Syntax:\nCSAssistStressTest <hostname> <port> <number_funcs_to_run> <game> <gameserver>\n";
		return 0;
	}
	serverhost = argv[1];
	serverport = atoi(argv[2]);
	numberFunctionsToRun = atoi(argv[3]);
	gamep = argv[4];
	gameserver = argv[5];

	if (serverhost == nullptr || strlen(serverhost) == 0)
	{
		std::cout << "Missing hostname!\n";
		return 0;
	}
	if (serverport < 3000 || serverport > 3020)
	{
		std::cout << "Missing or invalid serverport!\n";
		return 0;
	}
	if (numberFunctionsToRun <= 0)
	{
		std::cout << "Missing or invalid number of functions to run!\n";
		return 0;
	}
	if (gamep == nullptr || strlen(gamep) == 0)
	{
		std::cout << "Missing game name!\n";
		return 0;
	}
	if (gameserver == nullptr || strlen(gameserver) == 0)
	{
		std::cout << "Missing game server name!\n";
		return 0;
	}
#endif

	game = narrowToWide(gamep);
	server = narrowToWide(gameserver);
	character = narrowToWide("Sabinia");
	
	// ----- connect to API -----
while(1)
{
	ResetGlobals();
	unsigned updateDelay(100);
	unsigned loginWait(500);
	unsigned loginAttempts(0);
	//api = new apiTest(serverhost, serverport, 60, CSASSIST_APIFLAG_ASSUME_RECONNECT);		//, 0, CSASSIST_APIFLAG_ASSUME_RECONNECT);
	//api->connectCSAssist(nullptr, game.c_str(), server.c_str());
	//while (!ready)
	//	api->Update();

	cerr<<"numberFunctionsToRun="<< numberFunctionsToRun <<endl;
	while (!ready)
	{
		if (loginFailed)
		{
			// Here we try a different set of credintials...if we have a set.
			if (api) delete api;
			//api = new apiTest(serverhost, serverport, 60, CSASSIST_APIFLAG_ASSUME_RECONNECT);
			api = new apiTest(serverhost, serverport, 60, CSASSIST_APIFLAG_ASSUME_RECONNECT|CSASSIST_APIFLAG_NO_REDIRECT);
			track = api->connectCSAssist(nullptr, game.c_str(), server.c_str());
			loginFailed = false;
			std::cout <<"Trying to connect..."<<endl;
			Base::sleep(loginWait+=500);
			if (loginWait > 5000) loginWait=500;
		}
		if (loginAttempts++ > 30) 
		{
			loginAttempts = 0;
			loginFailed = true;
			delete api;
			api = nullptr;
			continue;
		}
		std::cout <<"Waiting for login: track:" << track<<" ..."<<endl;
		Base::sleep(updateDelay);
		api->Update();
	}

	for (i=0; i < numberFunctions; i++)
		functionCount[i] = 0;

	createTicket();									// works better if we do this before timing starts

	time_t now = time(0);
	srand(now);

	// ----- submit random packets -----

	for (i=0; i < (unsigned)numberFunctionsToRun; i++)
	{
		//unsigned randomFunction = 3; // to generate dummy tickets use this line
		unsigned randomFunction = 3;//rand() % numberFunctions;
		functionCount[randomFunction]++;
#ifndef WARP_SPEED
		std::cout << " " << i << " ";
#endif
		std::cout << " calling: "<<functionName[randomFunction] << ": " << functionCount[randomFunction] << endl;
		track = 0;
		switch (functionID[randomFunction])
		{
			case CSASSIST_CALL_NEWTICKETACTIVITY:	
								character = narrowToWide("Fippy");
								if ((rand() % 10) < 5)
									track = api->requestNewTicketActivity(nullptr, testUID, 0);
								else
									track = api->requestNewTicketActivity(nullptr, testUID, character.c_str());
								submitted++;
								
								break;
			case CSASSIST_CALL_REGISTERCHARACTER:			
								uid = rand();
								track = api->requestRegisterCharacter(nullptr, uid, 0, 0);
								register_list.push(uid);
								submitted++;
								break;
			case CSASSIST_CALL_GETISSUEHIERARCHY:			
								lang = narrowToWide("en");
								track = api->requestGetIssueHierarchy(nullptr, hierarchy.c_str(), lang.c_str());
								submitted++;
								break;
			case CSASSIST_CALL_CREATETICKET:					
								createTicket();
								break;
			case CSASSIST_CALL_APPENDCOMMENT:				
								comment = narrowToWide("Unicode comment by player.");
								tid = randomTicketID();
								track = api->requestAppendTicketComment(nullptr, tid, testUID, character.c_str(), comment.c_str());
								submitted++;
								break;
			case CSASSIST_CALL_GETTICKETBYID:				
								tid = randomTicketID();
								track = api->requestGetTicketByID(nullptr, tid, 1);
								submitted++;
								break;
			case CSASSIST_CALL_GETTICKETCOMMENTS:			
								tid = randomTicketID();
								track = api->requestGetTicketComments(nullptr, tid, 0, (rand() % 100)+1, CSASSIST_OFFSET_START);
								submitted++;
								break;
			case CSASSIST_CALL_GETTICKET:					
								track = api->requestGetTicketByCharacter(nullptr, testUID, character.c_str(), 0, (rand() % 100)+1, 1);
								submitted++;
								break;
			case CSASSIST_CALL_MARKREAD:						
								tid = randomTicketID();
								track = api->requestMarkTicketRead(nullptr, tid);
								submitted++;
								break;
			case CSASSIST_CALL_CANCELTICKET:					
								if (firstTicketID != 0)
								{
									comment = narrowToWide("Ticket closed by player");
									track = api->requestCancelTicket(nullptr, firstTicketID, testUID, comment.c_str());
									if (++firstTicketID > lastTicketID)
									{
										firstTicketID = 0;
										lastTicketID = 0;
									}
									submitted++;
								}
								break;
			case CSASSIST_CALL_COMMENTCOUNT:				
								tid = randomTicketID();
								track = api->requestGetTicketCommentsCount(nullptr, tid);
								submitted++;
								break;
			case CSASSIST_CALL_GETDOCUMENTLIST:				
//								lang = narrowToWide("en");
//								track = api->requestGetDocumentList(nullptr, hierarchy.c_str(), lang.c_str());
//								submitted++;
								break;
			case CSASSIST_CALL_GETDOCUMENT:					
//								track = api->requestGetDocument(nullptr, 1);
//								submitted++;
								break;
			case CSASSIST_CALL_GETTICKETXMLBLOCK:			
								tid = randomTicketID();
								track = api->requestGetTicketXMLBlock(nullptr, tid);
								submitted++;
								break;
			case CSASSIST_CALL_GETKBARTICLE:		
								/*id = narrowToWide("soe1401");
								lang = narrowToWide("en");
								track = api->requestGetKBArticle(nullptr, id.c_str(), lang.c_str());
								submitted++;
								*/break;
			case CSASSIST_CALL_SEARCHKB:
								/*Plat_Unicode::String searchStr = narrowToWide("video drivers");
								lang = narrowToWide("en");
								track = api->requestSearchKB(nullptr, searchStr.c_str(), lang.c_str());
								submitted++;
								*/break;
		}
		Base::sleep(5);
		//api->Update();
		if (track)
			std::cout << "called: "<< functionName[randomFunction] << ": " << functionCount[randomFunction] << " track:" <<track<< endl;
		if (submitted - received > 50)
		{
			while (submitted > received)
			{
				Base::sleep(50);
				api->Update();
			}
		}
	}
 
	// ----- Wait for final packet -----
	cout<<"Waiting for last packet..."<<endl;
	while (submitted > received)		// make sure everything is complete before disconnecting
	{
		Base::sleep(50);
		api->Update();
	}

	// ----- unregister any registered characters ----

	while (register_list.size())
	{
		uid = register_list.front();
		register_list.pop();
		api->requestUnRegisterCharacter(nullptr, uid, 0);
		submitted++;
	}
	cout<<"Waiting for unregisters..."<<endl;
	while (submitted > received)		// make sure everything is complete before disconnecting
	{
		Base::sleep(50);
		api->Update();
	}

	// ----- and clean up -----

	time_t elapsed = time(0) - now;
	if (elapsed == 0)
		elapsed = 1;
	cout<<"Going to disconnect now."<<endl;
	api->disconnectCSAssist(nullptr);
	submitted++;

	while (submitted > received)
	{
		api->Update();
	}

	cout<<"Going to delete API now."<<endl;
	delete api;
	api = nullptr;
	// ----- report findings -----

	for (i=0; i < numberFunctions; i++)
		std::cout << "\n" << functionName[i] << ": " << functionCount[i];

	std::cout << "\n\nSubmitted: " << submitted << "\nReceived: " << received << "\nData Packets Rx: " << packetsRx;
	std::cout << "\nElapsed time = " << elapsed << " seconds.\n" << (received / elapsed) << " req/sec.\n" << (packetsRx / elapsed) << " pkts/sec.\n";
//	getchar();
} //while(1)
	return(0);
}
