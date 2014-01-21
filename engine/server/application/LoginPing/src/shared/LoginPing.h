// ======================================================================
//
// LoginPing.h
//
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
//
// ======================================================================

#ifndef	_LoginPing_H
#define	_LoginPing_H

// ======================================================================

class LoginConnection;

// ======================================================================

class LoginPing
{
public:
	LoginPing();
	~LoginPing();

	static bool ping();
	static void update();

	static void receiveReplyMessage();
	static void onLoginConnectionDestroyed(LoginConnection const *loginConnection);

private:
	static LoginPing &instance();

	LoginConnection *m_loginConnection;
	bool m_waitingForResponse;	
};

// ======================================================================

#endif	// _LoginPing_H

