//========================================================================
//
// WeaponObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_WeaponObject_H
#define _INCLUDED_WeaponObject_H

#include "Archive/AutoDeltaByteStream.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "sharedGame/SharedWeaponObjectTemplate.h"

#include <climits>

class NetworkId;

/** 
  * A weapon object is an object that is used in combat to damage other objects.
  */
class WeaponObject : public TangibleObject
{
public:

	enum OutOfRangeAttackMod
	{
		OUT_OF_RANGE_ATTACK_MOD = INT_MIN
	};

public:
	
	WeaponObject(const ServerWeaponObjectTemplate* newTemplate);
	virtual ~WeaponObject();

	static WeaponObject * getWeaponObject(NetworkId const & networkId);
	virtual WeaponObject *asWeaponObject();
	virtual WeaponObject const *asWeaponObject() const;

	static void         removeDefaultTemplate          (void);


	virtual Controller* createDefaultController(void);
	void                addMembersToPackages();

	ServerWeaponObjectTemplate::WeaponType    getWeaponType        (void) const;
	ServerWeaponObjectTemplate::AttackType    getAttackType        (void) const;
	ServerWeaponObjectTemplate::DamageType    getDamageType        (void) const;
	ServerWeaponObjectTemplate::DamageType    getElementalType     (void) const;
	int                                       getElementalValue    (void) const;
	int                                       getMaxDamage         (void) const;
	int                                       getMinDamage         (void) const;
	float                                     getWoundChance       (void) const;
	virtual void                              getAttributes        (const NetworkId & playerId, AttributeVector &data) const;
	virtual void                              getAttributes        (AttributeVector &data) const;

	float                               getAudibleRange       (void) const;
	float                               getMinRange           (void) const;
	float                               getMaxRange           (void) const;
	int                                 getAccuracy           (void) const;

	float                               getDamageRadius       (void) const;

	int                                 getAttackCost         (void) const;

	float                               getAttackTime(void) const;
	void                                setAttackTime(float time);
	
	void                                setMinRange          (float range);
	void                                setMaxRange          (float range);
	void                                setAccuracy          (int value);
	
	void                                setDamageRadius      (float radius);

	void                                setMinDamage         (int damage);
	void                                setMaxDamage         (int damage);
	void                                setWoundChance       (float chance);

	void                                setAttackCost        (int cost);

	void								setDamageType        (int type);
	void								setElementalType     (int type);
	void                                setElementalValue    (int value);

	void                                setAsDefaultWeapon(bool isDefault);

	virtual bool                        canDestroy() const;
	virtual bool                        onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);

protected:

	virtual void initializeFirstTimeObject();
	virtual void onLoadedFromDatabase();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

private:
  	WeaponObject();
	WeaponObject(const WeaponObject& rhs);
	WeaponObject&	operator=(const WeaponObject& rhs);

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	Archive::AutoDeltaVariable<int>    m_minDamage;
	Archive::AutoDeltaVariable<int>    m_maxDamage;
	Archive::AutoDeltaVariable<float>  m_attackSpeed;
	Archive::AutoDeltaVariable<float>  m_woundChance;
	Archive::AutoDeltaVariable<float>  m_minRange;
	Archive::AutoDeltaVariable<float>  m_maxRange;
	Archive::AutoDeltaVariable<int>    m_accuracy;
	Archive::AutoDeltaVariable<float>  m_damageRadius;
	Archive::AutoDeltaVariable<int>    m_attackCost;
	Archive::AutoDeltaVariable<bool>   m_isDefaultWeapon;
	Archive::AutoDeltaVariable<int>    m_damageType;
	Archive::AutoDeltaVariable<int>    m_elementalType;
	Archive::AutoDeltaVariable<int>    m_elementalValue;
	Archive::AutoDeltaVariable<int>    m_weaponType;
};


inline float WeaponObject::getAttackTime(void) const
{
	return m_attackSpeed.get();
}

inline void WeaponObject::setAttackTime(float time)
{
	m_attackSpeed = time;
}

inline void WeaponObject::setMinDamage(int damage) 
{
	m_minDamage = damage;
}

inline void WeaponObject::setMaxDamage(int damage)
{
	m_maxDamage = damage;
}

inline float WeaponObject::getWoundChance(void) const
{
	return m_woundChance.get();
}

inline void WeaponObject::setWoundChance(float chance)
{
	m_woundChance = chance;
}
inline int WeaponObject::getAccuracy(void) const
{
	return m_accuracy.get();
}

inline void WeaponObject::setAccuracy(int value)
{
	m_accuracy = value;
}

inline float WeaponObject::getMinRange(void) const
{
	return m_minRange.get();
}

inline float WeaponObject::getMaxRange(void) const
{
	return m_maxRange.get();
}

inline void WeaponObject::setMinRange(float range)
{
	m_minRange = range;
}

inline void WeaponObject::setMaxRange(float range)
{
	m_maxRange = range;
}

inline float WeaponObject::getDamageRadius(void) const
{
	return m_damageRadius.get();
}

inline void WeaponObject::setDamageRadius(float radius)
{
	m_damageRadius = radius;
}

inline int WeaponObject::getAttackCost(void) const
{
	return m_attackCost.get();
}

inline void WeaponObject::setAttackCost(int cost)
{
	m_attackCost = cost;
}

inline ServerWeaponObjectTemplate::DamageType WeaponObject::getDamageType(void) const
{
	ServerWeaponObjectTemplate::DamageType value = static_cast<ServerWeaponObjectTemplate::DamageType>(m_damageType.get());
	return value;
}

inline void WeaponObject::setDamageType(int type)
{
	m_damageType = type;
}

inline ServerWeaponObjectTemplate::DamageType WeaponObject::getElementalType(void) const
{
	ServerWeaponObjectTemplate::DamageType value = static_cast<ServerWeaponObjectTemplate::DamageType>(m_elementalType.get());
	return value;
}

inline void WeaponObject::setElementalType(int type)
{
	m_elementalType = type;
}

inline int WeaponObject::getElementalValue(void) const
{
	return m_elementalValue.get();
}

inline void WeaponObject::setElementalValue(int value)
{
	m_elementalValue= value;
}

#endif	// _INCLUDED_WeaponObject_H
