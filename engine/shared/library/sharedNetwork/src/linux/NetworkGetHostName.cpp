// NetworkGetHostName.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSharedNetwork.h"
#include "Address.h"
#include "sharedNetwork/NetworkHandler.h"
#include <net/if.h>
#include <unistd.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//-----------------------------------------------------------------------

struct HN
{
	HN();
	std::string hostName;
};

//-----------------------------------------------------------------------

HN::HN()
{
	char name[512] = {"\0"};
	if(gethostname(name, sizeof(name)) == 0)
	{
		Address a(name, 0);
		//hostName = name;
		hostName = a.getHostAddress();
	}
}

//-----------------------------------------------------------------------

const std::string & NetworkHandler::getHostName()
{
	static HN hn;
	return hn.hostName;
}

//-----------------------------------------------------------------------

const std::string & NetworkHandler::getHumanReadableHostName()
{
	char name[512] = {"\0"};
	static std::string nameString;
	if(nameString.empty())
	{
		if(gethostname(name, sizeof(name)) == 0)
		{
			name[sizeof(name) - 1] = 0;
			//hostName = name;
			nameString = name;
		}
	}
	return nameString;
}

//-----------------------------------------------------------------------

struct HL
{
	HL();
	std::vector<std::pair<std::string, std::string> >  addrList;
};

//-----------------------------------------------------------------------

HL::HL() :
addrList()
{
	struct ifconf ifc;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	// create a socket to perform SIOCGIFCONF on
	if(sock > -1)
	{
		// determine buffer size to receive array of if_req's, initialize it
		// when length is appropriate
		int reqSize = 100 * sizeof(struct ifreq);
		int lastReqSize = reqSize;
		for(;;)
		{
			char * buf = new char[reqSize];
			ifc.ifc_len = reqSize;
			ifc.ifc_buf = buf;
			memset(ifc.ifc_buf, 0, reqSize);
			if (ioctl(sock, SIOCGIFCONF, &ifc) < 0)
			{
				if (errno != EINVAL || lastReqSize != 0)
				{
					WARNING(true, ("Error getting interface list: %s", strerror(errno)));
					delete[] buf;
					return;
				}
			}
			else
			{
				if(ifc.ifc_len == lastReqSize)
					break; // we have the reqs
				lastReqSize = ifc.ifc_len;
			}

			// need more buffer space for request
			reqSize += sizeof(struct ifreq);
			delete [] buf;
		}

		int reqCount = reqSize / sizeof(struct ifreq);
		int i;
		ifreq * reqs = (struct ifreq *)ifc.ifc_buf;
		for(i = 0; i < reqCount; ++i)
		{
			struct sockaddr_in * sa = (struct sockaddr_in *)&reqs[i].ifr_addr;
			if(sa->sin_family == AF_INET)
			{
				char addrbuf[17] = {"\0"};
				unsigned char * a = (unsigned char *)&sa->sin_addr;
				snprintf(addrbuf, sizeof(addrbuf), "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
				addrList.push_back(std::make_pair(std::string(reqs[i].ifr_name), std::string(addrbuf)));
			}
		}
		
	}
}

//-----------------------------------------------------------------------

const std::vector<std::pair<std::string, std::string> > & NetworkHandler::getInterfaceAddresses()
{
	static HL hl;
	return hl.addrList;
}

//-----------------------------------------------------------------------
