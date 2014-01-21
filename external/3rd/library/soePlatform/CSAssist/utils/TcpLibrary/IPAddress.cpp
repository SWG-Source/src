#include "IPAddress.h"

#if defined(WIN32)
    #include <winsock.h>
    typedef int socklen_t;
#else        // for non-windows platforms (linux)
    #include <arpa/inet.h>
    #include <sys/socket.h>
        #include <string.h>
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

IPAddress::IPAddress(unsigned int ip)
: m_IP(ip)
{
}

char *IPAddress::GetAddress(char *buffer) const
{
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = m_IP;
    strcpy(buffer, inet_ntoa(addr.sin_addr));
    return(buffer);
}


#ifdef EXTERNAL_DISTRO
};
#endif
