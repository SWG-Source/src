// ======================================================================
//
// TeleportFixupHandler.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _TeleportFixupHandler_H_
#define _TeleportFixupHandler_H_

// ======================================================================

class ServerObject;

// ======================================================================

class TeleportFixupHandler
{
public:
	static void add(ServerObject &obj);
	static void update(float time);

private:
	TeleportFixupHandler();
	TeleportFixupHandler(TeleportFixupHandler const &);
	TeleportFixupHandler &operator=(TeleportFixupHandler const &);
};

// ======================================================================

#endif // _TeleportFixupHandler_H_

