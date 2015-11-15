#include "TestClient.h"
#include <string>
#include <Base/Base.h>

bool g_callbackRecieved;

unsigned g_numSent;
unsigned g_numRecvd;

int main(int argc, char **argv)
{
    g_callbackRecieved = false;
    g_numSent=0;
    g_numRecvd=0;

    TestClient *client = new TestClient(std::string("localhost"), 2101);

    while (client->getState() != CON_CONNECT)
    {
        client->process();
        Base::sleep(1);
    }

    //now connected
    std::string msg("this is a test message from ryan.");
    client->sendMyMessage(msg);

    while (true)//!g_callbackRecieved)
    {
        client->process();
        //Base::sleep(1);
        


        for (int i=0;i<20;i++)
        {
            client->sendMyMessage(msg);
            if (g_numSent % 100 == 0)
            {
                printf("sent(%d) recvd(%d)\n", g_numSent, g_numRecvd);
            }
        }


    }

    delete client;

    return 0;
}

