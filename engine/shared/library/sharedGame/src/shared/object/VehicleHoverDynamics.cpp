//======================================================================
//
// VehicleHoverDynamics.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/VehicleHoverDynamics.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedGame/HoverPlaneHelper.h"
#include "sharedMath/AxialBox.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedTerrain/TerrainObject.h"
#include <algorithm>

//======================================================================

namespace
{
	bool ms_useDamping = true;
	bool ms_useFlattening = true;
	bool ms_showDebugVehicleBoxes = false;
	bool ms_useLookAhead = true;
	bool ms_lockLookAhead = false;
	bool ms_showCrumbs    = false;

	const float ms_lookAheadTime = 0.5f;

	void customizationDataModifiedCallback (const CustomizationData &cd, const void *context)
	{
		reinterpret_cast<const VehicleHoverDynamics *>(context)->onCustomizationDataModified (cd);
	}

	//----------------------------------------------------------------------

	class CdVariableData
	{
	public:
		std::string path;
		float conversionFactor;

		CdVariableData (const std::string & _path, float _conversionFactor) :
		path (_path),
		conversionFactor (_conversionFactor)
		{
		}
	};

	//----------------------------------------------------------------------

	namespace CdVariables
	{
		const CdVariableData speedMin    ("/private/index_speed_min",              RECIP (10.0f));
		const CdVariableData speedMax    ("/private/index_speed_max",              RECIP (10.0f));
		const CdVariableData turnRateMin ("/private/index_turn_rate_min",          RECIP (1.0f) * PI_OVER_180);  // comes down in degrees, must convert to radians
		const CdVariableData turnRateMax ("/private/index_turn_rate_max",          RECIP (1.0f) * PI_OVER_180);  // comes down in degrees, must convert to radians
		const CdVariableData accelMin    ("/private/index_accel_min",              RECIP (10.0f));
		const CdVariableData accelMax    ("/private/index_accel_max",              RECIP (10.0f));
		const CdVariableData decel       ("/private/index_decel",                  RECIP (10.0f));
		const CdVariableData slopeMod    ("/private/index_slope_mod",              RECIP (10.0f));
		const CdVariableData dampRoll    ("/private/index_damp_roll",              RECIP (10.0f));
		const CdVariableData dampPitch   ("/private/index_damp_pitch",             RECIP (10.0f));
		const CdVariableData dampHeight  ("/private/index_damp_height",            RECIP (10.0f));
		const CdVariableData glide       ("/private/index_glide",                  RECIP (10.0f));
		const CdVariableData banking     ("/private/index_banking",                RECIP (1.0f) * PI_OVER_180);  // comes down in degrees, must convert to radians
		const CdVariableData hoverHeight ("/private/index_hover_height",           RECIP (10.0f));
		const CdVariableData autoLevel   ("/private/index_auto_level",             RECIP (100.0f));
		const CdVariableData strafe      ("/private/index_strafe",                 RECIP (1.0f));
	}

	//----------------------------------------------------------------------
	
	class CdLookupData
	{
	public:
		mutable float * val;
		CdVariableData  variableData;
		
		CdLookupData (float * _val, const CdVariableData & _variableData) :
		val (_val),
		variableData (_variableData)
		{
		}
	};

	//----------------------------------------------------------------------
}

//----------------------------------------------------------------------

