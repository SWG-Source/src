#include "serverGame/FirstServerGame.h"
#include "serverGame/WeaponObject.h"

#include "SwgGameServer/CombatEngine.h"
#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "serverGame/WeaponController.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include <cstdio>

//-- this include path is an error
#include "swgSharedUtility/Attributes.h"

//----------------------------------------------------------------------


const SharedObjectTemplate * WeaponObject::m_defaultSharedTemplate = nullptr;
static const std::string OBJVAR_CERTIFICATION = "weapon.strCertUsed";


//-----------------------------------------------------------------------

WeaponObject::WeaponObject(const ServerWeaponObjectTemplate* newTemplate) : 
	TangibleObject(newTemplate),
	m_minDamage(0),
	m_maxDamage(0),
	m_attackSpeed(0),
	m_woundChance(0),
	m_minRange(0),
	m_maxRange(0),
	m_accuracy(0),
	m_damageRadius(0),
	m_attackCost(0),
	m_isDefaultWeapon(false),
	m_damageType(newTemplate->getDamageType()),
	m_elementalType(0), //m_elementalType(newTemplate->getElementalType()),
	m_elementalValue(0), //m_elementalValue(newTemplate->getElementalValue()),
	m_weaponType(newTemplate->getWeaponType())
{
	addMembersToPackages();
}

//-----------------------------------------------------------------------

