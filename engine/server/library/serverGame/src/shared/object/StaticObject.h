//========================================================================
//
// StaticObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_StaticObject_H
#define _INCLUDED_StaticObject_H

#include "serverGame/ServerObject.h"

class Controller;
class ServerStaticObjectTemplate;


class StaticObject : public ServerObject
{
public:

	StaticObject(const ServerStaticObjectTemplate* newTemplate);
	virtual ~StaticObject();

	virtual StaticObject *       asStaticObject();
	virtual StaticObject const * asStaticObject() const;

	static void removeDefaultTemplate(void);

	void addMembersToPackages();

	virtual bool canDropInWorld() const;
	virtual bool isVisibleOnClient (const Client & client) const;
	virtual void getAttributes     (std::vector<std::pair<std::string, Unicode::String> > &data) const;

protected:
	
	virtual void updatePlanetServerInternal(bool forceUpdate) const;
	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	
private:
  	StaticObject();
	StaticObject(const StaticObject& rhs);
	StaticObject&	operator=(const StaticObject& rhs);

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given
};

//------------------------------------------------------------------------------------------

#endif	// _INCLUDED_StaticObject_H
