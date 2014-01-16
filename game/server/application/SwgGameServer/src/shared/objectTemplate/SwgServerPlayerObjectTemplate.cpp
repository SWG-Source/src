//========================================================================
//
// SwgServerPlayerObjectTemplate.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgServerPlayerObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "SwgGameServer/SwgPlayerObject.h"


/**
 * Class constructor.
 */
SwgServerPlayerObjectTemplate::SwgServerPlayerObjectTemplate(const std::string & filename) :
	ServerPlayerObjectTemplate(filename)
{
}	// SwgServerPlayerObjectTemplate::SwgServerPlayerObjectTemplate

/**
 * Class destructor.
 */
SwgServerPlayerObjectTemplate::~SwgServerPlayerObjectTemplate()
{
}	// SwgServerPlayerObjectTemplate::~SwgServerPlayerObjectTemplate

/**
 * Replaces the ServerCreatureObjectTemplate tag->create mapping with our own.
 */
void SwgServerPlayerObjectTemplate::install(void)
{
	ObjectTemplateList::assignBinding(ServerPlayerObjectTemplate_tag, create);
}	// SwgServerPlayerObjectTemplate::install

/**
 * Creates a SwgServerPlayerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SwgServerPlayerObjectTemplate::create(const std::string & filename)
{
	return new SwgServerPlayerObjectTemplate(filename);
}	// SwgServerPlayerObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * SwgServerPlayerObjectTemplate::createObject(void) const
{
	return new SwgPlayerObject(this);
}	// SwgServerPlayerObjectTemplate::createObject

