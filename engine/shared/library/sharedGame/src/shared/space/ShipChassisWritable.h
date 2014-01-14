//======================================================================
//
// ShipChassisWritable.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipChassisWritable_H
#define INCLUDED_ShipChassisWritable_H

#include "sharedGame/ShipChassis.h"

class ShipComponentDescriptor;

//======================================================================

class ShipChassisWritable : public ShipChassis
{
public:

	class Messages
	{
	public:
		struct ChassisListChanged;
		struct ChassisChanged
		{
			typedef ShipChassisWritable Payload;
		};
	};

	ShipChassisWritable();
	ShipChassisWritable(ShipChassis const & rhs, std::string const & name);

	virtual ~ShipChassisWritable();

	virtual bool setName(CrcString const & name);
	virtual void addSlot(ShipChassisSlot const & slot);
	virtual void setSlotTargetable(int chassisSlot, bool targetable);
	virtual void setFlybySound(std::string const & flybySound);
	virtual void setWingOpenSpeedFactor(float wingOpenSpeedFactor);
	ShipChassisSlot * getSlot(ShipChassisSlotType::Type shipChassisSlotType);

	virtual bool addChassis(bool doSort);
	virtual bool removeChassis();

	bool canAcceptComponent(ShipComponentDescriptor const & shipComponentDescriptor) const;
	void notifyChanged() const;
	void setHitSoundGroup(std::string const & hitSoundGroup);

private:

	ShipChassisWritable(ShipChassisWritable const & rhs);
	ShipChassisWritable& operator=(ShipChassisWritable const & rhs);
};

//======================================================================

#endif
