//======================================================================
//
// ShipChassisWritable.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipChassisWritable.h"

#include "sharedGame/ShipChassisSlot.h"
#include "sharedMessageDispatch/Transceiver.h"

//======================================================================

namespace ShipChassisWritableNamespace 
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<bool const &, ShipChassisWritable::Messages::ChassisListChanged> chassisListChanged;
		MessageDispatch::Transceiver<ShipChassisWritable::Messages::ChassisChanged::Payload const &, ShipChassisWritable::Messages::ChassisChanged> chassisChanged;
	}
}

using namespace ShipChassisWritableNamespace;

//----------------------------------------------------------------------

ShipChassisWritable::ShipChassisWritable() :
ShipChassis()
{
}

//----------------------------------------------------------------------

ShipChassisWritable::ShipChassisWritable(ShipChassis const & rhs, std::string const & name) :
ShipChassis(rhs)
{
	IGNORE_RETURN(ShipChassis::setName(PersistentCrcString(name.c_str(), true)));
}

//----------------------------------------------------------------------

ShipChassisWritable::~ShipChassisWritable()
{
}

//----------------------------------------------------------------------

bool ShipChassisWritable::setName(CrcString const & name)
{
	if (ShipChassis::setName(name))
	{
		if (nullptr != findShipChassisByCrc(name.getCrc()))
			Transceivers::chassisListChanged.emitMessage(true);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void ShipChassisWritable::addSlot(ShipChassisSlot const & slot)
{
	ShipChassis::addSlot(slot);
	notifyChanged();
}

//----------------------------------------------------------------------

void ShipChassisWritable::setSlotTargetable(int chassisSlotType, bool targetable)
{
	ShipChassisSlot * const chassisSlot = getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));

	if (nullptr == chassisSlot)
	{
		WARNING(true, ("ShipChassisWritable::setSlotTargetable() invalid slot"));
	}
	else
		chassisSlot->setTargetable(targetable);

	ShipChassis::setSlotTargetable(chassisSlotType, targetable);
	notifyChanged();
}

//----------------------------------------------------------------------

void ShipChassisWritable::setFlybySound(std::string const & flybySound)
{
	ShipChassis::setFlybySound(flybySound);
	notifyChanged();
}

//----------------------------------------------------------------------

ShipChassisSlot * ShipChassisWritable::getSlot(ShipChassisSlotType::Type shipChassisSlotType)
{
	return ShipChassis::getSlot(shipChassisSlotType);
}

//----------------------------------------------------------------------

bool ShipChassisWritable::addChassis(bool doSort)
{
	if (ShipChassis::addChassis(doSort))
	{
		Transceivers::chassisListChanged.emitMessage(true);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipChassisWritable::removeChassis()
{
	if (ShipChassis::removeChassis())
	{
		Transceivers::chassisListChanged.emitMessage(true);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool ShipChassisWritable::canAcceptComponent(ShipComponentDescriptor const & shipComponentDescriptor) const
{
	SlotVector const & sv = getSlots();

	for (SlotVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
	{
		ShipChassisSlot const & shipSlot = *it;

		if (shipSlot.canAcceptComponent(shipComponentDescriptor))
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

void ShipChassisWritable::notifyChanged() const
{
	Transceivers::chassisChanged.emitMessage(*this);
}

//----------------------------------------------------------------------

void ShipChassisWritable::setWingOpenSpeedFactor(float const wingOpenSpeedFactor)
{
	ShipChassis::setWingOpenSpeedFactor(wingOpenSpeedFactor);
	notifyChanged();
}

//----------------------------------------------------------------------

void ShipChassisWritable::setHitSoundGroup(std::string const & hitSoundGroup)
{
	ShipChassis::setHitSoundGroup(hitSoundGroup);
	notifyChanged();
}

//======================================================================
