// ======================================================================
//
// VehicleHoverDynamics.h
// copyright 2003, sony online entertainment
//
//
// ======================================================================

#ifndef INCLUDED_VehicleHoverDynamics_H
#define INCLUDED_VehicleHoverDynamics_H

// ======================================================================

#include "sharedObject/Dynamics.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"

class CustomizationData;

// ======================================================================
// Abstract VehicleHoverDynamics entity for an object

class VehicleHoverDynamics : public Dynamics
{
public:

	explicit VehicleHoverDynamics (Object* newOwner, float yaw, float hoverHeight);
	virtual ~VehicleHoverDynamics () = 0;
	
	virtual float alter(float time);
	
	void setDampRoll     (float f);
	void setDampPitch    (float f);
	void setDampHeight   (float f);
	void setGlide        (float f);
	void setPitchAccel   (float f);
	void setPitchDecel   (float f);
	void setRollTurn     (float f);
	void setAutoLevel    (float f);
	void setHoverHeight  (float f);
	void setNumType      (float f);
	
	float getDampRoll    () const;
	float getDampPitch   () const;
	float getDampHeight  () const;
	float getGlide       () const;
	float getPitchAccel  () const;
	float getPitchDecel  () const;
	float getRollTurn    () const;
	float getAutoLevel   () const;
	float getHoverHeight () const;
	float getNumTypes    () const;
	
	float getSpeedMin     () const;
	float getSpeedMax     () const;
	float getTurnRateMin  () const;
	float getTurnRateMax  () const;
	float getTurnRateForSpeedRadians (float currentSpeed) const;
	float getTurnRateForSpeedDegrees (float currentSpeed) const;
	float getAccelMin      () const;
	float getAccelMax      () const;
	float getAccelForSpeed (float currentSpeed) const;

	bool getCanStrafe      () const;

	void onCustomizationDataModified (const CustomizationData & cd) const;

	static void setInitialParams (CustomizationData & cd,
		float speedMin,
		float speedMax,
		float turnRateMin,
		float turnRateMax,
		float accelMin,
		float accelMax);

	static void readParamsFromCustomizationData (const CustomizationData & cd, 
		float & speedMin,
		float & speedMax,
		float & turnRateMin,
		float & turnRateMax,
		float & accelMin,
		float & accelMax,
		float & decel,
		float & slopeMod,
		float & dampFactorRoll,
		float & dampFactorPitch,
		float & dampFactorGlide,
		float & glideFactorMoving,
		float & rollFactorTurn,
		float & hoverHeight,
		float & autoLevellingForce,
		bool & strafe);

	static void readParamsFromCustomizationData (const CustomizationData & cd, 
		float & speedMin,
		float & speedMax,
		float & turnRateMin,
		float & turnRateMax,
		float & accelMin,
		float & accelMax);

protected:

	float m_currentSpeed;
	float m_speedMin;
	float m_speedMax;
	float m_turnRateMin;
	float m_turnRateMax;
	float m_accelMax;
	float m_accelMin;

//	float m_maximumAccel;
//	float m_maximumTurnRate;
	float m_vehicleMotorOffsetY;
	float m_distancePopupThisFrame;
	bool m_canStrafe;

	virtual void showDebugBoxHoverPlane (const Vector & lookAhead) const;
	virtual void showDebugBoxHeight     (const Vector & lookAhead) const;
	virtual void updateCrumbTrail       () const;

	float getTurnRatePercentLastFrame   () const;

private:

	// disable these
	VehicleHoverDynamics ();
	VehicleHoverDynamics (const VehicleHoverDynamics&);
	VehicleHoverDynamics& operator= (const VehicleHoverDynamics&);
	
	void readParamsFromCustomizationData ();

private:

	float m_dampFactorRoll;
	float m_dampFactorPitch;
	float m_dampFactorGlide;
	float m_glideFactorMoving;
	float m_pitchFactorAccel;
	float m_pitchFactorDecel;
	float m_rollFactorTurn;

	float m_targetRoll;
	float m_targetPitch;
	
	float m_roll;
	float m_pitch;
	
	float m_targetY_w;
	float m_lastY_w;
	
	Vector m_lastFrameK_w;
	
	float m_hoverHeight;
	float m_hoverHeightStopped;

	Transform m_baseTransform_o2p;

	float m_autoLevellingForce;

	float m_lastSpeed;

	float m_lastTurnDeltaRoll;
	float m_lastAccelDeltaPitch;

	CustomizationData * m_customizationData;

	int  m_hasAlteredCount;
	mutable bool m_customizationDataChanged;

	float m_turnRatePercentLastFrame;
};


//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getDampRoll    () const
{
	return m_dampFactorRoll;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getDampPitch   () const
{
	return m_dampFactorPitch;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getDampHeight  () const
{
	return m_dampFactorGlide;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getGlide       () const
{
	return m_glideFactorMoving;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getPitchAccel  () const
{
	return m_pitchFactorAccel;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getPitchDecel  () const
{
	return m_pitchFactorDecel;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getRollTurn    () const
{
	return m_rollFactorTurn;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getAutoLevel   () const
{
	return m_autoLevellingForce;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getHoverHeight () const
{
	return m_hoverHeight;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getSpeedMin () const
{
	return m_speedMin;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getSpeedMax () const
{
	return m_speedMax;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getTurnRateMin  () const
{
	return m_turnRateMin;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getTurnRateMax  () const
{
	return m_turnRateMax;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getAccelMin      () const
{
	return m_accelMin;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getAccelMax      () const
{
	return m_accelMax;
}

//----------------------------------------------------------------------

inline float VehicleHoverDynamics::getTurnRatePercentLastFrame   () const
{
	return m_turnRatePercentLastFrame;
}

//----------------------------------------------------------------------

inline bool VehicleHoverDynamics::getCanStrafe  () const
{
	return m_canStrafe;
}

// ======================================================================

#endif
