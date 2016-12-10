// ======================================================================
//
// MovementTable.h
// Copyright 2002-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MovementTable_H
#define INCLUDED_MovementTable_H

//===================================================================

#include "swgSharedUtility/Locomotions.def"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

//===================================================================

class MovementTable
{
public:

	enum eLocomotionSpeed
	{
		kInvalid = -1,
		kStationary,
		kSlow,
		kFast
	};

public:

	static void install(const std::string &stateFileName);

public:

	MovementTable (const std::string &fileName, const std::string &templateName); // templateName is used for error messages
	virtual ~MovementTable ();

	eLocomotionSpeed         getLocomotionData(Locomotions::Enumerator locomotion, Postures::Enumerator &o_posture) const;
	eLocomotionSpeed         getPostureSpeed(Postures::Enumerator o_posture, Locomotions::Enumerator locomotion) const;

	Locomotions::Enumerator  getLocomotion(Postures::Enumerator i_posture, uint64 i_states, eLocomotionSpeed speed) const;
	void                     getAllLocomotions(Postures::Enumerator i_posture, uint64 i_states, Locomotions::Enumerator &fastLocomotion, Locomotions::Enumerator &slowLocomotion, Locomotions::Enumerator &stationaryLocomotion) const;

	bool                     getRates(Postures::Enumerator i_posture, float &moveScale, float &accelScale, float &turnScale) const;
	bool                     hasMovingLocomotion(Postures::Enumerator i_posture, uint64 i_states) const;
	
	float                    getSlowMoveRate(Postures::Enumerator i_posture, uint64 i_states) const;
	float                    getFastMoveRate(Postures::Enumerator i_posture, uint64 i_states) const;
	void                     getAllMovementRateScales(Postures::Enumerator posture, uint64 states, float &slowMoveRateScale, float &fastMoveRateScale, float &accelerationScale, float &turnScale) const;

	float                    getCanSeeHeightMod(Postures::Enumerator i_posture, uint64 i_states) const;

	float                    getMovementRateModifierForStates(uint64 stateFlags) const;

private:

	struct rates;
	typedef std::map<Postures::Enumerator, rates> PostureMap;

private:

	eLocomotionSpeed         adjustSpeedForStates(eLocomotionSpeed i_speed, uint64 i_states) const;
	void                     adjustTwoSpeedsForStates(uint64 states, eLocomotionSpeed &speed1, eLocomotionSpeed &speed2) const;

	// Disabled.
	MovementTable();
	MovementTable(const MovementTable&);
	MovementTable& operator=(const MovementTable&);

private:

	static bool              ms_installed;
	static eLocomotionSpeed  ms_stateMoveLimit[States::NumberOfStates];

private:

	PostureMap* const        m_postures;
	Postures::Enumerator     m_locomotionPostures[Locomotions::NumberOfLocomotions];

};

//===================================================================

#endif