VehicleHoverDynamics::VehicleHoverDynamics (Object* newOwner, float yaw, float hoverHeight) :
Dynamics                   (newOwner),
m_currentSpeed             (0.0f),
m_speedMin                 (0.0f),
m_speedMax                 (0.0f),
m_turnRateMin              (0.0f),
m_turnRateMax              (0.0f),
m_accelMax                 (0.0f),
m_accelMin                 (0.0f),
m_canStrafe                (false),
m_dampFactorRoll           (2.0f),
m_dampFactorPitch          (3.0f),
m_dampFactorGlide          (4.0f),
m_glideFactorMoving        (2.5f),
m_pitchFactorAccel         (0.0f),
m_pitchFactorDecel         (0.0f),
m_rollFactorTurn           (PI_OVER_4),
m_targetRoll               (0.0f),
m_targetPitch              (0.0f),
m_roll                     (0.0f),
m_pitch                    (0.0f),
m_targetY_w                (0.0f),
m_lastY_w                  (0.0f),
m_lastFrameK_w             (),
m_hoverHeight              (hoverHeight),
m_hoverHeightStopped       (0.1f),
m_baseTransform_o2p        (),
m_autoLevellingForce       (0.50f),
m_lastTurnDeltaRoll        (0.0f),
m_lastAccelDeltaPitch      (0.0f),
m_customizationData        (0),
m_hasAlteredCount          (0),
m_customizationDataChanged (0),
m_turnRatePercentLastFrame (0.0f)
{
	Object * const parent = newOwner->getParent ();
	if (parent)
	{
		CustomizationDataProperty * const cdprop = safe_cast<CustomizationDataProperty *>(parent->getProperty (CustomizationDataProperty::getClassPropertyId()));
		if (cdprop)
		{
			m_customizationData = cdprop->fetchCustomizationData ();

			if (m_customizationData)
				m_customizationData->registerModificationListener (&customizationDataModifiedCallback, this);
		}
	}

	WARNING (!m_customizationData, ("VehicleHoverDynamics parent customization data not found, physics will be borked"));

	static bool installed = false;
	if (!installed)
	{
		installed = true;
		DebugFlags::registerFlag (ms_showDebugVehicleBoxes,   "VehicleHoverDynamics", "debugBoxes");
		DebugFlags::registerFlag (ms_useLookAhead,            "VehicleHoverDynamics", "lookAhead");
		DebugFlags::registerFlag (ms_lockLookAhead,           "VehicleHoverDynamics", "lockLookAhead");
		DebugFlags::registerFlag (ms_useDamping,              "VehicleHoverDynamics", "damping");
		DebugFlags::registerFlag (ms_useFlattening,           "VehicleHoverDynamics", "flattening");
		DebugFlags::registerFlag (ms_showCrumbs,              "VehicleHoverDynamics", "showCrumbs");
	}

	m_baseTransform_o2p = Transform::identity;
	m_baseTransform_o2p.yaw_l (yaw);
}

//----------------------------------------------------------------------

VehicleHoverDynamics::~VehicleHoverDynamics ()
{
	if (m_customizationData)
	{
		m_customizationData->deregisterModificationListener (&customizationDataModifiedCallback, this);
		m_customizationData->release ();
	}
}

//----------------------------------------------------------------------

