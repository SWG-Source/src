//-----------------------------------------------------------------
//
// CSAssist Module
//
// API Test Program
//
// Author: Graeme Ing (ging@soe.sony.com x3130)
//
// Copyright (c) 2002 Sony Online Interactive inc.
//
// This program simply demonstrates some calls to the
// CSAssist API. See main()
//
//-----------------------------------------------------------------


#include "test.h"
#include "Unicode/UnicodeUtils.h"

using namespace CSAssist;
using namespace Plat_Unicode;
#include <signal.h>
#include <iostream>
#include <time.h>
#include <string.h>
#ifdef WIN32
#include <conio.h>
#endif

using namespace std;

apiTest *api;
int received = 0;
int submitted = 0;
CSAssistGameAPIUID		gUID=623259672; // csatest account
CSAssistGameAPITicketID	globalTicketID=0;
bool ready = false;
Plat_Unicode::String globalArticleID;

//----------------------------------------------
CSAssistUnicodeChar *get_c_str(Plat_Unicode::String s)
// This replaces c_str() by allocating a new buffer, erasing it, and then
// using the non-nullptr terminated data() to copy the data into the buffer.
//----------------------------------------------
{
	unsigned length = s.size() + 1;
	CSAssistUnicodeChar *temp = new CSAssistUnicodeChar[length];
	memset(temp, 0, length * sizeof(CSAssistUnicodeChar));
	memcpy(temp, s.data(), (length - 1) * sizeof(CSAssistUnicodeChar));
	return temp;
}

//----------------------------------------------
CSAssistUnicodeChar *copy_c_str(Plat_Unicode::String s, CSAssistUnicodeChar *dest)
//----------------------------------------------
{
	unsigned length = s.size() + 1;
	memset(dest, 0, length * sizeof(CSAssistUnicodeChar));
	memcpy(dest, s.data(), (length - 1) * sizeof(CSAssistUnicodeChar));
	return dest;
}

//---------------------------------------------
//
// The routines that follow display basic objects
//
//---------------------------------------------

//---------------------------------------------
void DisplayTicket(const CSAssistGameAPITicket *t)
//---------------------------------------------
{
	if (t != 0)
	{
		char date1[64], date2[64];
		strcpy(date1,ctime(&t->createDate));
		Plat_Unicode::String game = t->game;
		Plat_Unicode::String server = t->server;
		Plat_Unicode::String character = t->character;
		Plat_Unicode::String language = t->language;
		Plat_Unicode::String location = t->location;
		Plat_Unicode::String details = t->details;
//		std::cout << wideToNarrow(character) << " " << t->modifiedDate << " " << date2;
		std::cout << "***Ticket: " << t->ticketID << "\n   Created: " << date1;
		strcpy(date2,ctime(&t->modifiedDate));
		std::cout << "   Modified: " << date2 << "   Game/Server: " << wideToNarrow(game) << ", " << wideToNarrow(server);
		std::cout << "\n   Character: " << wideToNarrow(character) << "\n   Status: " << t->status << "," << t->bugstatus;
		std::cout << "\n   UID: " << t->uid << "\n   Category: ";
		for (short i=0; i < CSASSIST_NUM_CATEGORIES; i++)
			std::cout << t->category[i] << ",";
		std::cout << "\n   Language: " << wideToNarrow(language);
		std::cout << "\n   Location: " << wideToNarrow(location);
		std::cout << "\n   IsRead: " << t->isRead << "\n   ";
		std::cout << wideToNarrow(details) << "\n***\n";
	}
	else
		std::cout << "***Ticket IS nullptr! ***\n";
}

//---------------------------------------------
void DisplayComment(const CSAssistGameAPITicketComment *t)
//---------------------------------------------
{
	if (t != 0)
	{
		Plat_Unicode::String name = t->name;
		Plat_Unicode::String comment = t->comment;
		std::cout << "***Comment: Comment ID: " << t->commentID << " Ticket ID: " << t->ticketID << " UID: " << t->uid;
		std::cout << " Name: " << wideToNarrow(name) << " Type: " << t->type << " Modified: " << ctime(&t->modifiedDate);
		std::cout << "\n   " << wideToNarrow(comment) << "\n";
	}
	else
		std::cout << "***Comment IS nullptr! ***\n";
}

