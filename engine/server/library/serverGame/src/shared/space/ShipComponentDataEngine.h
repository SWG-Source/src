//======================================================================
//
// ShipComponentDataEngine.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataEngine_H
#define INCLUDED_ShipComponentDataEngine_H

//======================================================================

#include "serverGame/ServerShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataEngine : public ServerShipComponentData
{
public:

	explicit ShipComponentDataEngine (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataEngine ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;

	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (std::vector<std::pair<std::string, Unicode::String> > & data) const;

private:
	ShipComponentDataEngine (const ShipComponentDataEngine & rhs);
	ShipComponentDataEngine & operator= (const ShipComponentDataEngine & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	float m_engineAccelerationRate;
	float m_engineDecelerationRate;
	float m_enginePitchAccelerationRate;
	float m_engineYawAccelerationRate;
	float m_engineRollAccelerationRate;
	float m_enginePitchRateMaximum;
	float m_engineYawRateMaximum;
	float m_engineRollRateMaximum;
	float m_engineSpeedMaximum;
	float m_engineSpeedRotationFactorMaximum;
	float m_engineSpeedRotationFactorMinimum;
	float m_engineSpeedRotationFactorOptimal;
};

//======================================================================

#endif
