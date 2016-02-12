// ======================================================================
//
// SwgServerUniverse.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgServerUniverse.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "sharedObject/ObjectTemplateList.h"
#include "SwgGameServer/JediManagerObject.h"


// ======================================================================

void SwgServerUniverse::install()
{
	Universe::installDerived(new SwgServerUniverse);
	ServerUniverse::install();
}

// ----------------------------------------------------------------------

SwgServerUniverse::SwgServerUniverse() :
	ServerUniverse (),
	m_jediManager  (nullptr)
{
}

// ----------------------------------------------------------------------

SwgServerUniverse::~SwgServerUniverse()
{
}

// ----------------------------------------------------------------------

/** 
 * Check the Universe Objects.  Spawn new objects as
 * needed, update the amount remaining on all the resource pools, etc.
 *
 * May be time consuming, so this should not be called every frame.
 */

void SwgServerUniverse::updateAndValidateData()
{
	WARNING_STRICT_FATAL(!isAuthoritative(), ("UpdateAndValidateData() should "
		"only be called on the process that is authoritative for UniverseObjects.\n"));

	// create Jedi manager
	if (m_jediManager == nullptr)
	{
		const ServerObjectTemplate * objTemplate = safe_cast<const ServerObjectTemplate *>(ObjectTemplateList::fetch(ConfigServerGame::getJediManagerTemplate()));
		m_jediManager = safe_cast<JediManagerObject *>(ServerWorld::createNewObject(*objTemplate, Transform::identity, 0, false));
		NOT_NULL(m_jediManager);
		registerJediManager(*m_jediManager);
		m_jediManager->addToWorld();
		objTemplate->releaseReference();
	}

	ServerUniverse::updateAndValidateData();
}


// ======================================================================
