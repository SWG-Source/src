// ======================================================================
//
// MovementTable.cpp
// Copyright 2002-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/MovementTable.h"

#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgSharedUtility/Postures.h"
#include "swgSharedUtility/States.h"

#include <map>

// ======================================================================

namespace MovementTableNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<States::Enumerator, float>  StateFloatMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const *const cs_stateRateModifierFilename = "datatables/movement/state_rate_modifiers.iff";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	StateFloatMap  s_stateMovementRateModifiers;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	inline bool hasState(uint64 flags, States::Enumerator state)
	{
		return (flags & States::getStateMask(state)) != 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace MovementTableNamespace;

// ======================================================================

struct MovementTable::rates
{
	Locomotions::Enumerator stationary;
	Locomotions::Enumerator slow;
	Locomotions::Enumerator fast;
	float move;
	float accel;
	float turn;
	float canSeeHeightMod;
};

// ======================================================================

bool                            MovementTable::ms_installed;
MovementTable::eLocomotionSpeed MovementTable::ms_stateMoveLimit[States::NumberOfStates];

// ======================================================================
// class MovementTable: PUBLIC STATIC
// ======================================================================

void MovementTable::install(const std::string &stateFileName)
{
	DEBUG_FATAL(ms_installed, ("MovementTable already installed"));

	//-- Process state locomotion speed limit file.
	{
		DataTable *table = DataTableManager::getTable(stateFileName, true);
		FATAL(!table, ("Couldn't open movement state datatable [%s]", stateFileName.c_str()));

		int i;
		for (i = 0; i < States::NumberOfStates; ++i)
			ms_stateMoveLimit[i] = kFast;

		int const rowCount = table->getNumRows();

		for (i = 0; i < rowCount; ++i)
		{
			States::Enumerator state = static_cast<States::Enumerator>(table->getIntValue("state", i));
			ms_stateMoveLimit[state] = static_cast<eLocomotionSpeed>(table->getIntValue("maxSpeedCategory", i));
		}
		DataTableManager::close(stateFileName);
	}

	//-- Process state movement rate modifiers file.
	{
		s_stateMovementRateModifiers.clear();

		// @todo -TRF- get filename from caller.
		char const *filename = cs_stateRateModifierFilename;

		DataTable *table = DataTableManager::getTable(filename, true);
		FATAL(!table, ("Couldn't open state movement rate modifier datatable [%s].", filename));

		int const rowCount = table->getNumRows();

		for (int i = 0; i < rowCount; ++i)
		{
			States::Enumerator const state        = static_cast<States::Enumerator>(table->getIntValue("state", i));
			float const              rateModifier = table->getFloatValue("movementRateModifier", i);

			IGNORE_RETURN(s_stateMovementRateModifiers.insert(StateFloatMap::value_type(state, rateModifier)));
		}
		DataTableManager::close(filename);
	}

	ms_installed = true;
}

// ======================================================================
// class MovementTable: PUBLIC
// ======================================================================

MovementTable::MovementTable (const std::string &filename, const std::string &templateName) :
	m_postures(new PostureMap)
{
	DataTable * table = DataTableManager::getTable(filename, true);
	UNREF(templateName);
	FATAL(!table, ("Couldn't open movement datatable [%s] for template [%s]", filename.c_str(), templateName.c_str()));

	int x;
	for (x=0; x<Locomotions::NumberOfLocomotions; ++x)
	{
		m_locomotionPostures[x] = Postures::Invalid;
	}

	for (x=0; x<table->getNumRows(); ++x)
	{
		Postures::Enumerator posture = static_cast<Postures::Enumerator>(table->getIntValue("posture", x));
		rates temp;

		temp.stationary = static_cast<Locomotions::Enumerator>(table->getIntValue("stationary", x));
		temp.slow = static_cast<Locomotions::Enumerator>(table->getIntValue("slow", x));
		temp.fast = static_cast<Locomotions::Enumerator>(table->getIntValue("fast", x));

		temp.move = table->getFloatValue("movementScale", x);
		temp.accel = table->getFloatValue("accelerationScale", x);
		temp.turn = table->getFloatValue("turnScale", x);
		temp.canSeeHeightMod = table->getFloatValue("canSeeHeightMod", x);

		(*m_postures)[posture] = temp;
		if (temp.stationary != Locomotions::Invalid)
			m_locomotionPostures[temp.stationary] = posture;
		if (temp.slow != Locomotions::Invalid)
			m_locomotionPostures[temp.slow] = posture;
		if (temp.fast != Locomotions::Invalid)
			m_locomotionPostures[temp.fast] = posture;
	}
	table = 0;
}

// ----------------------------------------------------------------------

MovementTable::~MovementTable (void)
{
	delete m_postures;
}

// ----------------------------------------------------------------------

MovementTable::eLocomotionSpeed MovementTable::getLocomotionData(Locomotions::Enumerator locomotion, Postures::Enumerator &o_posture) const
{
	if (locomotion < 0 || locomotion >= Locomotions::NumberOfLocomotions)
		return kInvalid;

	o_posture = m_locomotionPostures[locomotion];
	if (o_posture == Postures::Invalid)
		return kInvalid;

	return getPostureSpeed(o_posture, locomotion);
}

// ----------------------------------------------------------------------

MovementTable::eLocomotionSpeed MovementTable::getPostureSpeed(Postures::Enumerator i_posture, Locomotions::Enumerator i_locomotion) const
{
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return kInvalid;

	const rates &rate = m_postures->find(i_posture)->second;
	if (rate.fast == i_locomotion)
		return kFast;

	if (rate.slow == i_locomotion)
		return kSlow;

	DEBUG_FATAL(rate.stationary != i_locomotion, ("Bad movement rate setup. See a programmer"));
	return kStationary;
}

// ----------------------------------------------------------------------

Locomotions::Enumerator MovementTable::getLocomotion(Postures::Enumerator i_posture, uint64 i_states, MovementTable::eLocomotionSpeed i_speed) const
{
	//-- Validate posture arg.
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return Locomotions::Invalid;

	//-- Lookup base rates for posture.
	PostureMap::const_iterator f = m_postures->find(i_posture);
	if (f == m_postures->end())
		return Locomotions::Invalid;

	const rates &rate = f->second;

	//-- Adjust locomotion speed for the prevailing states.
	eLocomotionSpeed speed = adjustSpeedForStates(i_speed, i_states);

	//-- Return the appropriate rate based on locomotion speed.
	Locomotions::Enumerator  locomotion = Locomotions::Invalid;

	switch (i_speed)
	{
		case kInvalid:
			locomotion = Locomotions::Invalid;
			break;

		case kStationary:
			locomotion = rate.stationary;
			break;

		case kSlow:
			if (speed >= kSlow)
				locomotion = rate.slow;
			break;

		case kFast:
			if (speed >= kFast)
				locomotion = rate.fast;
			break;
	}

	return locomotion;
}

// ----------------------------------------------------------------------

void MovementTable::getAllLocomotions(Postures::Enumerator i_posture, uint64 i_states, Locomotions::Enumerator &fastLocomotion, Locomotions::Enumerator &slowLocomotion, Locomotions::Enumerator &stationaryLocomotion) const
{
	//-- Initialize to invalid.
	fastLocomotion       = Locomotions::Invalid;
	slowLocomotion       = Locomotions::Invalid;
	stationaryLocomotion = Locomotions::Invalid;

	//-- Validate posture arg.
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return;

	//-- Lookup base rates for posture.
	PostureMap::const_iterator f = m_postures->find(i_posture);
	if (f == m_postures->end())
		return;

	const rates &rate = f->second;

	//-- Retrieve locomotion for each rate if that rate is supported.
	if (adjustSpeedForStates(kFast, i_states) >= kFast)
		fastLocomotion = rate.fast;

	if (adjustSpeedForStates(kSlow, i_states) >= kSlow)
		slowLocomotion = rate.slow;

	stationaryLocomotion = rate.stationary;
}

// ----------------------------------------------------------------------

bool MovementTable::getRates(Postures::Enumerator i_posture, float &o_moveScale, float &o_accelScale, float &o_turnScale) const
{
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return false;

	PostureMap::const_iterator f = m_postures->find(i_posture);
	if (f == m_postures->end())
		return false;

	const rates &rate = f->second;

	o_moveScale = rate.move;
	o_turnScale = rate.turn;
	o_accelScale = rate.accel;

	return true;
}

// ----------------------------------------------------------------------

bool MovementTable::hasMovingLocomotion(Postures::Enumerator i_posture, uint64 i_states) const
{
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return false;

	if (adjustSpeedForStates(kFast, i_states) == kStationary)
		return false;

	const rates &rate = m_postures->find(i_posture)->second;
	return (rate.fast != Locomotions::Invalid) || (rate.slow != Locomotions::Invalid);
}

// ----------------------------------------------------------------------

float MovementTable::getSlowMoveRate(Postures::Enumerator i_posture, uint64 i_states) const
{
	//-- Validate posture arg.
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return 0.0f;

	//-- Check if state speed adjustment causes us to stop moving.
	if (adjustSpeedForStates(kSlow, i_states) < kSlow)
		return 0.0f;

	//-- Lookup rate table for posture.
	PostureMap::iterator findIt = m_postures->find(i_posture);
	if (findIt == m_postures->end())
	{
		DEBUG_WARNING(true, ("MovementTable::getSlowMoveRate(): failed to find rate table entry for posture id=[%s],name=[%s], will return 0 rate.", i_posture, Postures::getPostureName(i_posture)));
		return 0.0f;
	}

	const rates &rate = findIt->second;

	//-- Start with the fast movement rate.
	float const baseRate = (rate.slow != Locomotions::Invalid) ? rate.move : 0.0f;

	//-- Retrieve state rate modifiers.
	float const stateRateModifier = getMovementRateModifierForStates(i_states);

	float const finalRate = baseRate * stateRateModifier;
	return finalRate;
}

// ----------------------------------------------------------------------

float MovementTable::getFastMoveRate(Postures::Enumerator i_posture, uint64 i_states) const
{
	//-- Validate posture arg.
	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return 0.0f;

	//-- Check if state speed adjustment causes us to stop moving.
	if (adjustSpeedForStates(kFast, i_states) <= kSlow)
		return 0.0f;

	//-- Lookup rate table for posture.
	PostureMap::iterator findIt = m_postures->find(i_posture);
	if (findIt == m_postures->end())
	{
		DEBUG_WARNING(true, ("MovementTable::getFastMoveRate(): failed to find rate table entry for posture id=[%s],name=[%s], will return 0 rate.", i_posture, Postures::getPostureName(i_posture)));
		return 0.0f;
	}

	const rates &rate = findIt->second;

	//-- Start with the fast movement rate.
	float const baseRate = (rate.fast != Locomotions::Invalid) ? rate.move : 0.0f;

	//-- Retrieve state rate modifiers.
	float const stateRateModifier = getMovementRateModifierForStates(i_states);

	float const finalRate = baseRate * stateRateModifier;
	return finalRate;
}

// ----------------------------------------------------------------------

void MovementTable::getAllMovementRateScales(Postures::Enumerator posture, uint64 states, float &slowMoveRateScale, float &fastMoveRateScale, float &accelerationScale, float &turnScale) const
{
	//-- Lookup rate table for posture.
	PostureMap::iterator findIt = m_postures->find(posture);
	if (findIt == m_postures->end())
	{
		slowMoveRateScale = 0.0f;
		fastMoveRateScale = 0.0f;
		accelerationScale = 0.0f;
		turnScale         = 0.0f;

		DEBUG_WARNING(true, ("MovementTable::getSlowAndFastMoveRates(): failed to find rate table entry for posture id=[%i],name=[%s], will return 0 rate.", posture, Postures::getPostureName(posture)));
		return;
	}

	const rates &rate = findIt->second;

	//-- Retrieve accel/turn rate modifiers.
	accelerationScale = rate.accel;
	turnScale         = rate.turn;

	//-- Apply posture's movement rate.
	slowMoveRateScale = (rate.slow != Locomotions::Invalid) ? rate.move : 0.0f;
	fastMoveRateScale = (rate.fast != Locomotions::Invalid) ? rate.move : 0.0f;

	//-- Check if a state's max speed causes us to stop moving.
	eLocomotionSpeed  slowSpeed = kSlow;
	eLocomotionSpeed  fastSpeed = kFast;

	adjustTwoSpeedsForStates(states, slowSpeed, fastSpeed);

	slowMoveRateScale *= (slowSpeed >= kSlow) ? 1.0f : 0.0f;
	fastMoveRateScale *= (fastSpeed >= kFast) ? 1.0f : 0.0f;

	//-- Retrieve and apply state movement rate modifiers.
	float const stateRateModifier = getMovementRateModifierForStates(states);

	slowMoveRateScale *= stateRateModifier;
	fastMoveRateScale *= stateRateModifier;
}

// ----------------------------------------------------------------------

float MovementTable::getCanSeeHeightMod(Postures::Enumerator i_posture, uint64 i_states) const
{
	UNREF(i_states);

	if (i_posture < 0 || i_posture >= Postures::NumberOfPostures)
		return 1.0f;

	const rates &rate = m_postures->find(i_posture)->second;

	return rate.canSeeHeightMod;
}

// ----------------------------------------------------------------------

float MovementTable::getMovementRateModifierForStates(uint64 stateFlags) const
{
	DEBUG_FATAL(!ms_installed, ("MovementTable not installed."));

	//-- Start with base rate modifier of multiplicative identity.
	float  rateModifier = 1.0f;

	if (stateFlags)
	{
		//-- Apply each state, one at a time.
		for (States::Enumerator i = 0; i < States::NumberOfStates; ++i)
		{
			// Check if this state is present.
			if (hasState(stateFlags, i))
			{
				// Check if we have an entry for this state.
				StateFloatMap::iterator const findIt = s_stateMovementRateModifiers.find(i);
				if (findIt != s_stateMovementRateModifiers.end())
				{
					// This state is present and we have a movement rate modifier for it.  Apply the movement rate modifier.
					rateModifier *= findIt->second;
				}
			}
		}
	}

	return rateModifier;
}

// ======================================================================
// class MovementTable: PRIVATE
// ======================================================================

MovementTable::eLocomotionSpeed MovementTable::adjustSpeedForStates(eLocomotionSpeed i_speed, uint64 i_states) const
{
	if (i_states)
		for (States::Enumerator i = 0; i < States::NumberOfStates; ++i)
			if (hasState(i_states, i) && ms_stateMoveLimit[i] < i_speed)
				i_speed = ms_stateMoveLimit[i];
	return i_speed;
}

// ----------------------------------------------------------------------

void MovementTable::adjustTwoSpeedsForStates(uint64 states, eLocomotionSpeed &speed1, eLocomotionSpeed &speed2) const
{
	if (states)
	{
		for (States::Enumerator i = 0; i < States::NumberOfStates; ++i)
		{
			if (hasState(states, i))
			{
				eLocomotionSpeed const maxSpeed = ms_stateMoveLimit[i];
				if (speed1 > maxSpeed)
					speed1 = maxSpeed;

				if (speed2 > maxSpeed)
					speed2 = maxSpeed;
			}
		}
	}
}

// ======================================================================
