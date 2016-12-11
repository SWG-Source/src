//======================================================================
//
// ServerShipComponentData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerShipComponentData_H
#define INCLUDED_ServerShipComponentData_H

//======================================================================

#include "sharedGame/ShipComponentData.h"

//----------------------------------------------------------------------

class ServerShipComponentData : public ShipComponentData
{
public:

	explicit ServerShipComponentData (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ServerShipComponentData ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;
	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ServerShipComponentData (const ServerShipComponentData & rhs);
	ServerShipComponentData & operator= (const ServerShipComponentData & rhs);
};

//======================================================================

#endif
