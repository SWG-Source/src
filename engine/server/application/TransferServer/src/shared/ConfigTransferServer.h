// ConfigTransferServer.h
// Copyright 2000-04, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_ConfigTransferServer_H
#define	_ConfigTransferServer_H

//-----------------------------------------------------------------------

#include <string>
#include <vector>

//-----------------------------------------------------------------------

class ConfigTransferServer
{
  public:
    struct Data
    {
		int           centralServerServiceBindPort;
		const char *  consoleServiceBindInterface;
		const char *  gameServiceBindInterface;
		const char *  centralServerServiceBindInterface;
		bool          allowSameServerTransfers;
		bool          allowAccountTransfers;
		const char *  apiServerHostAddress;
		bool          transferChatAvatar;
    };

	static uint16        getCentralServerServiceBindPort       ();
	static const char *  getCentralServerServiceBindInterface  ();
	static bool          getAllowSameServerTransfers           ();
	static bool          getAllowAccountTransfers              ();
	
	static void          install                        ();
	static void          remove                         ();

	static const std::vector<std::string> & getServersAllowedToUploadCharacterData();
	static const std::vector<std::string> & getServersAllowedToDownloadCharacterData();
	static const char * getApiServerHostAddress();
	static const bool getTransferChatAvatar();
  private:
    static Data *	data;
};

// ----------------------------------------------------------------------

#endif	// _ConfigTransferServer_H
