//======================================================================
//
// ShipComponentDataManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataManager_H
#define INCLUDED_ShipComponentDataManager_H

//======================================================================

class ShipComponentData;
class ShipComponentDescriptor;
class TangibleObject;

//----------------------------------------------------------------------

class ShipComponentDataManager
{
public:

	static ShipComponentData * create (TangibleObject const & component);
	static ShipComponentData * create (ShipComponentDescriptor const & shipComponentDescriptor);

private:
	ShipComponentDataManager ();
	ShipComponentDataManager (const ShipComponentDataManager & rhs);
	ShipComponentDataManager & operator= (const ShipComponentDataManager & rhs);
};

//======================================================================

#endif
