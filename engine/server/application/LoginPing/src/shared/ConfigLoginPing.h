// ConfigLoginPing.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_ConfigLoginPing_H
#define	_ConfigLoginPing_H

//-----------------------------------------------------------------------

class ConfigLoginPing
{
  public:
    struct Data
    {
		const char *  rcFileName;
		bool          passthroughMode;
		const char *  loginServerAddress;
		uint16        loginServerPingServicePort;
    };

	static const char *  getRcFileName                  ();
	static bool          getPassthroughMode             ();
	static const char *  getLoginServerAddress          ();
	static uint16        getLoginServerPingServicePort  ();
    static void          install                        ();
    static void          remove                         ();

  private:
    static Data *	data;
};

//-----------------------------------------------------------------------

inline const char * ConfigLoginPing::getRcFileName()
{
	return data->rcFileName;
}

//-----------------------------------------------------------------------

inline bool ConfigLoginPing::getPassthroughMode()
{
	return data->passthroughMode;
}

//-----------------------------------------------------------------------

inline const char * ConfigLoginPing::getLoginServerAddress()
{
	return data->loginServerAddress;
}
//-----------------------------------------------------------------------

inline uint16 ConfigLoginPing::getLoginServerPingServicePort()
{
	return data->loginServerPingServicePort;
}

//-----------------------------------------------------------------------

#endif	// _ConfigLoginPing_H
