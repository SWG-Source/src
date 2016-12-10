//======================================================================
//
// ShipComponentDataBridge.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataBridge_H
#define INCLUDED_ShipComponentDataBridge_H

//======================================================================

#include "serverGame/ServerShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataBridge : public ServerShipComponentData
{
public:

	explicit ShipComponentDataBridge (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataBridge ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;

	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ShipComponentDataBridge (const ShipComponentDataBridge & rhs);
	ShipComponentDataBridge & operator= (const ShipComponentDataBridge & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.
};

//======================================================================

#endif
