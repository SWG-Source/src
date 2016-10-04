#ifndef TCPIPADDRESS_H
#define TCPIPADDRESS_H

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

/**
 * @brief Container object for IP Address.
 */
class IPAddress
{
public:

    /**
     * @brief Constructor, sets the ip address if specified.
     */
    IPAddress(unsigned int ip = 0);

    /**
     * @brief Sets the ip address.
     */
    void SetAddress(unsigned int ip){ m_IP = ip; }

    /**
     * @brief Returns the unsigned int representation of this address.
     */
    unsigned int GetAddress() const { return m_IP; }

    /**
     * @brief Used to retreive the the dot-notation represenatatiion of this address.
     *
     * @param buffer  A pointer to the buffer to place the ip address into.
     *          Must be at least 17 characters long, will be nullptr terminated.
     *
     * @return A pointer to the buffer the address was placed into.
     */
    char *GetAddress(char *buffer) const;

private:
    unsigned int m_IP;
};

#ifdef EXTERNAL_DISTRO
};
#endif

#endif //TCPIPADDRESS_H


