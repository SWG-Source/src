//======================================================================
//
// ShipComponentDataModification.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataModification_H
#define INCLUDED_ShipComponentDataModification_H

//======================================================================

#include "serverGame/ServerShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataModification : public ServerShipComponentData
{
public:

	explicit ShipComponentDataModification (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataModification ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;

	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ShipComponentDataModification (const ShipComponentDataModification & rhs);
	ShipComponentDataModification & operator= (const ShipComponentDataModification & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.
};

//======================================================================

#endif
