// ======================================================================
//
// SetupSharedMath.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/SetupSharedMath.h"

#include "sharedMath/ConfigSharedMath.h"
#include "sharedMath/CompressedQuaternion.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedMath/Transform.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

void SetupSharedMath::install()
{
	InstallTimer const installTimer("SetupSharedMath::install");

	ConfigSharedMath::install();

	//-- install palette support
	PaletteArgb::install();
	PaletteArgbList::install();

	CompressedQuaternion::install();
	Transform::install();
}

// ======================================================================
