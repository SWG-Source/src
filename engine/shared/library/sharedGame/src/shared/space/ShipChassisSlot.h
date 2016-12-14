//======================================================================
//
// ShipChassisSlot.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipChassisSlot_H
#define INCLUDED_ShipChassisSlot_H

//======================================================================

#include "sharedGame/ShipChassisSlotType.h"
#include "sharedFoundation/PersistentCrcString.h"

class ShipComponentDescriptor;

//----------------------------------------------------------------------

class ShipChassisSlot
{
public:
	typedef std::vector<CrcString *> CompatibilityVector;

	ShipChassisSlot (ShipChassisSlotType::Type slotType, std::string const & compatibilities, int hitWeight, bool targetable);
	ShipChassisSlot ();
	~ShipChassisSlot ();

	ShipChassisSlotType::Type   getSlotType () const;
	CompatibilityVector const & getCompatibilities () const;
	std::string const getCompatibilityString () const;

	bool canAcceptComponent     (ShipComponentDescriptor const & shipComponentDescriptor) const;
	bool canAcceptComponentType (int shipComponentType) const;
	bool canAcceptCompatibility (CrcString const & compatibility) const;
	int  getHitWeight           () const;
	bool isTargetable           () const;

	Unicode::String getLocalizedSlotName () const;
	Unicode::String getLocalizedSlotDescription () const;

	ShipChassisSlot (const ShipChassisSlot & rhs);
	ShipChassisSlot & operator= (const ShipChassisSlot & rhs);

	std::string const getMatchingCompatibilityString(ShipComponentDescriptor const & shipComponentDescriptor) const;

public:

	/**
	* The following methods are for editor use only
	*/

	void setTargetable(bool targetable);
	void setHitWeight(int hitWeight);
	void setCompatibilities(std::string const & compatibilities);
	void removeCompatibility(std::string const & compat);
	void addCompatibility(std::string const & compat);

private:

	void unpackCompatibilities (std::string const & compatibilities);
	Unicode::String getLocalizedSlotInformation (std::string const & stringTableGeneral, std::string const & stringTableSpecific, 
		bool allowCompoundInfo = false, const std::string & compoundSeperator = "") const;

private:
	
	ShipChassisSlotType::Type   m_slotType;
	CompatibilityVector *       m_compatibilities;
	int                         m_hitWeight;
	bool                        m_targetable;
};

//----------------------------------------------------------------------

inline ShipChassisSlotType::Type ShipChassisSlot::getSlotType () const
{
	return m_slotType;
}

//----------------------------------------------------------------------

inline ShipChassisSlot::CompatibilityVector const & ShipChassisSlot::getCompatibilities () const
{
	return *m_compatibilities;
}

//----------------------------------------------------------------------

inline int ShipChassisSlot::getHitWeight           () const
{
	return m_hitWeight;
}

//----------------------------------------------------------------------

inline bool ShipChassisSlot::isTargetable           () const
{
	return m_targetable;
}

//======================================================================

#endif
