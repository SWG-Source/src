//========================================================================
//
// SwgServerCreatureObjectTemplate.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgServerCreatureObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "SwgGameServer/SwgCreatureObject.h"


/**
 * Class constructor.
 */
SwgServerCreatureObjectTemplate::SwgServerCreatureObjectTemplate(const std::string & filename) :
	ServerCreatureObjectTemplate(filename)
{
}	// SwgServerCreatureObjectTemplate::SwgServerCreatureObjectTemplate

/**
 * Class destructor.
 */
SwgServerCreatureObjectTemplate::~SwgServerCreatureObjectTemplate()
{
}	// SwgServerCreatureObjectTemplate::~SwgServerCreatureObjectTemplate

/**
 * Replaces the ServerCreatureObjectTemplate tag->create mapping with our own.
 */
void SwgServerCreatureObjectTemplate::install(void)
{
	ObjectTemplateList::assignBinding(ServerCreatureObjectTemplate_tag, create);
}	// SwgServerCreatureObjectTemplate::install

/**
 * Creates a SwgServerCreatureObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SwgServerCreatureObjectTemplate::create(const std::string & filename)
{
	return new SwgServerCreatureObjectTemplate(filename);
}	// SwgServerCreatureObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * SwgServerCreatureObjectTemplate::createObject(void) const
{
	return new SwgCreatureObject(this);
}	// SwgServerCreatureObjectTemplate::createObject

