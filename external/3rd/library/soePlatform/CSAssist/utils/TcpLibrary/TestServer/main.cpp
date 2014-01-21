#include "TestServer.h"



#include <string>
#include <Base/Base.h>


int main(int argc, char **argv)
{

    TestServer *server = new TestServer(2101);

    while (true)
    {
        server->process();
        Base::sleep(1);
    }

    delete server;

    return 0;
}

