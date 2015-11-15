//======================================================================
//
// SharedObjectTemplateInterface.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SharedObjectTemplateInterface_H
#define INCLUDED_SharedObjectTemplateInterface_H

//======================================================================

class Object;
class SharedObjectTemplate;

//----------------------------------------------------------------------

class SharedObjectTemplateInterface
{
public:
	SharedObjectTemplateInterface();
	virtual ~SharedObjectTemplateInterface();

	virtual SharedObjectTemplate const * getSharedTemplate(Object const * object) const = 0;
	virtual bool isShipObject(Object const * object) const = 0;
	virtual uint32 getShipChassisType(Object const * object) const = 0;
	virtual uint32 getShipComponentCrc(Object const * object, int chassisSlot) const = 0;

private:

	SharedObjectTemplateInterface(SharedObjectTemplateInterface const & copy);
	SharedObjectTemplateInterface & operator=(SharedObjectTemplateInterface const & copy);
};

//======================================================================

#endif
