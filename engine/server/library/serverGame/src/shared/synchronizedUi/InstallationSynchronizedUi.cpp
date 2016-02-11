// ======================================================================
//
// InstallationSynchronizedUi.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/InstallationSynchronizedUi.h"

#include "UnicodeUtils.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedUtility/InstallationResourceData.h"

//-----------------------------------------------------------------------
InstallationSynchronizedUi::InstallationSynchronizedUi(InstallationObject& owner) :
		ServerSynchronizedUi(owner),
		m_resourcesModified       (0),
		m_resourcePools           (),
		m_resourceTypesId         (),
		m_resourceTypesName       (),
		m_resourceTypesParent     (),
		m_resourcePoolId          (NetworkId::cms_invalid),
		m_active                  (false),
		m_maxExtractionRate       (0),
		m_currentExtractionRate   (0.0f),
		m_installedExtractionRate (0.0f),
		m_hopperContentsMax       (0),
		m_hopperContentsModified  (0),
		m_hopperContents          (),
		m_hopperContentsCur       (0.0f),
		m_condition               (0)
{
	addToUiPackage(m_resourcesModified);
	addToUiPackage(m_resourcePools);
	addToUiPackage(m_resourceTypesId);
	addToUiPackage(m_resourceTypesName);
	addToUiPackage(m_resourceTypesParent);
	addToUiPackage(m_resourcePoolId);
	addToUiPackage(m_active);
	addToUiPackage(m_maxExtractionRate);
	addToUiPackage(m_currentExtractionRate);
	addToUiPackage(m_installedExtractionRate);
	addToUiPackage(m_hopperContentsCur);
	addToUiPackage(m_hopperContentsMax);
	addToUiPackage(m_hopperContentsModified);
	addToUiPackage(m_hopperContents);
	addToUiPackage(m_condition);

	resetResourcePools ();
}

//-----------------------------------------------------------------------

InstallationSynchronizedUi::~InstallationSynchronizedUi()
{
	
}

//----------------------------------------------------------------------

//@ todo: this should be a server-side funtcion only

void  InstallationSynchronizedUi::addResource (const NetworkId & id)
{
	ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeById(id);
	if (!typeObj || typeObj->isDepleted())
		return;

	if (std::find (m_resourcePools.begin (), m_resourcePools.end (), id) != m_resourcePools.end ())
		return;

	m_resourcePools.push_back (id);
	addResourceType(id);
	m_resourcesModified = static_cast<uint8>(m_resourcesModified.get () + 1);
}

//----------------------------------------------------------------------

namespace InstallationSynchronizedUiNamespace
{
	InstallationResourceData makeInstallationResourceData (const NetworkId & id, const Vector & position)
	{
		ResourceTypeObject const * const rto = ServerUniverse::getInstance().getResourceTypeById(id);
		ResourcePoolObject const * const pool = rto ? rto->getPoolForCurrentPlanet() : nullptr;
		if (rto && pool)
		{
			std::string const & resourceTypeParent = rto->getParentClass().getResourceClassName();

			return InstallationResourceData (rto->getResourceName(), id, resourceTypeParent, static_cast<uint8>(100.0 * pool->getEfficiencyAtLocation (position.x, position.z)));
		}
		else
			return InstallationResourceData (std::string (), id, std::string(), 0);
	}
}

using namespace InstallationSynchronizedUiNamespace;

//----------------------------------------------------------------------

void InstallationSynchronizedUi::getResourceData (ResourceDataVector & data)
{
	InstallationObject * const installation = dynamic_cast<InstallationObject *>(getOwner ());

	if (!installation)
		return;

	const Vector position = installation->getPosition_w ();

	std::vector<NetworkId> ::const_iterator it;
	for (it = m_resourcePools.begin (); it != m_resourcePools.end (); ++it)
		data.push_back (makeInstallationResourceData (*it, position));

	for (HopperContentsVector::const_iterator hit = m_hopperContents.begin (); hit != m_hopperContents.end (); ++hit)
	{
		const HopperContentElement & elem = *hit;
		ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(elem.first);
		
		if (resourceTypeObject)
		{
				data.push_back (InstallationResourceData (resourceTypeObject->getResourceName (), elem.first, resourceTypeObject->getParentClass().getResourceClassName(), 0));
		}
	}
}

//----------------------------------------------------------------------

void InstallationSynchronizedUi::clearResources       ()
{
	m_resourcePools.clear ();
	m_resourceTypesId.clear ();
	m_resourceTypesName.clear ();
	m_resourceTypesParent.clear ();
	m_resourcesModified = static_cast<uint8>(m_resourcesModified.get () + 1);
}

