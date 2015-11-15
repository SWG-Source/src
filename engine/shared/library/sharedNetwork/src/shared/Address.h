// Address.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//---------------------------------------------------------------------

#ifndef	INCLUDED_Address_H
#define	INCLUDED_Address_H

//---------------------------------------------------------------------

#include <string>

//---------------------------------------------------------------------

/**
	@brief An IPv4 network address

	Address provides an interface for working
	with IPv4 addresses. It contains a resolved	host name string and a
	BSD sockaddr. It also handles name to address resolution.

	@author Justin Randall
*/
class Address
{
public:
	Address();
	Address(const std::string & dottedDecimalIPv4Address, unsigned short hostPort);
	Address(const Address & source);
	Address(const struct sockaddr_in & ipv4sockaddr);
	~Address();
	Address & operator=(const Address & rhs);
	Address & operator=(const struct sockaddr_in & rhs);
	const bool operator<(const Address & rhs) const;
	const bool operator==(const Address & rhs) const;
	const bool operator!=(const Address & rhs) const;
	const bool operator>(const Address & rhs) const;
	const std::string &  getHostAddress() const;
	const unsigned short getHostPort() const;
	size_t hashFunction() const;
	const struct sockaddr_in & getSockAddr4() const;

	/**
		@brief Determine equality between to address for the STL hash_map
		@author Justin Randall
	*/
	struct EqualFunction
	{
		bool operator() (const Address & lhs, const Address & rhs) const;
	};

	/**
		@brief STL hash_map suport function -- returns the s_addr member of
		the BSD sockaddr struct

		@author Justin Randall
	*/
	struct HashFunction
	{
		size_t operator() (const Address & a) const;
	};


private:
	void               convertFromSockAddr(const struct sockaddr_in & source);
private:
	struct sockaddr_in * addr4;
	std::string        hostAddress;
};


/**
	\include TestAddress.h
	\example TestAddress.cpp
*/
//---------------------------------------------------------------------

#endif	// _NetworkAddress_H
