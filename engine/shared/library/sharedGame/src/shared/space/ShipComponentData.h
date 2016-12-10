//======================================================================
//
// ShipComponentData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentData_H
#define INCLUDED_ShipComponentData_H

//======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

class ShipComponentDescriptor;
class ShipObject;
class TangibleObject;

//----------------------------------------------------------------------

class ShipComponentData
{
public:

	typedef std::pair<std::string, Unicode::String> AttributePair;
	typedef std::vector<AttributePair>           AttributeVector;

	explicit ShipComponentData (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentData () = 0;

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship) = 0;
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const = 0;
	virtual bool readDataFromComponent (TangibleObject const & component) = 0;
	virtual void writeDataToComponent  (TangibleObject & component) const = 0;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (AttributeVector & data) const;

	ShipComponentDescriptor const &    getDescriptor () const;

private:
	ShipComponentData (const ShipComponentData & rhs);
	ShipComponentData & operator= (const ShipComponentData & rhs);

protected:
	ShipComponentDescriptor const * m_descriptor;
	Unicode::String const           cm_slash;
	std::string const               cm_shipComponentCategory;

public:
	float                           m_armorHitpointsCurrent;
	float                           m_armorHitpointsMaximum;
	float                           m_efficiencyGeneral;
	float                           m_efficiencyEnergy;
	float                           m_energyMaintenanceRequirement;
	float                           m_mass;
	float                           m_hitpointsCurrent;
	float                           m_hitpointsMaximum;
	int                             m_flags;
	Unicode::String                 m_name;
	NetworkId                       m_creator;
};

//----------------------------------------------------------------------

inline ShipComponentDescriptor const & ShipComponentData::getDescriptor () const
{
	return *NON_NULL (m_descriptor);
}

//======================================================================

#endif
