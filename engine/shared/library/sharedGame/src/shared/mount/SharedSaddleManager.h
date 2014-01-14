// ======================================================================
//
// SaddleManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SharedSaddleManager_H
#define INCLUDED_SharedSaddleManager_H

// ======================================================================

class CrcString;
class CreatureObject;
class HardpointObject;
class Object;

// ======================================================================

class SharedSaddleManager
{
public:

	static void install();

	static CrcString const * getLogicalSaddleNameForMountAppearance(CrcString const & mountObjectTemplateName);
	static CrcString const * getSaddleAppearanceForLogicalSaddleName(CrcString const & logicalSaddleName);
	static CrcString const * getClientDataFilenameForLogicalSaddleName(CrcString const & logicalSaddleName);
	static CrcString const * getRiderPoseForSaddleAppearanceNameAndRiderSeatIndex(CrcString const & saddleAppearanceName, int riderSeatIndex);

	static int getSaddleSeatingCapacityForLogicalSaddleName(CrcString const & logicalSaddleName);

private:
	SharedSaddleManager();
	SharedSaddleManager(SharedSaddleManager const & copy);
	SharedSaddleManager & operator=(SharedSaddleManager const & copy);

};

// ======================================================================

#endif
