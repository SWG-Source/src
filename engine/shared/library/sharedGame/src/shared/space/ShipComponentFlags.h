//======================================================================
//
// ShipComponentFlags.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentFlags_H
#define INCLUDED_ShipComponentFlags_H

//======================================================================

namespace ShipComponentFlags
{
	enum Flags
	{
		F_disabled   = 0x0001,
		F_lowPower   = 0x0002, //this flag is transient and is overriden at runtime
		F_active     = 0x0004,
		F_demolished = 0x0008,
		F_reverse_engineered = 0x0010,
		F_shieldsFront = 0x0020, //this flag is transient and is overriden at runtime
		F_shieldsBack = 0x0040,  //this flag is transient and is overriden at runtime
		F_disabledNeedsPower = 0x0080
	};
}

//======================================================================

#endif
