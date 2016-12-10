//======================================================================
//
// ShipChassis.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipChassis_H
#define INCLUDED_ShipChassis_H

//======================================================================

#include "sharedGame/ShipChassisSlotType.h"
#include "sharedFoundation/PersistentCrcString.h"

class ShipChassisSlot;

//----------------------------------------------------------------------

class ShipChassis
{
public:

	typedef std::vector<ShipChassisSlot> SlotVector;
	typedef std::set<ShipChassisSlot> SlotSet;
	typedef std::vector<int> IntVector;
	typedef std::vector<std::string> StringVector;

	static void install ();
	static void remove  ();

	static void load();
	static bool save(std::string const & filename);

	static void setUseWritableChassis(bool onlyUseThisForTools);

	typedef std::vector<PersistentCrcString *> PersistentCrcStringVector;

	static PersistentCrcStringVector const &   getShipChassisCrcVector         ();

	static StringVector & getSortedNameList(StringVector & chassisNames);
	static char const * const getFilename();

	static ShipChassis const * findShipChassisByName           (CrcString const & chassisName);
	static ShipChassis const * findShipChassisByCrc            (uint32 chassisCrc);
	static bool                       isSlotTargetable      (uint32 chassisCrc, ShipChassisSlotType::Type shipChassisSlotType);
	static ShipChassisSlotType::Type  getPreviousTargetableSlot (uint32 chassisCrc, ShipChassisSlotType::Type currentlyTargetedSlotType);
	static ShipChassisSlotType::Type  getNextTargetableSlot (uint32 chassisCrc, ShipChassisSlotType::Type currentlyTargetedSlotType);

	CrcString const &          getName               () const;
	uint32                     getCrc                () const;

	SlotVector const &         getSlots              () const;
	ShipChassisSlot const *    getSlot               (ShipChassisSlotType::Type shipChassisSlotType) const;

	ShipChassisSlotType::Type  getSlotPowerPriority  (int slotIndex) const;
	std::string const &        getFlyBySound         () const;
	void                       setFlyBySound         (std::string const & flyBySound);
	bool isSlotTargetable(int chassisSlotType) const;

	ShipChassisSlotType::Type  getPreviousTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const;
	ShipChassisSlotType::Type  getNextTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const;

	float getWingOpenSpeedFactor() const;

	std::string const & getHitSoundGroup() const;

protected:

	ShipChassis ();
	ShipChassis(ShipChassis const & rhs);
	virtual ~ShipChassis ();

	virtual bool               setName               (CrcString const & name);
	virtual void               addSlot               (ShipChassisSlot const & slot);
	virtual void               setSlotTargetable     (int chassisSlotType, bool targetable);
	virtual void               removeSlot(int chassisSlotType);
	virtual void               setFlybySound(std::string const & flybySound);
	ShipChassisSlot * getSlot(ShipChassisSlotType::Type shipChassisSlotType);
	virtual bool addChassis(bool doSort);
	virtual bool removeChassis();
	virtual void setWingOpenSpeedFactor(float wingOpenSpeedFactor);
	virtual void setHitSoundGroup(std::string const & hitSoundGroup);

private:

	ShipChassis & operator= (const ShipChassis & rhs);

private:

	PersistentCrcString          m_name;
	SlotVector *                 m_slots;
	IntVector *                  m_targetableSlots;
	std::string                  m_flybySound;
	float                        m_wingOpenSpeedFactor;
	std::string m_hitSoundGroup;
};

//----------------------------------------------------------------------

inline CrcString const & ShipChassis::getName               () const
{
	return m_name;
}

//----------------------------------------------------------------------

inline uint32 ShipChassis::getCrc                () const
{
	return m_name.getCrc ();
}

//----------------------------------------------------------------------

inline ShipChassis::SlotVector const &    ShipChassis::getSlots              () const
{
	return *m_slots;
}

//----------------------------------------------------------------------

inline std::string const & ShipChassis::getFlyBySound() const
{
	return m_flybySound;
}

//----------------------------------------------------------------------

inline void ShipChassis::setFlyBySound(std::string const & flyBySound)
{
	m_flybySound = flyBySound;
}

//======================================================================

#endif