WeaponObject::~WeaponObject()
{
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * WeaponObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/weapon/base/shared_weapon_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "WeaponObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// WeaponObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void WeaponObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// WeaponObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Sets up the data for a new weapon object.
 */
void WeaponObject::initializeFirstTimeObject()
{
	TangibleObject::initializeFirstTimeObject();

	const ServerWeaponObjectTemplate * myTemplate = safe_cast<
		const ServerWeaponObjectTemplate *>(getObjectTemplate());
	NOT_NULL(myTemplate);

	m_minDamage        = myTemplate->getMinDamageAmount();
	m_maxDamage        = myTemplate->getMaxDamageAmount();
	m_attackSpeed      = myTemplate->getAttackSpeed();
	m_woundChance      = myTemplate->getWoundChance();
	m_minRange         = myTemplate->getMinRange();
	m_maxRange         = myTemplate->getMaxRange();
	m_accuracy         = myTemplate->getAccuracy();
	m_damageRadius     = myTemplate->getDamageRadius();
	m_attackCost       = myTemplate->getAttackCost();
	m_elementalType    = myTemplate->getElementalType();
	m_elementalValue   = myTemplate->getElementalValue();
}	// WeaponObject::initializeFirstTimeObject

//-----------------------------------------------------------------------

/**
 * Sets up the data for an old weapon object.
 */
void WeaponObject::onLoadedFromDatabase()
{
	TangibleObject::onLoadedFromDatabase();

	const ServerWeaponObjectTemplate * myTemplate = safe_cast<
		const ServerWeaponObjectTemplate *>(getObjectTemplate());
	NOT_NULL(myTemplate);

	// if the min or max range hasn't been set, set it from the template
	if (m_minRange.get() < 0)
		m_minRange = myTemplate->getMinRange();
	if (m_maxRange.get() < 0)
		m_maxRange = myTemplate->getMaxRange();

}	// WeaponObject::initializeFirstTimeObject

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* WeaponObject::createDefaultController(void)
{
	Controller* controller = new WeaponController(this);

	setController(controller);
	return controller;
}	// WeaponObject::createDefaultController

// ----------------------------------------------------------------------

ServerWeaponObjectTemplate::WeaponType WeaponObject::getWeaponType(void) const
{
	const ServerWeaponObjectTemplate * myTemplate = safe_cast<
		const ServerWeaponObjectTemplate *>(getObjectTemplate());
	NOT_NULL(myTemplate);

	return myTemplate->getWeaponType();
}

// ----------------------------------------------------------------------

ServerWeaponObjectTemplate::AttackType WeaponObject::getAttackType() const
{
	const ServerWeaponObjectTemplate * myTemplate = safe_cast<
		const ServerWeaponObjectTemplate *>(getObjectTemplate());
	NOT_NULL(myTemplate);

	return myTemplate->getAttackType();
}

// ----------------------------------------------------------------------

float WeaponObject::getAudibleRange() const
{
	const ServerWeaponObjectTemplate * myTemplate = safe_cast<
		const ServerWeaponObjectTemplate *>(getObjectTemplate());
	NOT_NULL(myTemplate);

	return myTemplate->getAudibleRange();
}

// ----------------------------------------------------------------------

/**
 * Returns the minimum amount of damage the weapon can do, scaled by the amount 
 * of damage the weapon has taken.
 *
 * @return the min damage value
 */
int WeaponObject::getMinDamage() const		
{
	if (isDisabled())
		return 0;

	float effectiveness = 1.0f;
	float damagePercent = 1.0f - static_cast<float>(getDamageTaken()) / getMaxHitPoints();
	if (damagePercent < ConfigServerGame::getWeaponDecayThreshold())
	{
		effectiveness -= ConfigServerGame::getWeaponDecayThreshold() - damagePercent;
	}

	return static_cast<int>(m_minDamage.get() * effectiveness);
}	// WeaponObject::getMinDamageAmount

// ----------------------------------------------------------------------

/**
 * Returns the minimum amount of damage the weapon can do, scaled by the amount 
 * of damage the weapon has taken.
 *
 * @return the min damage value
 */
int WeaponObject::getMaxDamage() const
{
	if (isDisabled())
		return 0;

	float effectiveness = 1.0f;
	float damagePercent = 1.0f - static_cast<float>(getDamageTaken()) / getMaxHitPoints();
	if (damagePercent < ConfigServerGame::getWeaponDecayThreshold())
	{
		effectiveness -= ConfigServerGame::getWeaponDecayThreshold() - damagePercent;
	}

	return static_cast<int>(m_maxDamage.get() * effectiveness);
}	// WeaponObject::getMaxDamageAmount

// ----------------------------------------------------------------------

/**
 * Prevent destruction of the weapon if it is a default weapon.
 */
bool WeaponObject::canDestroy() const
{
	if (m_isDefaultWeapon.get())
	{
		WARNING(true, ("WeaponObject::canDestroy called for default weapon "
			"%s", getNetworkId().getValueString().c_str()));
		return false;
	}
	return TangibleObject::canDestroy();
}	// WeaponObject::canDestroy

// ----------------------------------------------------------------------

/**
 * Prevent transfer of the weapon if it is a default weapon.
 */
bool WeaponObject::onContainerAboutToTransfer(ServerObject * destination, 
	ServerObject* transferer)
{
	if (m_isDefaultWeapon.get())
	{
		WARNING(true, ("WeaponObject::onContainerAboutToTransfer called "
			"for default weapon %s", getNetworkId().getValueString().c_str()));
		return false;
	}
	return TangibleObject::onContainerAboutToTransfer(destination, transferer);
}	// WeaponObject::onContainerAboutToTransfer

// ----------------------------------------------------------------------

/**
 * Flags this weapon as/not a default weapon.
 *
 * @param isDefault		flag that the weapon is a default weapon or not.
 */
void WeaponObject::setAsDefaultWeapon(bool isDefault)
{
	if (isDefault != m_isDefaultWeapon.get())
	{
		if (isAuthoritative())
			m_isDefaultWeapon = isDefault;
		else
		{
			WARNING_STRICT_FATAL(true, ("WeaponObject::setAsDefaultWeapon called on "
				"non-auth weapon %s", getNetworkId().getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void WeaponObject::getAttributes(const NetworkId & playerId, AttributeVector &data) const
{
	TangibleObject::getAttributes(playerId, data);
}

// ----------------------------------------------------------------------

void WeaponObject::getAttributes(AttributeVector &data) const
{
	TangibleObject::getAttributes(data);	
}

// ----------------------------------------------------------------------

WeaponObject *WeaponObject::asWeaponObject()
{
	return this;
}

// ----------------------------------------------------------------------

WeaponObject const *WeaponObject::asWeaponObject() const
{
	return this;
}

// ----------------------------------------------------------------------

WeaponObject * WeaponObject::getWeaponObject(NetworkId const & networkId)
{
	ServerObject * serverObject = ServerObject::getServerObject(networkId);

	return (serverObject != nullptr) ? serverObject->asWeaponObject() : nullptr;
}

// ----------------------------------------------------------------------
