// ConfigCustomerServiceServer.h
// copyright 2000 Verant Interactive

#ifndef	INCLUDED_ConfigCustomerServiceServer_H
#define	INCLUDED_ConfigCustomerServiceServer_H

//-----------------------------------------------------------------------

class ConfigCustomerServiceServer
{
public:
	
	static void			  install();
	static void			  remove();

	static const char *   getCustomerServiceServerAddress();
	static unsigned short getCustomerServiceServerPort();
	static const char *   getCentralServerAddress();
	static unsigned short getCentralServerPort();
	static const char *	  getClusterName();
	static const char *   getGameCode();
	static int            getMaxPacketsPerSecond();
	static unsigned int   getRequestTimeoutSeconds();
	static int            getMaxAllowedNumberOfTickets();
	static unsigned short getGameServicePort();
	static unsigned short getChatServicePort();
	static const char*    getChatServiceBindInterface();
	static const char*    getGameServiceBindInterface();
	static bool           getWriteTicketToBugLog();
private:

	// Disabled

	ConfigCustomerServiceServer();
	~ConfigCustomerServiceServer();
	ConfigCustomerServiceServer(ConfigCustomerServiceServer const &);
	ConfigCustomerServiceServer &operator =(ConfigCustomerServiceServer const &);
};

#endif	// INCLUDED_ConfigCustomerServiceServer_H
