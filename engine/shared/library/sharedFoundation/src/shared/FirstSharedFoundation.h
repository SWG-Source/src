// ======================================================================
//
// FirstSharedFoundation.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstSharedFoundation_H
#define INCLUDED_FirstSharedFoundation_H

// ======================================================================
// Set the different build debug levels

#define DEBUG_LEVEL_RELEASE    0
#define DEBUG_LEVEL_OPTIMIZED  1
#define DEBUG_LEVEL_DEBUG      2

#define _CRT_SECURE_NO_DEPRECATE 1

#ifndef DEBUG_LEVEL
	#ifdef _DEBUG
		#define DEBUG_LEVEL DEBUG_LEVEL_DEBUG
	#else
		#define DEBUG_LEVEL DEBUG_LEVEL_RELEASE
	#endif	
#endif

#if (DEBUG_LEVEL != DEBUG_LEVEL_RELEASE) && (DEBUG_LEVEL != DEBUG_LEVEL_OPTIMIZED) && (DEBUG_LEVEL != DEBUG_LEVEL_DEBUG)
#error DEBUG_LEVEL is not valid
#endif

// ======================================================================
// things we really have to have

#include "sharedFoundationTypes/FoundationTypes.h"
#include "sharedFoundation/FirstPlatform.h"


// ======================================================================

#include <cfloat>

// ======================================================================
// deprecated stuff that should eventually go away

typedef float real;

#define CONST_REAL(a)  static_cast<float>(a)
#define RECIP(a)       (1.0f / (a))

const float REAL_MIN = FLT_MIN;
const float REAL_MAX = FLT_MAX;

// ======================================================================

#ifndef NULL
#define NULL 0
#endif

// @todo codereorg
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// ======================================================================
// things that are useful just about everywhere

#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "sharedFoundation/MacroFoundation.h"

#include "sharedFoundation/Binary.h"
#include "sharedFoundation/Fatal.h"
#include "sharedFoundation/StlForwardDeclaration.h"
#include "sharedFoundation/Misc.h"
#include "sharedDebug/RemoteDebug.h"
#include "sharedDebug/Report.h"
#include "sharedFoundation/FloatMath.h"
#include "sharedFoundation/SafeCast.h"

// ======================================================================

#endif
