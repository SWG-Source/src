// ======================================================================
//
// UniverseLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UniverseLocator_H
#define INCLUDED_UniverseLocator_H

#include "serverDatabase/ObjectLocator.h"

// ======================================================================

class UniverseLocator:public ObjectLocator
{
  public:
	virtual bool locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated);
	virtual void sendPostBaselinesCustomData(GameServerConnection &conn) const;
};

// ======================================================================

#endif
