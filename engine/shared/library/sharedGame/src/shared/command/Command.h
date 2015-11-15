// ======================================================================
//
// Command.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================


#ifndef INCLUDED_Command_H
#define INCLUDED_Command_H

#include "sharedGame/CommandCppFunc.def"
#include "swgSharedUtility/Locomotions.h"
#include "swgSharedUtility/States.h"
#include <bitset>

class NetworkId;
class StringId;

//----------------------------------------------------------------------

class Command
{
public:

	Command();
	Command(Command const &);
	Command &operator=(Command const &);

	bool isNull() const;
	bool isPrimaryCommand() const;   // test cooldown group against "defaultAttack"

	enum Priority
	{
		CP_Immediate            = 0,
		CP_Front                = 1,
		CP_Normal               = 2,
		CP_Default              = 3, // not a real priority, falls through
		CP_NumberOfPriorities   = 4,
	};

	enum Target
	{
		CT_Friend          = 0,
		CT_Enemy           = 1,
		CT_Other           = 2,
		CT_NumberOfTargets = 3
	};

	enum TargetType
	{
		CTT_None                = 0,
		CTT_Required            = 1,
		CTT_Optional            = 2,
		CTT_Location            = 3,
		CTT_All					= 4,
		CTT_NumberOfTargetTypes = 5
	};

	enum ErrorCode
	{
		CEC_Success      = 0,
		CEC_Locomotion   = 1,
		CEC_Ability      = 2,
		CEC_TargetType   = 3,
		CEC_TargetRange  = 4,
		CEC_StateMustNotHave = 5, // command not allowed while in a particular state
		CEC_StateMustHave = 6,    // command not allowed until in a particular state
		CEC_GodLevel     = 7,
		CEC_Cancelled    = 8,
		CEC_Max          = 9
	};

	enum WeaponType
	{
		CWT_Rifle             =  0,
		CWT_Carbine           =  1,
		CWT_Pistol            =  2,
		CWT_Heavy             =  3,
		CWT_1handMelee        =  4,
		CWT_2handMelee        =  5,
		CWT_Unarmed           =  6,
		CWT_Polearm           =  7,
		CWT_Thrown            =  8,
		CWT_1handLightsaber   =  9,
		CWT_2handLightsaber   = 10,
		CWT_PolearmLightsaber = 11,
		CWT_Ranged            = -1,
		CWT_Melee             = -2,
		CWT_All               = -3,
		CWT_AllLightsabers    = -4,
		CWT_ForcePower        = -5,
		CWT_AllButHeavy       = -6
	};
	
	static const StringId & getStringIdForErrorCode (const ErrorCode code);

	typedef CommandCppFunc::Type CppFunc;

public:
	std::string                                    m_commandName;
	uint32                                         m_commandHash;
	Priority                                       m_defaultPriority;
	std::string                                    m_scriptHook;
	std::string                                    m_failScriptHook;
	CppFunc                                        m_cppHook;
	CppFunc                                        m_failCppHook;
	float                                          m_defaultTime;
	std::string                                    m_characterAbility;
	std::bitset<Locomotions::NumberOfLocomotions>  m_locomotionPermissions;
	std::bitset<States::NumberOfStates>            m_statePermissions;
	std::bitset<States::NumberOfStates>            m_stateRequired;
	std::string                                    m_tempScript;
	Target                                         m_target;
	TargetType                                     m_targetType;
	std::string                                    m_stringId;
	int                                            m_visibleToClients;
	bool                                           m_callOnTarget;
	uint32                                         m_commandGroup;
	float                                          m_maxRangeToTarget;
	float                                          m_maxRangeToTargetSquared;
	int                                            m_godLevel;
	uint32                                         m_displayGroup;
	bool										   m_addToCombatQueue;
	float                                          m_warmTime;
	float                                          m_execTime;
	float                                          m_coolTime;
	uint32                                         m_coolGroup;
	uint32                                         m_weaponTypesValid;
	uint32                                         m_weaponTypesInvalid;
	uint32                                         m_coolGroup2;
	float                                          m_coolTime2;
	bool                                           m_toolbarOnly;
	bool                                           m_fromServerOnly;
};

//----------------------------------------------------------------------

#endif // INCLUDED_Command_H

