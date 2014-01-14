//========================================================================
//
// PlayerData.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_PlayerData_H
#define INCLUDED_PlayerData_H

namespace PlayerDataPriviledgedTitle
{
	// DO NOT DELETE OR CHANGE THE ORDER OF THESE ENUMS,
	// BECAUSE THEY ARE PERSISTED IN THE DB; ADD NEW ONES
	// AT THE BOTTOM
	enum
	{
		NormalPlayer = 0,
		CustomerServiceRepresentative = 1,
		Developer = 2,
		Warden = 3,
		QualityAssurance = 4,
	};
}	// namespace PlayerDataPriviledgedTitle

//========================================================================

#endif	// INCLUDED_PlayerData_H
