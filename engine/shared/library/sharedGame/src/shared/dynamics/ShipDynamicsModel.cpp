// ======================================================================
//
// ShipDynamicsModel.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipDynamicsModel.h"

#include "sharedGame/ShipObjectInterface.h"
#include "sharedTerrain/TerrainObject.h"

// ======================================================================
// ShipDynamicsModelNamespace
// ======================================================================

namespace ShipDynamicsModelNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool s_clampMovementToZoneVolume = true;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void updateAxis(float & currentRate, float const controlPosition, float const maximumRate, float const acceleration, float const deceleration, float const elapsedTime)
	{
		float const desiredRate = controlPosition * maximumRate;
	
		if (currentRate < desiredRate)
		{
			currentRate += acceleration * elapsedTime;

			if (currentRate > desiredRate)
				currentRate = desiredRate;
		}
		else
			if (currentRate > desiredRate)
			{
				currentRate -= deceleration * elapsedTime;

				if (currentRate < desiredRate)
					currentRate = desiredRate;
			}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void model(Transform & transform, Vector & velocity, float const elapsedTime, float const yawRate, float const pitchRate, float const rollRate, float const speed, float const slideDampener)
	{
		//-- Model yaw
		{
			transform.yaw_l(yawRate * elapsedTime);
		}

		//-- Model pitch
		{
			transform.pitch_l(pitchRate * elapsedTime);
		}

		//-- Model roll
		{
			transform.roll_l(rollRate * elapsedTime);
		}

		//-- Model speed
		{
			if (slideDampener == 0.0f)
			{
				// 0.0f means no sliding at all
				velocity = transform.getLocalFrameK_p() * speed;
			}
			else
			{
				velocity += transform.getLocalFrameK_p() * speed * slideDampener * elapsedTime;
			
				float const magnitudeSquared = velocity.magnitudeSquared();
				if (magnitudeSquared > sqr(speed))
				{
					IGNORE_RETURN(velocity.normalize());
					velocity *= speed;
				}
			}

			Vector newPosition = transform.getPosition_p() + velocity * elapsedTime;
			if (s_clampMovementToZoneVolume)
			{
				float const halfMapWidth = TerrainObject::getConstInstance()->getMapWidthInMeters() * 0.5f;
				float const boundary = 32.f;
				float const coordMinimum = -halfMapWidth + boundary;
				float const coordMaximum = halfMapWidth - boundary;

				newPosition.x = clamp(coordMinimum, newPosition.x, coordMaximum);
				newPosition.y = clamp(coordMinimum, newPosition.y, coordMaximum);
				newPosition.z = clamp(coordMinimum, newPosition.z, coordMaximum);
			}

			transform.setPosition_p(newPosition.x, newPosition.y, newPosition.z);
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ShipDynamicsModelNamespace;

// ======================================================================
// PUBLIC ShipDynamicsModel
// ======================================================================

ShipDynamicsModel::ShipDynamicsModel() :
	m_yawRate(0.f),
	m_pitchRate(0.f),
	m_rollRate(0.f),
	m_speed(0.f),
	m_velocity(),
	m_transform()
{
}

// ----------------------------------------------------------------------

ShipDynamicsModel::~ShipDynamicsModel()
{
}

// ----------------------------------------------------------------------

bool ShipDynamicsModel::getClampMovementToZoneVolume()
{
	return s_clampMovementToZoneVolume;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::setClampMovementToZoneVolume(bool const clampMovement)
{
	s_clampMovementToZoneVolume = clampMovement;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::makeStationary()
{
	m_yawRate = 0.f;
	m_pitchRate = 0.f;
	m_rollRate = 0.f;
	m_speed = 0.f;
	m_velocity.makeZero();
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::stop()
{
	m_speed = 0.f;
	m_velocity.makeZero();
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::model(float const elapsedTime, float const yawPosition, float const pitchPosition, float const rollPosition, float const throttlePosition, ShipObjectInterface const & shipObjectInterface)
{
	if (elapsedTime <= 0.f)
		return;

	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(-1.f, yawPosition, 1.f);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(-1.f, pitchPosition, 1.f);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(-1.f, rollPosition, 1.f);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0.f, throttlePosition, 1.f);

	updateAxis(m_yawRate, yawPosition, shipObjectInterface.getMaximumYaw(), shipObjectInterface.getYawAcceleration(), shipObjectInterface.getYawAcceleration(), elapsedTime);
	updateAxis(m_pitchRate, pitchPosition, shipObjectInterface.getMaximumPitch(), shipObjectInterface.getPitchAcceleration(), shipObjectInterface.getPitchAcceleration(), elapsedTime);
	updateAxis(m_rollRate, rollPosition, shipObjectInterface.getMaximumRoll(), shipObjectInterface.getRollAcceleration(), shipObjectInterface.getRollAcceleration(), elapsedTime);
	updateAxis(m_speed, throttlePosition, shipObjectInterface.getMaximumSpeed(), shipObjectInterface.getSpeedAcceleration(), shipObjectInterface.getSpeedDeceleration(), elapsedTime);

	//-- Modeling is the same as prediction with specified inputs
	predict(elapsedTime, shipObjectInterface);
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::predict(float const elapsedTime, ShipObjectInterface const & shipObjectInterface)
{
	if (elapsedTime <= 0.f)
		return;

	ShipDynamicsModelNamespace::model(m_transform, m_velocity, elapsedTime, m_yawRate, m_pitchRate, m_rollRate, m_speed, shipObjectInterface.getSlideDampener());
}

// ----------------------------------------------------------------------

Transform const & ShipDynamicsModel::getTransform() const
{
	return m_transform;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::setTransform(Transform const & transform)
{
	m_transform = transform;
}

// ----------------------------------------------------------------------

Vector const & ShipDynamicsModel::getVelocity() const
{
	return m_velocity;
}

// ----------------------------------------------------------------------

float ShipDynamicsModel::getSpeed() const
{
	return m_speed;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::setVelocity(Vector const & velocity)
{
	m_velocity = velocity;
	m_speed = m_velocity.magnitude();
}

// ----------------------------------------------------------------------

float ShipDynamicsModel::getYawRate() const
{
	return m_yawRate;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::setYawRate(float const yawRate)
{
	m_yawRate = yawRate;
}

// ----------------------------------------------------------------------

float ShipDynamicsModel::getPitchRate() const
{
	return m_pitchRate;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::setPitchRate(float const pitchRate)
{
	m_pitchRate = pitchRate;
}

// ----------------------------------------------------------------------

float ShipDynamicsModel::getRollRate() const
{
	return m_rollRate;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::setRollRate(float const rollRate)
{
	m_rollRate = rollRate;
}

// ----------------------------------------------------------------------

void ShipDynamicsModel::getApproximateFutureTransform(Transform & transform, Vector & velocity, float const elapsedTime, ShipObjectInterface const & shipObjectInterface) const
{
	transform = m_transform;
	velocity = m_velocity;

	float const timeSlice = 0.1f;

	float remainingTime = elapsedTime;
	while (remainingTime > 0.f)
	{
		float const slicedTime = (remainingTime < timeSlice) ? remainingTime : timeSlice;
		ShipDynamicsModelNamespace::model(transform, velocity, slicedTime, m_yawRate, m_pitchRate, m_rollRate, m_speed, shipObjectInterface.getSlideDampener());

		remainingTime -= slicedTime;
	}
}

// ======================================================================
