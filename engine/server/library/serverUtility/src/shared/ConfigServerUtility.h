// ConfigServerUtility.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_ConfigServerUtility_H
#define	_INCLUDED_ConfigServerUtility_H

//-----------------------------------------------------------------------

class ConfigServerUtility
{
public:
	~ConfigServerUtility();

	static int  getSpawnCookie  ();
	static int getChatLogMinutes();
	static int getServerMaxChatLogLines();
	static int getPlayerMaxChatLogLines();
	static bool isChatLogManagerLoggingEnabled();
	static bool isExternalAdminLevelsEnabled();
	static const char * getExternalAdminLevelsURL();
	static const char * getExternalAdminLevelsSecretKey();

	static void install();
	static void remove();

private:
	ConfigServerUtility();
	ConfigServerUtility & operator = (const ConfigServerUtility & rhs);
	ConfigServerUtility(const ConfigServerUtility & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConfigServerUtility_H