//-----------------------------------------------------------------------

void InstallationSynchronizedUi::refresh()
{
	HarvesterInstallationObject* const harvester=dynamic_cast<HarvesterInstallationObject*>(getOwner());
	NOT_NULL(harvester);

	m_resourcePoolId          = harvester->getSelectedResourceTypeId();
	m_active                  = harvester->isActive();
	addResource(m_resourcePoolId.get());

	m_maxExtractionRate       = harvester->getMaxExtractionRate();
	m_currentExtractionRate   = harvester->getCurrentExtractionRate();
	m_installedExtractionRate = harvester->getInstalledExtractionRate();

	int8 condition = static_cast<int8>(100 - (100 * harvester->getDamageTaken() / harvester->getMaxHitPoints()));
	if (condition == 100 && harvester->getDamageTaken() != 0)
		condition = 99; // make sure even a trivial amount of damage is visible on the meter
	m_condition = condition;

	for (ResourcePoolVector::const_iterator i = m_resourcePools.begin (); i != m_resourcePools.end (); )
	{
		ResourceTypeObject const * const typeObj = ServerUniverse::getInstance().getResourceTypeById(*i);
		if (!typeObj || typeObj->isDepleted())
		{
			m_resourcePools.erase(std::distance (m_resourcePools.begin (), i));
			m_resourcesModified = static_cast<uint8>(m_resourcesModified.get()+1);
		}
		else
			++i;
	}

	//----------------------------------------------------------------------
	//-- update the hopper contents

	m_hopperContentsMax       = harvester->getMaxHopperAmount();

	HopperContentsVector hopperContents;
	float hopperContentsCurLocal = harvester->getHopperContents (&hopperContents);

	size_t index = 0;

	if (hopperContents.size () != m_hopperContents.size ())
	{
		m_hopperContents.resize (hopperContents.size (), HopperContentElement ());
		m_hopperContentsModified = static_cast<uint8>(m_hopperContentsModified.get()+1);
	}
	
	for (HopperContentsVector::const_iterator harvesterIt = hopperContents.begin (); harvesterIt != hopperContents.end (); ++harvesterIt, ++index)
	{
		const HopperContentElement & harvesterElem = *harvesterIt;
		const HopperContentElement & syncElem      = m_hopperContents.get (index);
		
		addResource(harvesterIt->first);

		if (syncElem.first != harvesterElem.first || harvesterElem.second != syncElem.second) //lint !e777 // != on floats
		{
			m_hopperContents.set (index, harvesterElem);
			m_hopperContentsModified = static_cast<uint8>(m_hopperContentsModified.get()+1);
		}
	}

	m_hopperContentsCur =  hopperContentsCurLocal; // update this last so that the client refreshes the interface propery (little bit of a hack here)
}

//----------------------------------------------------------------------

void InstallationSynchronizedUi::resetResourcePools   ()
{
	m_resourcePools.clear ();

	clearResourceTypes();

	m_resourcesModified = static_cast<uint8>(m_resourcesModified.get()+1);

	// Puts all the possible pools in the active list
	// refresh() will move the deleted ones to the other list
	HarvesterInstallationObject * const harvester = dynamic_cast<HarvesterInstallationObject *>(getOwner ());
	NOT_NULL (harvester);

	if (harvester) //lint !e774 // harvester is not nullptr in debug
	{
		std::vector<ResourceTypeObject const *> const & pools = harvester->getSurveyTypes();
		for (std::vector<ResourceTypeObject const *>::const_iterator i=pools.begin(); i!=pools.end(); ++i)
		{
			if (*i)
			{
				m_resourcePools.push_back((*i)->getNetworkId());
				addResourceType((*i)->getNetworkId());
			}
		}
	}

	refresh ();
}

//----------------------------------------------------------------------

void InstallationSynchronizedUi::addResourceType (const NetworkId& typeId)
{
	ResourceTypeObject const * const rto = ServerUniverse::getInstance().getResourceTypeById(typeId);
	if (rto)
	{
		m_resourceTypesId.push_back(typeId);
		m_resourceTypesName.push_back(rto->getResourceName());
		m_resourceTypesParent.push_back(rto->getParentClass().getResourceClassName());
	}
}

//----------------------------------------------------------------------

void InstallationSynchronizedUi::clearResourceTypes ()
{
	m_resourceTypesId.clear ();
	m_resourceTypesName.clear ();
	m_resourceTypesParent.clear ();
}

// ======================================================================
