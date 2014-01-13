// ======================================================================
//
// ConstCharCrcLowerString.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"

// ======================================================================

ConstCharCrcLowerString::ConstCharCrcLowerString(const char *string)
: CrcLowerString(string, PersistentCrcString::CC_true)
{
}

// ----------------------------------------------------------------------

ConstCharCrcLowerString::~ConstCharCrcLowerString()
{
}

// ======================================================================
