#include <stdio.h>
#pragma warning (disable: 4786)
#include "Client.h"

#ifdef WIN32
#include <winsock.h>
#include <conio.h>
#endif

using namespace CTService;

unsigned						openRequests = 0;
CTServiceAPI					*waitclient = nullptr;
std::map<unsigned, unsigned>	m_tests;
std::string						game_code;

//--------------------------------------------
void wait()
//--------------------------------------------
{
	while (openRequests > 0 || m_tests.size())
	{
		waitclient->process();
		Base::sleep(1);
#if TEST_MULTIPLE_SERVERS == 1
		for(std::map<unsigned, unsigned>::iterator iter = m_tests.begin(); iter != m_tests.end(); iter++)
		{
			if ((unsigned)time(0) > (*iter).second)
			{
				unsigned strack = (*iter).first;
				printf("\nProcessing send for server track = %d (%d remain)", strack, m_tests.size() - 1);
				waitclient->replyTest(strack, strack * 100, "Test");
				openRequests++;
				m_tests.erase(strack);
				break;
			}
		}
#endif
	}
}

//--------------------------------------------
int main(int argc, char *argv[])
//--------------------------------------------
{
	if (argc < 2)
	{
		printf("\nParams required: <game code>\n");
		while(1);
	}
	game_code = argv[1];

	Client client("localhost:2000", game_code.c_str());
	waitclient = &client;
	printf("\nCreated Client object, game = %s", game_code.c_str());

    while (!client.m_connected)
    {
        client.process();
        Base::sleep(1);
    }

#if ACT_AS_SERVER == 1
	printf("\n\nListening for requests from server....\n");
	while (1)
	{
        client.process();
        Base::sleep(1);
	}
#endif

    //begin the testing !!
    printf("\nCommencing Tests...\n");

#if TEST_FUNCTIONS == 1
	std::string testString = "FIP1";
	client.requestTest(testString.c_str(), GING_TEST_STATUS, (void *)"Test");
    openRequests++;
	wait();
	client.requestTest(testString.c_str(), GING_TEST_MOVE, (void *)"Test");
    openRequests++;
	wait();
	client.requestTest(testString.c_str(), GING_TEST_VALIDATE, (void *)"Test");
    openRequests++;
	wait();
	client.requestTest(testString.c_str(), GING_TEST_CHARACTER, (void *)"Test");
    openRequests++;
	wait();
	client.requestTest(testString.c_str(), GING_TEST_SERVER, (void *)"Test");
    openRequests++;
	wait();
	client.requestTest(testString.c_str(), GING_TEST_DESTSERVER, (void *)"Test");
    openRequests++;
	wait();
#endif

#if TEST_MULTIPLE_SERVERS == 1
	std::string testString = "FIP1";
	client.requestTest(testString.c_str(), 10, (void *)"Test");
    openRequests++;
	testString = "FIP2";
	client.requestTest(testString.c_str(), 100, (void *)"Test");
    openRequests++;
	testString = "FIP3";
	client.requestTest(testString.c_str(), 1000, (void *)"Test");
    openRequests++;
	testString = "FIP4";
	client.requestTest(testString.c_str(), 10000, (void *)"Test");
    openRequests++;

	testString = "FIP2";
	client.requestTest(testString.c_str(), 10, (void *)"Test");
    openRequests++;
	testString = "FIP3";
	client.requestTest(testString.c_str(), 100, (void *)"Test");
    openRequests++;
	testString = "FIP4";
	client.requestTest(testString.c_str(), 1000, (void *)"Test");
    openRequests++;
	testString = "FIP1";
	client.requestTest(testString.c_str(), 10000, (void *)"Test");
    openRequests++;

	testString = "FIP3";
	client.requestTest(testString.c_str(), 10, (void *)"Test");
    openRequests++;
	testString = "FIP4";
	client.requestTest(testString.c_str(), 100, (void *)"Test");
    openRequests++;
	testString = "FIP1";
	client.requestTest(testString.c_str(), 1000, (void *)"Test");
    openRequests++;
	testString = "FIP2";
	client.requestTest(testString.c_str(), 10000, (void *)"Test");
    openRequests++;

	testString = "FIP4";
	client.requestTest(testString.c_str(), 10, (void *)"Test");
    openRequests++;
	testString = "FIP1";
	client.requestTest(testString.c_str(), 100, (void *)"Test");
    openRequests++;
	testString = "FIP2";
	client.requestTest(testString.c_str(), 1000, (void *)"Test");
    openRequests++;
	testString = "FIP3";
	client.requestTest(testString.c_str(), 10000, (void *)"Test");
    openRequests++;
	wait();
#endif

	wait();
    return 0;
}
