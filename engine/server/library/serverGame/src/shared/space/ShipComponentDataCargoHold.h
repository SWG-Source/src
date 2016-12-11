//======================================================================
//
// ShipComponentDataCargoHold.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataCargoHold_H
#define INCLUDED_ShipComponentDataCargoHold_H

//======================================================================

#include "serverGame/ServerShipComponentData.h"
#include "serverGame/ShipObject.h"

//----------------------------------------------------------------------

class ShipComponentDataCargoHold : public ServerShipComponentData
{
public:

	explicit ShipComponentDataCargoHold (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataCargoHold ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;

	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ShipComponentDataCargoHold (const ShipComponentDataCargoHold & rhs);
	ShipComponentDataCargoHold & operator= (const ShipComponentDataCargoHold & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	int m_cargoHoldContentsMaximum;
	int m_cargoHoldContentsCurrent;

	ShipObject::NetworkIdIntMap m_cargoHoldContents;
};

//======================================================================

#endif
