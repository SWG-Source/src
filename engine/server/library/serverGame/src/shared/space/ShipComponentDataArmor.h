//======================================================================
//
// ShipComponentDataArmor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataArmor_H
#define INCLUDED_ShipComponentDataArmor_H

//======================================================================

#include "serverGame/ServerShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataArmor : public ServerShipComponentData
{
public:

	explicit ShipComponentDataArmor (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataArmor ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;

	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ShipComponentDataArmor (const ShipComponentDataArmor & rhs);
	ShipComponentDataArmor & operator= (const ShipComponentDataArmor & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.
};

//======================================================================

#endif
