//======================================================================
//
// ServerSharedObjectTemplateInterface.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerSharedObjectTemplateInterface_H
#define INCLUDED_ServerSharedObjectTemplateInterface_H

//======================================================================

#include "sharedGame/SharedObjectTemplateInterface.h"

class Object;
class SharedObjectTemplate;

//----------------------------------------------------------------------

class ServerSharedObjectTemplateInterface : public SharedObjectTemplateInterface
{
public:
	ServerSharedObjectTemplateInterface();
	virtual ~ServerSharedObjectTemplateInterface();

	virtual SharedObjectTemplate const * getSharedTemplate(Object const * object) const;
	virtual bool isShipObject(Object const * object) const;
	virtual uint32 getShipChassisType(Object const * object) const;
	virtual uint32 getShipComponentCrc(Object const * object, int chassisSlot) const;

private:
	ServerSharedObjectTemplateInterface(ServerSharedObjectTemplateInterface const & copy);
	ServerSharedObjectTemplateInterface & operator=(ServerSharedObjectTemplateInterface const & copy);
};

//======================================================================

#endif