//---------------------------------------------
void DisplayDocumentHeader(const CSAssistGameAPIDocumentHeader *doc)
//---------------------------------------------
{
	if (doc != 0)
	{
		char date2[32];
		strcpy(date2,ctime(&doc->modifiedDate));
		Plat_Unicode::String game = doc->game;
		Plat_Unicode::String language = doc->language;
		Plat_Unicode::String title = doc->title;
		Plat_Unicode::String version = doc->version;
		std::cout << "***Document Header:  ID: " << doc->id << ", Game: " << wideToNarrow(game);
		std::cout << ", Lang: " << wideToNarrow(language);
		std::cout << ", Title: " << wideToNarrow(title);
		std::cout << ", Version: " << wideToNarrow(version);
		std::cout << ", Modified: " << date2 << "\n";
	}	
	else
		std::cout << "***Document Header IS nullptr! ***\n";
}

//---------------------------------------------
void DisplaySearchResult(const CSAssistGameAPISearchResult *doc)
//---------------------------------------------
{
	if (doc != 0)
	{
		Plat_Unicode::String id = doc->idstring;
		globalArticleID = id;
		Plat_Unicode::String title = doc->title;
		std::cout << "ID: " << wideToNarrow(id) << ", Match: " << doc->matchPercent;
		std::cout << ", Title: " << wideToNarrow(title) << "\n";
	}	
	else
		std::cout << "***Search Result IS nullptr! ***\n";
}


//---------------------------------------------
apiTest::apiTest(const char *hostList, const unsigned timeout = 0, const unsigned flags = 0)
: CSAssistGameAPI(hostList, timeout, flags)
//---------------------------------------------
{
}

apiTest::apiTest(const char *host, const unsigned port, const unsigned timeout, const unsigned flags)
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
//
// The routines that follow override the API
// callbacks. You only need to override those
// that you are interested in.
// Default API behavior on the callbacks is to
// delete the message and continue.
//
//---------------------------------------------

//---------------------------------------------
void apiTest::OnConnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
	std::cout << "Connect: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
	if (result == CSASSIST_RESULT_SUCCESS)
	{
		std::cerr <<"Connected to host("<<api->GetConnectedHost()<<") port("<<api->GetConnectedPort()<<")"<<endl;
		ready = true;
	}
	else
	{

	}
}

//---------------------------------------------
void apiTest::OnConnectRejectedCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
	std::cerr << "Connect REJECTED("<<GetConnectedHost() <<","<<GetConnectedPort()<<"): Track: " << track 
		<< " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
}

//---------------------------------------------
void apiTest::OnDisconnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
	std::cerr << "DisConnect: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
	received++;
}

//---------------------------------------------
void apiTest::OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
	std::cout << "Register: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
	received++;
}

//---------------------------------------------
void apiTest::OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
	std::cout << "UnRegister: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
	received++;
}

