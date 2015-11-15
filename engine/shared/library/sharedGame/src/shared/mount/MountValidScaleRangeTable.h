// ======================================================================
//
// MountValidScaleRangeTable.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MountValidScaleRangeTable_H
#define INCLUDED_MountValidScaleRangeTable_H

// ======================================================================

class CrcString;

// ======================================================================

class MountValidScaleRangeTable
{
public:

	enum MountabilityStatus
	{
		MS_creatureMountable = 0,
		MS_speciesUnmountable,
		MS_speciesMountableSeatingCapacityUnsupported,
		MS_speciesMountableScaleOutOfRange,
		MS_speciesMountableMissingRiderSlot
	};

public:

	static void install(char const *const filename);

	static MountabilityStatus doesCreatureSupportScaleAndSaddleCapacity(CrcString const &creatureAppearanceName, float creatureScale, int saddleCapacity);

	static int                getCreatureCount();
	static CrcString const   &getCreatureName(int index);

	static bool               getScaleRangeForCreatureAndSaddleCapacity(CrcString const &creatureAppearanceName, int saddleCapacity, float &minimumScale, float &maximumScale);

private:

	// Disabled.
	MountValidScaleRangeTable();
	MountValidScaleRangeTable(MountValidScaleRangeTable const&);
	MountValidScaleRangeTable &operator =(MountValidScaleRangeTable const&);

};

// ======================================================================

#endif
