//========================================================================
//
// SwgServerCreatureObjectTemplate.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SwgServerCreatureObjectTemplate_H
#define _INCLUDED_SwgServerCreatureObjectTemplate_H

#include "serverGame/ServerCreatureObjectTemplate.h"


class SwgServerCreatureObjectTemplate : public ServerCreatureObjectTemplate
{
public:
	         SwgServerCreatureObjectTemplate(const std::string & filename);
	virtual ~SwgServerCreatureObjectTemplate();

	static void      install(void);
	virtual Object * createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SwgServerCreatureObjectTemplate(const SwgServerCreatureObjectTemplate &);
	SwgServerCreatureObjectTemplate & operator =(const SwgServerCreatureObjectTemplate &);
};


#endif	// _INCLUDED_SwgServerCreatureObjectTemplate_H