//---------------------------------------------
void apiTest::OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets)
//---------------------------------------------
{
	std::cout << "NewActivity: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", New(" << NewActivityFlag;
	std::cout << "), HasTickets(" << HasTickets << ")\n";
	received++;
}

//---------------------------------------------
void apiTest::OnGetIssueHierarchy(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML hierarchyBody, const unsigned modifyDate)
//---------------------------------------------
{
	received++;
	Plat_Unicode::String body = hierarchyBody;
	std::cout << "Hierarchy: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData;
	std::cout << ", XML(" << wideToNarrow(body).c_str();
	std::cout << "), ModifyDate(" << ctime((const time_t *)(&modifyDate)) << ")";
}

//---------------------------------------------
void apiTest::OnCreateTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------------------------
{
	std::cout << "Create Ticket: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", TicketID(" << ticket << ")\n";
	received++;
	globalTicketID = ticket;
}
//---------------------------------------------
void apiTest::OnAppendTicketComment(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------------------------
{
	std::cout << "Append Comment: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", TicketID(" << ticket << ")\n";
	received++;
}

//---------------------------------------------
void apiTest::OnGetTicketByID(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicket *ticketBody)
//---------------------------------------------
{
	std::cout << "Get Ticket ID: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
	if (result == CSASSIST_RESULT_SUCCESS)
		DisplayTicket(ticketBody);
	received++;
}

//---------------------------------------------
void apiTest::OnGetTicketComments(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketComment *comments, const unsigned numberRead)
//---------------------------------------------
{
	std::cout << "Get Ticket Comments: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", numberRead(" << numberRead << ")\n";
	if (comments == 0)
	{
		std::cout << "***Returned no data array***\n";
	}
	else
	{
		const CSAssistGameAPITicketComment *pt = comments;
		for (unsigned i=0; i < numberRead; i++)
		{
			DisplayComment(pt);
			pt++;
		}
	}
	received++;
}

//---------------------------------------------
void apiTest::OnGetTicketByCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NumberReturned, const unsigned TotalNumber, const CSAssistGameAPITicket *tickets)
//---------------------------------------------
{
	std::cout << "Get Tickets Char: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", numberRead(" << NumberReturned;
	std::cout << ") totalNumber(" << TotalNumber << ")\n";
	if (tickets == 0)
	{
		std::cout << "***Returned no data array***\n";
	}
	else
	{
		const CSAssistGameAPITicket *pt = tickets;
		for (unsigned i=0; i < NumberReturned; i++)
		{
			DisplayTicket(pt);
			pt++;
		}
	}
	received++;
}

//---------------------------------------------
void apiTest::OnTicketChange(const CSAssistGameAPITicketID ticketID, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
//---------------------------------------------
{
	Plat_Unicode::String charry = character;
	std::cout << "Ticket Change: Ticket(" << ticketID << "), uid(" << uid << "), character(" << wideToNarrow(charry) << ")\n";
}

//---------------------------------------------
void apiTest::OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------------------------
{
	std::cout << "Mark Read: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << "\n";
	received++;
}

//---------------------------------------------
void apiTest::OnCancelTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------------------------
{
	std::cout << "Cancel Ticket: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", TicketID: " << ticket << "\n";
	received++;
}

//---------------------------------------------
void apiTest::OnGetTicketCommentsCount(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket, const unsigned count)
//---------------------------------------------
{
	std::cout << "Comment Count: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", TicketID(" << ticket << "), Count: " << count << "\n";
	received++;
}

//---------------------------------------------
void apiTest::OnRequestGameLocation(const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID CSRUID)
//---------------------------------------------
{
	Plat_Unicode::String charry = character;
	Plat_Unicode::String loc = narrowToWide("-debug character=qeynos hills");
	CSAssistUnicodeChar *rawCharry = get_c_str(charry);
	CSAssistUnicodeChar *rawLoc = get_c_str(loc);
	std::cout << "Request Game Location: Source Track(" << sourceTrack << "), uid(" << uid << "), character(";
	std::cout << wideToNarrow(charry) << "), CSR UID(" << CSRUID << ")\n";
	api->replyGameLocation(nullptr, sourceTrack, uid, rawCharry, CSRUID, rawLoc);
	delete [] rawCharry;
	delete [] rawLoc;
}

//---------------------------------------------
void apiTest::OnGetDocumentList(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIDocumentHeader *docList, const unsigned numberRead)
//---------------------------------------------
{
	std::cout << "Get DocumentList: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", numberRead(" << numberRead << ")\n";
	if (docList == 0)
	{
		std::cout << "***Returned no data array***\n";
	}
	else
	{
		const CSAssistGameAPIDocumentHeader *pt = docList;
		for (unsigned i=0; i < numberRead; i++)
		{
			DisplayDocumentHeader(pt);
			pt++;
		}
	}
	received++;
}

//---------------------------------------------
void apiTest::OnGetDocument(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistUnicodeChar *documentBody)
//---------------------------------------------
{
	Plat_Unicode::String body = documentBody;
	std::cout << "Document: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", Text(";
	std::cout << wideToNarrow(body) << ")\n";
	received++;
}

//---------------------------------------------
void apiTest::OnGetTicketXMLBlock(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML XMLBody)
//---------------------------------------------
{
	Plat_Unicode::String xml = XMLBody;
	std::cout << "Ticket XML Block: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData;
	std::cout << ", XML(" << wideToNarrow(xml) << ")\n";
	received++;
}

//---------------------------------------------
void apiTest::OnSearchKB(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPISearchResult *results, const unsigned numberRead)
//---------------------------------------------
{
	std::cout << "Search KB: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData << ", numberRead(" << numberRead << ")\n";
	if (results == 0)
	{
		std::cout << "***Returned no data array***\n";
	}
	else
	{
		const CSAssistGameAPISearchResult *pt = results;
		for (unsigned i=0; i < numberRead; i++)
		{
			DisplaySearchResult(pt);
			pt++;
		}
	}
	received++;
}

//---------------------------------------------
void apiTest::OnGetKBArticle(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML articleBody)
//---------------------------------------------
{
	std::cout << "KB Article: Track: " << track << " - Result: " << getErrorString(result) << " UserData: " << (unsigned)userData;
	Plat_Unicode::String body = articleBody;
	std::cout << ", Text(" << wideToNarrow(body) << ")\n";
	received++;
}

//---------------------------------------------
void apiTest::OnIssueHierarchyChanged(const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language)
//---------------------------------------------
{
	Plat_Unicode::String ver = version;
	Plat_Unicode::String lang = language;
	std::cout << "Hierarchy Changed: " << "Version: " << wideToNarrow(ver) << ", Lang: " << wideToNarrow(lang) << "\n";
}

void sigh(int signal)
{
	cerr<<"Caught signal: "<< signal << endl;
}

//---------------------------------------------
int main(int argc, char **argv) 
//
// Here is some test code to demonstrate the API
//
// NOTE: The copy_c_str() and get_c_str() calls are simply "hacks"
// to replace the standard c_str() member function for unicode strings (Plat_Unicode::String)
// This is necessary because c_str() is broken under gcc 2.96 and will not compile with
// unicode strings :(
// In replace of these hacks and the associated "delete [] foo" code,
// simply use .c_str()
//
//---------------------------------------------
{
	//signal(SIGPIPE, sigh);
	printf("CSAssist test client\n");
//	int serverport = 3010;
	int serverport = 3019;
	char *host = "sdplatdev2";
	string gameStr("SWG");
	for (int i(0); i < argc; i++)
	{
		if (!strcmp(argv[i],"-host"))
			host = argv[i+1];
		else if(!strcmp(argv[i], "-port"))
			serverport = atoi(argv[i+1]);
		else if(!strcmp(argv[i], "-server"))
			gameStr = argv[i+1];
	}
	//api = new apiTest(host, serverport, 160, CSASSIST_APIFLAG_ASSUME_RECONNECT);
	//api = new apiTest(host, serverport, 160, CSASSIST_APIFLAG_ASSUME_RECONNECT|CSASSIST_APIFLAG_NO_REDIRECT);
	api = new apiTest("sdplatdev2:3016 ", 30, CSASSIST_APIFLAG_ASSUME_RECONNECT);
	std::cout << "Client API ceated.\n";
	//std::cout <<"Connected to host via("<<api->GetConnectedHost()<<") port("<<api->GetConnectedPort()<<")"<<endl;
	//api->setMaxPacketsPerSecond(40);

	//---------------------------------------------
	// test out some calls:
	// for a list of all those available, see
	// CSAssistgameapi.h
	//---------------------------------------------

	Plat_Unicode::String game = narrowToWide(gameStr);	//EQOA
	Plat_Unicode::String server = narrowToWide("CsatestServer");	// Test
	CSAssistUnicodeChar rawGame[16];
	CSAssistUnicodeChar rawServer[16];
	copy_c_str(game, rawGame);
	copy_c_str(server, rawServer);
	api->connectCSAssist((void *)6000, rawGame, rawServer);
	while (!ready)
		api->Update();

	//return 0;
	Plat_Unicode::String lang = narrowToWide("en");
	CSAssistUnicodeChar rawLanguage[16];
	copy_c_str(lang, rawLanguage);
/*	Plat_Unicode::String hversion = narrowToWide("test");	// test
	CSAssistUnicodeChar rawVersion[32];
	copy_c_str(hversion, rawVersion);
	api->requestGetIssueHierarchy((void *)1, rawVersion, rawLanguage);
	submitted++;
*/

/*	Plat_Unicode::String search = narrowToWide("jedi");
	CSAssistUnicodeChar rawSearch[64];
	copy_c_str(search, rawSearch);
	api->requestSearchKB((void *)1, rawSearch, rawLanguage);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}
	Plat_Unicode::String kbid = narrowToWide("soe603");
	CSAssistUnicodeChar rawkbid[64];
	copy_c_str(kbid, rawkbid);
	api->requestGetKBArticle((void *)1, rawkbid, rawLanguage);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}
*/


	api->setMaxPacketsPerSecond(200);

	Plat_Unicode::String testc;
	//unsigned i;
	unsigned j;
	//while (1)
	{
		CSAssistUnicodeChar rawChar[64];
		for (j=0; j < 100; j++)
		{
			char buff[100] = {0};
			char uidc[25] = {0};
			sprintf(buff,"TestChar%04d", j+1);
			testc = narrowToWide(buff);
			copy_c_str(testc, rawChar);
			sprintf(uidc, "%04d", j+1);
			unsigned uid = atoi(uidc);
			api->requestRegisterCharacter((void *)uid, uid, rawChar, 0);
			submitted++;
			//api->requestNewTicketActivity((void *)6002, uid, rawChar);
			//submitted++;
			//while (submitted > received)	
			{
				api->Update();
			}
		}
	/*	for (j=0; j < 20; j++)
		{
			for (i=0; i < 100; i++)
			{
				sprintf(buff,"TestChar%d%d", j+1, i+1);
				testc = narrowToWide(buff);
				copy_c_str(testc, rawChar);
				api->requestUnRegisterCharacter((void *)6001, (i+1)*(j+1), rawChar);
				submitted++;
			}
			//while (submitted > received)	
			//{
			//	api->Update();
			//}
		}
	*/
		api->Update();
		cerr<<"Waiting for register requests..."<<endl;
		while (submitted > received)	
		{
			api->Update();
		}
		cerr<<"done waiting for registers."<<endl;

	}
	
	// now wait for events
	std::cerr <<"Connected to host("<<api->GetConnectedHost()<<") port("<<api->GetConnectedPort()<<")"<<endl;
	while (1)
	{
		api->Update();
		Base::sleep(50);
	}
	

	Plat_Unicode::String character = narrowToWide("CSATestChar1");
	CSAssistUnicodeChar rawChar[64];
	copy_c_str(character, rawChar);
	api->requestRegisterCharacter((void *)6001, gUID, rawChar, 0);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}
	api->requestNewTicketActivity((void *)6002, gUID, rawChar);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}

	getchar();return 0;

	Plat_Unicode::String details = narrowToWide("CsaTest's petition text.");
	CSAssistUnicodeChar rawDetails[64];
	copy_c_str(details, rawDetails);
	Plat_Unicode::String xml = narrowToWide("CsaTest's XML text.");
	CSAssistUnicodeChar rawxml[64];
	copy_c_str(xml, rawxml);
	CSAssistGameAPITicket t;
	t.setUID(gUID);
	t.setGame(rawGame);
	t.setServer(rawServer);
	t.setCharacter(rawChar);
	t.setDetails(rawDetails);
	t.setLanguage(rawLanguage);
	t.setCategory(0, 12);
	t.setCategory(1, 2);
	//t.setBug();
	api->requestCreateTicket((void *)1, &t, rawxml, gUID);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}

//	api->requestNewTicketActivity((void *)6002, 99709401, 0);
//	submitted++;
//	while (submitted > received)	
//	{
//		api->Update();
//	}

/*	api->requestGetTicketByID((void *)6001, 156368, 1);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}
*/
//	api->requestGetTicketXMLBlock((void *)1, globalTicketID);
//	submitted++;
//	while (submitted > received)	
//	{
//		api->Update();
//	}

/*	Plat_Unicode::String comment = narrowToWide("Fippys closing comment");
	CSAssistUnicodeChar rawComment[64];
	copy_c_str(comment, rawComment);
	api->requestCancelTicket((void *)1, 267997, 99709401, rawComment);
	submitted++;
	while (submitted > received)	
	{
		api->Update();
	}
*/
#ifdef WIN32
	while (submitted > received || !kbhit())		// make sure everything is complete before disconnecting
#else
	while (submitted > received)		// make sure everything is complete before disconnecting
#endif
	{
		api->Update();
	}

	api->disconnectCSAssist((void *)6013);
	submitted++;

//	----- loop here until all sent messages have arrived -----
//	----- in your application, just call api->Update() at regular intervals -----

	while (submitted > received)
	{
		api->Update();
	}

	delete api;
	return(0);
}