float VehicleHoverDynamics::alter(float elapsedTime)
{
	if (elapsedTime <= 0.0f)
		return AlterResult::cms_keepNoAlter;

	if (m_hasAlteredCount++ == 2)
		m_customizationDataChanged = true;

	if (m_customizationDataChanged)
	{
		readParamsFromCustomizationData ();
	}
	
	if (m_speedMax <= 0.0f || m_speedMax - m_speedMin <= 0.0f)
		return AlterResult::cms_keepNoAlter;

	const Object * const motorObject = getOwner ()->getParent ();
	Object * const vehicleObject     = getOwner ();
	const Vector & motorFrameK_w     = motorObject->getObjectFrameK_w ();
	
	const float speedPercent     = std::max(0.0f, std::min (1.0f, (m_currentSpeed - m_speedMin) / (m_speedMax - m_speedMin)));

	//-- speed glide factor
	//-- 1.0 at motionless
	//-- 0.1 at full speed
	const float speedGlideFactor = 1.0f / (1.0f + (speedPercent * m_glideFactorMoving));
	
	//-- change in heading increases roll/pitch/glide interpolation rates
	const float frameKDot                = motorFrameK_w.dot (m_lastFrameK_w);	
//	const float deltaHeading             = ((1.0f - frameKDot) * 0.5f) * PI; //approximate angle to avoid calling acos()
	const float deltaHeading = (frameKDot >= 1.0f) ? 0.0f : ((frameKDot <= -1.0f) ? PI : acos (frameKDot));
	const float turnRateLastFrame        = deltaHeading / elapsedTime;

	const float maximumTurnRate = getTurnRateForSpeedRadians (m_currentSpeed);
	m_turnRatePercentLastFrame  = maximumTurnRate > 0.0f ? (turnRateLastFrame / maximumTurnRate) : 0.0f;
	const float headingChangeFactor      = 1.0f;// + m_turnRatePercentLastFrame;

	const float timeFactorRoll  = std::min (1.0f, elapsedTime * m_dampFactorRoll  * headingChangeFactor);
	const float timeFactorPitchUp = std::min (1.0f, elapsedTime * m_dampFactorPitch * headingChangeFactor * 2.0f);
	const float timeFactorPitchDown = std::min (1.0f, elapsedTime * m_dampFactorPitch * headingChangeFactor);
	const float timeFactorGlide = std::min (1.0f, elapsedTime * m_dampFactorGlide * speedGlideFactor);
	
	//-- compute acceleration to pitch the vehicle appropriately
	float accelDeltaPitch = 0.0f;
	const float maximumAccel = getAccelForSpeed (m_currentSpeed);
	if (maximumAccel > 0.0f)
	{
		const float accel = (m_currentSpeed - m_lastSpeed) / elapsedTime;
		
		if (accel != 0.0f)
		{
			const float accelPercent = std::min (1.0f, accel / maximumAccel);

			if (accel > 0.0f)
				accelDeltaPitch = accelPercent * m_pitchFactorAccel * (1.0f - m_turnRatePercentLastFrame) * (1.0f - speedPercent);
			else
				accelDeltaPitch = -accelPercent * m_pitchFactorDecel * (1.0f - m_turnRatePercentLastFrame) * speedPercent;
		}
	}

	const float targetAccelDeltaPitch = accelDeltaPitch;
	accelDeltaPitch = linearInterpolate (m_lastAccelDeltaPitch, accelDeltaPitch, std::min (1.0f, timeFactorPitchDown * 2.0f));
	m_lastAccelDeltaPitch = targetAccelDeltaPitch;
	if (accelDeltaPitch > 0.0f && accelDeltaPitch > m_targetPitch) 
		m_targetPitch = std::max (-PI_OVER_2, std::min (PI_OVER_2, std::min (m_targetPitch + accelDeltaPitch, accelDeltaPitch)));
	else if (accelDeltaPitch < 0.0f && accelDeltaPitch < m_targetPitch) 
		m_targetPitch = std::max (-PI_OVER_2, std::min (PI_OVER_2, std::max (m_targetPitch + accelDeltaPitch, accelDeltaPitch)));

	//-- compute turn rate to roll the vehicle appropriately
	float turnDeltaRoll = 0.0f;
	if (m_turnRatePercentLastFrame > 0.0f && maximumTurnRate > 0.0f)
	{
		const Vector frameKCross = motorFrameK_w.cross (m_lastFrameK_w);
		turnDeltaRoll = m_turnRatePercentLastFrame * m_rollFactorTurn;

		//-- turning right
		if (frameKCross.y < 0.0f)
		{
			m_turnRatePercentLastFrame = -m_turnRatePercentLastFrame;
			turnDeltaRoll = -turnDeltaRoll;
		}

		//-- banking is damped to 25% when not moving
		turnDeltaRoll *= 0.25f + (speedPercent * 0.75f);
	}

	const float targetTurnDeltaRoll = turnDeltaRoll;
	turnDeltaRoll = linearInterpolate (m_lastTurnDeltaRoll, turnDeltaRoll, std::min (1.0f, timeFactorRoll * 2.0f));
	m_lastTurnDeltaRoll = targetTurnDeltaRoll;
	m_targetRoll = std::max (-PI_OVER_2, std::min (PI_OVER_2, m_targetRoll + turnDeltaRoll));

	m_distancePopupThisFrame = 0.0f;

	//-- interpolate the roll, pitch, and glide height
	if (ms_useDamping)
	{
		m_roll   = m_roll   + (m_targetRoll   - m_roll)   * timeFactorRoll;

		if (m_targetPitch < m_pitch)
			m_pitch  = m_pitch  + (m_targetPitch  - m_pitch)  * timeFactorPitchUp;
		else
			m_pitch  = m_pitch  + (m_targetPitch  - m_pitch)  * timeFactorPitchDown;
		
		//const float desiredHoverHeight = linearInterpolate (m_hoverHeightStopped, m_hoverHeight, speedPercent);
		const float desiredHoverHeight = m_hoverHeight;
		const float vehicle_y_w = m_lastY_w;
		m_distancePopupThisFrame = m_targetY_w - vehicle_y_w;
		float y = std::max (vehicle_y_w, m_targetY_w);
		const float localTargetY = m_targetY_w + desiredHoverHeight;
		y = y + (localTargetY - y) * timeFactorGlide;
		vehicleObject->move_p (Vector::unitY * (y - vehicleObject->getPosition_w ().y));
		m_lastY_w = y;
	}
				
	vehicleObject->resetRotateTranslate_o2p ();
	vehicleObject->setTransform_o2p (m_baseTransform_o2p);
	vehicleObject->move_p (Vector::unitY * (m_lastY_w - vehicleObject->getPosition_w ().y));
	
	float lookAheadDistance = ms_useLookAhead ? m_currentSpeed * ms_lookAheadTime: 0.0f;
	if (ms_lockLookAhead)
		lookAheadDistance = m_speedMax * ms_lookAheadTime;

	const Vector lookAhead        = motorFrameK_w * lookAheadDistance;
	
	float deltaRoll  = 0.0f;
	float deltaPitch = 0.0f;
	
	//--
	//-- compute the desired change in roll & pitch
	//--

	if (HoverPlaneHelper::findMinimumHoverPlane (*vehicleObject, deltaRoll, deltaPitch, lookAhead))
	{
		m_targetRoll  = deltaRoll;
		m_targetPitch = deltaPitch;
		
		if (ms_showDebugVehicleBoxes && ms_useLookAhead)
			showDebugBoxHoverPlane (lookAhead);
		
		const float vehicle_y_w = m_lastY_w;
		
		vehicleObject->resetRotateTranslate_o2p ();
		vehicleObject->setTransform_o2p (m_baseTransform_o2p);
		
		//-- auto-flatten the roll & pitch if we are moving slowly or stopped
		if (ms_useFlattening)
		{
			m_targetRoll  = linearInterpolate (m_targetRoll,  0.0f, sqr (sqr(1.0f - speedPercent)) * m_autoLevellingForce);
			m_targetPitch = linearInterpolate (m_targetPitch, 0.0f, sqr (sqr(1.0f - speedPercent)) * m_autoLevellingForce);
		}
		
		if (!ms_useDamping)
		{
			m_roll  = m_targetRoll;
			m_pitch = m_targetPitch;
		}
		
		vehicleObject->pitch_o (m_pitch);
		vehicleObject->roll_o  (m_roll);	
		vehicleObject->move_p (Vector::unitY * (m_lastY_w - vehicleObject->getPosition_w ().y));

		const Vector & vehicleFrameK_w = vehicleObject->getObjectFrameK_w ();
		Vector lookAheadVehicle  = vehicleFrameK_w * lookAheadDistance;
		lookAheadVehicle = m_baseTransform_o2p.rotateTranslate_p2l (lookAheadVehicle);
	
		//-- 
		//-- determine the hover height target
		//-- 
		
		if (HoverPlaneHelper::findMinimumHoverHeight (*vehicleObject, m_targetY_w, lookAheadVehicle))
		{
			if (ms_showDebugVehicleBoxes && ms_useLookAhead)
				showDebugBoxHeight (lookAheadVehicle);
			
			m_distancePopupThisFrame += m_targetY_w - vehicle_y_w;

			float y = std::max (vehicle_y_w, m_targetY_w);
			
			if (!ms_useDamping)
			{
				const float desiredHoverHeight = m_hoverHeight;
//				const float desiredHoverHeight = linearInterpolate (m_hoverHeightStopped, m_hoverHeight, speedPercent);
				m_targetY_w += desiredHoverHeight;
				y = m_targetY_w ;
			}

			const float deltaY = (y - vehicleObject->getPosition_w ().y);
			vehicleObject->move_p (Vector::unitY * deltaY);
			m_lastY_w = y;
			
			m_vehicleMotorOffsetY = m_lastY_w - motorObject->getPosition_w ().y;
		}
	}
	
	if (ms_showCrumbs)
		updateCrumbTrail ();

	m_lastFrameK_w = motorFrameK_w;
	m_lastSpeed = m_currentSpeed;

	return AlterResult::cms_keepNoAlter;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::showDebugBoxHoverPlane (const Vector & ) const
{
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::showDebugBoxHeight     (const Vector & ) const
{
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::updateCrumbTrail () const
{
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setDampRoll    (float f)
{
	m_dampFactorRoll = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setDampPitch   (float f)
{
	m_dampFactorPitch = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setDampHeight  (float f)
{
	m_dampFactorGlide = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setGlide       (float f)
{
	m_glideFactorMoving = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setPitchAccel  (float f)
{
	m_pitchFactorAccel = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setPitchDecel  (float f)
{
	m_pitchFactorDecel = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setRollTurn    (float f)
{
	m_rollFactorTurn = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setAutoLevel   (float f)
{
	m_autoLevellingForce = f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::setHoverHeight (float f)
{
	m_hoverHeight = f;
}

//----------------------------------------------------------------------

float VehicleHoverDynamics::getTurnRateForSpeedRadians (float currentSpeed) const
{
	if (m_speedMax <= 0.0f || m_speedMax - m_speedMin <= 0.0f)
		return 0.0f;

	const float speedPercent = std::max (0.0f, std::min (1.0f, (currentSpeed - m_speedMin) / (m_speedMax - m_speedMin)));

	return m_turnRateMin + (m_turnRateMax - m_turnRateMin) * speedPercent;
}

//----------------------------------------------------------------------

float VehicleHoverDynamics::getTurnRateForSpeedDegrees (float currentSpeed) const
{
	return convertRadiansToDegrees (getTurnRateForSpeedRadians (currentSpeed));
}

//----------------------------------------------------------------------

float VehicleHoverDynamics::getAccelForSpeed (float currentSpeed) const
{
	if (m_speedMax <= 0.0f || m_speedMax - m_speedMin <= 0.0f)
		return 0.0f;

	const float speedPercent = std::max (0.0f, std::min (1.0f, (currentSpeed - m_speedMin) / (m_speedMax - m_speedMin)));

	return m_accelMin + (m_accelMax - m_accelMin) * speedPercent;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::readParamsFromCustomizationData (const CustomizationData & cd, 
		float & speedMin,
		float & speedMax,
		float & turnRateMin,
		float & turnRateMax,
		float & accelMin,
		float & accelMax)
{
	float dummy = 0.0f;
	bool dumbool = false;

	readParamsFromCustomizationData (cd,
		speedMin,
		speedMax,
		turnRateMin,
		turnRateMax,
		accelMin,
		accelMax,
		dummy,
		dummy,
		dummy,
		dummy,
		dummy,
		dummy,
		dummy,
		dummy,
		dummy,
		dumbool);
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::readParamsFromCustomizationData (const CustomizationData & cd, 
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
		bool & strafe)
{
	float tempStrafe = 0.0f;
	const CdLookupData s_data [] =
	{
		CdLookupData (&speedMin,               CdVariables::speedMin),
		CdLookupData (&speedMax,               CdVariables::speedMax),
		CdLookupData (&turnRateMin,            CdVariables::turnRateMin),
		CdLookupData (&turnRateMax,            CdVariables::turnRateMax),
		CdLookupData (&accelMin,               CdVariables::accelMin),
		CdLookupData (&accelMax,               CdVariables::accelMax),
		CdLookupData (&decel,                  CdVariables::decel),
		CdLookupData (&slopeMod,               CdVariables::slopeMod),
		CdLookupData (&dampFactorRoll,         CdVariables::dampRoll),
		CdLookupData (&dampFactorPitch,        CdVariables::dampPitch),
		CdLookupData (&dampFactorGlide,        CdVariables::dampHeight),
		CdLookupData (&glideFactorMoving,      CdVariables::glide),
		CdLookupData (&rollFactorTurn,         CdVariables::banking),
		CdLookupData (&hoverHeight,            CdVariables::hoverHeight),
		CdLookupData (&autoLevellingForce,     CdVariables::autoLevel),
		CdLookupData (&tempStrafe,             CdVariables::strafe)
	};

	const int s_data_len = sizeof (s_data) / sizeof (s_data [0]);

	for (int i = 0; i < s_data_len; ++i)
	{
		const CdLookupData & cld   = s_data [i];
		const CdVariableData & cvd = cld.variableData;

		const RangedIntCustomizationVariable * const ricv = dynamic_cast<const RangedIntCustomizationVariable *>(cd.findConstVariable(cvd.path));
		if (ricv)
		{
			const int val = ricv->getValue ();
			const float fval = val * cvd.conversionFactor;
			*cld.val = fval;
		}
		else
			WARNING (true, ("VehicleHoverDynamics customization param [%s] not found", cvd.path.c_str ()));
	}

	strafe = tempStrafe > 0.0f;
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::readParamsFromCustomizationData ()
{
	if (!m_customizationData)
		return;
	
	m_customizationDataChanged = false;

	float m_decel = 0.0f;
	float m_slopeMod = 0.0f;

	readParamsFromCustomizationData (*m_customizationData,
		m_speedMin,
		m_speedMax,
		m_turnRateMin,
		m_turnRateMax,
		m_accelMin,
		m_accelMax,
		m_decel,
		m_slopeMod,
		m_dampFactorRoll,
		m_dampFactorPitch,
		m_dampFactorGlide,
		m_glideFactorMoving,
		m_rollFactorTurn,
		m_hoverHeight,
		m_autoLevellingForce,
		m_canStrafe);
}

//----------------------------------------------------------------------

void VehicleHoverDynamics::onCustomizationDataModified (const CustomizationData & cd) const
{
	if (&cd == m_customizationData)
		m_customizationDataChanged = true;
}

//----------------------------------------------------------------------

/**
* turn rates are passed into this function in degrees
*/

void VehicleHoverDynamics::setInitialParams (CustomizationData & cd,
		float speedMin,
		float speedMax,
		float turnRateMin,
		float turnRateMax,
		float accelMin,
		float accelMax)
{
	turnRateMin = convertDegreesToRadians (turnRateMin);
	turnRateMax = convertDegreesToRadians (turnRateMax);

	const CdLookupData s_data [] =
	{
		CdLookupData (&speedMin,               CdVariables::speedMin),
		CdLookupData (&speedMax,               CdVariables::speedMax),
		CdLookupData (&turnRateMin,            CdVariables::turnRateMin),
		CdLookupData (&turnRateMax,            CdVariables::turnRateMax),
		CdLookupData (&accelMin,               CdVariables::accelMin),
		CdLookupData (&accelMax,               CdVariables::accelMax)
	};

	const int s_data_len = sizeof (s_data) / sizeof (s_data [0]);

	for (int i = 0; i < s_data_len; ++i)
	{
		const CdLookupData & cld   = s_data [i];
		const CdVariableData & cvd = cld.variableData;

		RangedIntCustomizationVariable * const ricv = dynamic_cast<RangedIntCustomizationVariable *>(cd.findVariable(cvd.path));
		if (ricv)
		{
			const float fval = *cld.val;
			const int ival   = static_cast<int>(fval / cvd.conversionFactor);
			ricv->setValue (ival);
		}
		else
			WARNING (true, ("VehicleHoverDynamics customization param [%s] not found", cvd.path.c_str ()));
	}
}

//======================================================================
