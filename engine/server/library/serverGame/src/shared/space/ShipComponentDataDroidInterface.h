//======================================================================
//
// ShipComponentDataDroidInterface.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataDroidInterface_H
#define INCLUDED_ShipComponentDataDroidInterface_H

//======================================================================

#include "serverGame/ServerShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataDroidInterface : public ServerShipComponentData
{
public:

	explicit ShipComponentDataDroidInterface (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataDroidInterface ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;
	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ShipComponentDataDroidInterface (const ShipComponentDataDroidInterface & rhs);
	ShipComponentDataDroidInterface & operator= (const ShipComponentDataDroidInterface & rhs);

public:
	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	float m_droidInterfaceCommandSpeed;
};

//======================================================================

#endif
