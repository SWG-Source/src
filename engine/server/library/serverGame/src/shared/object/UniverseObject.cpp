// ======================================================================
//
// UniverseObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/UniverseObject.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ServerUniverseObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/UniverseController.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/ObjectTemplateList.h"

// objvars for dynamic regions
const static std::string OBJVAR_DYNAMIC_REGION("dynamic_region");

const SharedObjectTemplate * UniverseObject::m_defaultSharedTemplate = nullptr;


// ======================================================================

UniverseObject::UniverseObject(const ServerUniverseObjectTemplate *newTemplate) :
		ServerObject(newTemplate, ServerWorld::getUniverseNotification())
{
	addMembersToPackages();
	ObjectTracker::addUniverseObject();
}

// ----------------------------------------------------------------------

UniverseObject::~UniverseObject()
{
	if (isDynamicRegion())
		RegionMaster::removeDynamicRegion(*this);

	if (isInWorld())
		removeFromWorld();

	ObjectTracker::removeUniverseObject();
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * UniverseObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/universe/base/shared_universe_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "UniverseObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// UniverseObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void UniverseObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// UniverseObject::removeDefaultTemplate

// ----------------------------------------------------------------------

Controller* UniverseObject::createDefaultController (void)
{
	Controller* controller = new UniverseController(this);
	
	setController(controller);
	return controller;
}

// ----------------------------------------------------------------------

void UniverseObject::initializeFirstTimeObject()
{
	ServerObject::initializeFirstTimeObject();
	setSceneId("universe");
//	if (isDynamicRegion())
//		RegionMaster::addDynamicRegion(*this);
}

// ----------------------------------------------------------------------

void UniverseObject::endBaselines()
{
	ServerObject::endBaselines();
	if (isDynamicRegion())
		RegionMaster::addDynamicRegion(*this);
}

// ----------------------------------------------------------------------

void UniverseObject::setupUniverse()
{
}

// ----------------------------------------------------------------------

void UniverseObject::onServerUniverseGainedAuthority()
{
}

// ----------------------------------------------------------------------

void UniverseObject::updatePlanetServerInternal(bool) const
{
}

// ----------------------------------------------------------------------

bool UniverseObject::isVisibleOnClient (const Client & /*client*/) const
{
	return false;
}

// ----------------------------------------------------------------------

void UniverseObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	ServerObject::getAttributes(data);
}

// ----------------------------------------------------------------------

bool UniverseObject::isDynamicRegion() const
{
	return getObjVars().hasItem(OBJVAR_DYNAMIC_REGION);
}

// ----------------------------------------------------------------------

const Region * UniverseObject::getDynamicRegion() const
{
	return RegionMaster::getDynamicRegionFromObject(*this);
}

// ======================================================================
