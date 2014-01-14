// ======================================================================
//
// ShipDynamicsModel.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipDynamicsModel_H
#define INCLUDED_ShipDynamicsModel_H

// ======================================================================

#include "sharedMath/Transform.h"

class ShipObjectInterface;

// ======================================================================

class ShipDynamicsModel
{
public:

	ShipDynamicsModel();
	~ShipDynamicsModel();

	static bool getClampMovementToZoneVolume();
	static void setClampMovementToZoneVolume(bool const clampMovement);

	void makeStationary();
	void stop();

	//-- Modeling
	void model(float elapsedTime, float yawPosition, float pitchPosition, float rollPosition, float throttlePosition, ShipObjectInterface const & shipObjectInterface);

	//-- Prediction
	void predict(float elapsedTime, ShipObjectInterface const & shipObjectInterface);

	Transform const & getTransform() const;
	void setTransform(Transform const & transform);

	Vector const & getVelocity() const;
	float getSpeed() const;
	void setVelocity(Vector const & velocity);

	float getYawRate() const;
	void setYawRate(float yawRate);
	float getPitchRate() const;
	void setPitchRate(float pitchRate);
	float getRollRate() const;
	void setRollRate(float rollRate);

	//-- Query where this transform would be with no changes to any inputs if elapsedTime passed
	void getApproximateFutureTransform(Transform & transform, Vector & velocity, float elapsedTime, ShipObjectInterface const & shipObjectInterface) const;

private:

	ShipDynamicsModel(ShipDynamicsModel const &);
	ShipDynamicsModel & operator=(ShipDynamicsModel const &);

private:

	float m_yawRate;
	float m_pitchRate;
	float m_rollRate;
	float m_speed;
	Vector m_velocity;
	Transform m_transform;
};

// ======================================================================

#endif
