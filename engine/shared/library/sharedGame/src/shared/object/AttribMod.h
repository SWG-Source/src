//========================================================================
//
// AttribMod.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef _INCLUDED_AttribMod_H
#define _INCLUDED_AttribMod_H


namespace AttribMod
{

struct AttribMod
{
	uint32                 tag;         // crc value to identify the mod
	union
	{
		int                attrib;      // which attribute the mod affects
		uint32             skill;       // which skill the mod affects
	};
	int                    value;       // how much the mod changes the attribute
	float                  attack;      // time to go from 0 to value
	float                  sustain;     // time to stay at value
	float                  decay;       // time to go from value to 0
	int                    flags;       // flag to define how the mod operates
};

// If decay is negative, it means on of these special conditions
enum AttribModDecaySpecial
{
	AMDS_pool     = -1,     // use the attribute pool recovery rate (normal recovery)
	AMDS_wound    = -2,     // can only be healed by skill/item use, heal difficulty adjusted by shock wound value
	AMDS_antidote = -3,     // flag to remove attrib mods that affect the target attribute
	AttribModDecaySpecial_Last = AMDS_antidote
};

// AttribMod flag values
enum AttribModFlags
{
	AMF_changeMax     = 0x00000001,      // the mod affects the max attrib value
	AMF_attackCurrent = 0x00000002,      // the mod attack/sustain affects the current attrib value
	AMF_decayCurrent  = 0x00000004,      // the mod decay affects the current attrib value
	AMF_triggerOnDone = 0x00000008,      // cause a trigger to go off when the mod ends
	AMF_visible       = 0x00000010,      // the attrib mod info should be sent to the player
	AMF_skillMod      = 0x00000020,      // this mod affects skill mods, not attribs
	AMF_directDamage  = 0x00000040,      // the mod is direct damage that should be applied to the creature
};

//-----------------------------------------------------------------------

inline bool isAttribMod(const AttribMod & mod)
{
	return (mod.flags & (AMF_skillMod | AMF_directDamage)) == 0;
}

inline bool isSkillMod(const AttribMod & mod)
{
	return (mod.flags & AMF_skillMod) != 0;
}

inline bool isDirectDamage(const AttribMod & mod)
{
	return (mod.flags & AMF_directDamage) != 0;
}

};

//-----------------------------------------------------------------------

bool operator!=(const AttribMod::AttribMod & a, const AttribMod::AttribMod & b);
inline bool operator==(const AttribMod::AttribMod & a, const AttribMod::AttribMod & b)
{
	return !(a != b);
}


#endif	// _INCLUDED_AttribMod_H

