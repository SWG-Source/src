//========================================================================
//
// SwgServerPlayerObjectTemplate.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SwgServerPlayerObjectTemplate_H
#define _INCLUDED_SwgServerPlayerObjectTemplate_H

#include "serverGame/ServerPlayerObjectTemplate.h"


class SwgServerPlayerObjectTemplate : public ServerPlayerObjectTemplate
{
public:
	         SwgServerPlayerObjectTemplate(const std::string & filename);
	virtual ~SwgServerPlayerObjectTemplate();

	static void      install(void);
	virtual Object * createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SwgServerPlayerObjectTemplate(const SwgServerPlayerObjectTemplate &);
	SwgServerPlayerObjectTemplate & operator =(const SwgServerPlayerObjectTemplate &);
};


#endif	// _INCLUDED_SwgServerPlayerObjectTemplate_H
