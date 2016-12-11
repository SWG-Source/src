//========================================================================
//
// StaticObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#include "serverGame/FirstServerGame.h"
#include "serverGame/StaticObject.h"

#include "serverGame/GameServer.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/ServerCollisionProperty.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectTemplateList.h"

const SharedObjectTemplate * StaticObject::m_defaultSharedTemplate = nullptr;

//-----------------------------------------------------------------------

StaticObject::StaticObject(const ServerStaticObjectTemplate* newTemplate) :
	ServerObject(newTemplate)
{
	SharedObjectTemplate const * sharedObjectTemplate = getSharedTemplate();

	WARNING_STRICT_FATAL(!sharedObjectTemplate, ("Tried to create a STATIC object %s without a shared template!\n", newTemplate->DataResource::getName()));

	if(ConfigServerGame::getCreateAppearances())
	{
		const std::string &appearanceString = sharedObjectTemplate->getAppearanceFilename();
		if (appearanceString.size() != 0)
		{
			Appearance * newAppearance = AppearanceTemplateList::createAppearance(appearanceString.c_str());

			if(newAppearance != nullptr)
			{
				setAppearance(newAppearance);
			} else {
				DEBUG_WARNING(true, ("FIX ME: Appearance template for StaticObject::StaticObject missing for %s.", newTemplate->DataResource::getName()));
			}
		}
	}

	// attach the collision property

	ServerCollisionProperty* collision = new ServerCollisionProperty(*this,sharedObjectTemplate);

	addProperty(*collision);

	addMembersToPackages();

	//@todo -- HACK! this is supposed to be set by the database,
	// but that code isn't written. Assume all buildings and cells
	// are in the client cache file. //-- it is now [8/4/2002 4:20:48 PM],
	// this should be implemented in a few days. If you see this comment
	// and the code below calls setIsCachedOnClient(true), get with Chris
	// and Calan to determine the status of this code.
	//setCacheVersion(1);
}

//-----------------------------------------------------------------------

StaticObject::~StaticObject()
{
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * StaticObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/static/base/shared_static_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "StaticObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// StaticObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void StaticObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// StaticObject::removeDefaultTemplate

// ----------------------------------------------------------------------

/**
 * Sends a message to the Planet Server to update the position of the object.
 */
void StaticObject::updatePlanetServerInternal(const bool forceUpdate) const
{
	// only update static objects when forced, and only topmost static objects
	if (forceUpdate && !ContainerInterface::getContainedByObject(*this))
	{
		Vector const &position = getPosition_p();
	
		UpdateObjectOnPlanetMessage const msg(
			getNetworkId(),
			NetworkId::cms_invalid,
			static_cast<int>(position.x),
			static_cast<int>(position.y),
			static_cast<int>(position.z),
			0,
			static_cast<int>(getLocationReservationRadius()),
			false,
			false,
			static_cast<int>(getObjectType()),
			0,
			false,
			getTemplateCrc(),
			-1,
			-1);
		GameServer::getInstance().sendToPlanetServer(msg);
	}
}

//-----------------------------------------------------------------------

bool StaticObject::isVisibleOnClient (const Client & /*client*/) const
{
	return true;
}

//-----------------------------------------------------------------------

bool StaticObject::canDropInWorld() const
{
	return true;
}

//-----------------------------------------------------------------------

void StaticObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	ServerObject::getAttributes(data);
}

// ----------------------------------------------------------------------

StaticObject * StaticObject::asStaticObject()
{
	return this;
}

// ----------------------------------------------------------------------

StaticObject const * StaticObject::asStaticObject() const
{
	return this;
}

// ======================================================================
