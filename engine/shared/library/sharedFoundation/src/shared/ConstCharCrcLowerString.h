// ======================================================================
//
// ConstCharCrcLowerString.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConstCharCrcLowerString_H
#define INCLUDED_ConstCharCrcLowerString_H

// ======================================================================

#include "sharedFoundation/CrcLowerString.h"

// ======================================================================

class ConstCharCrcLowerString : public CrcLowerString
{
public:
	ConstCharCrcLowerString(const char *string);
	virtual ~ConstCharCrcLowerString();
};

// ======================================================================

#endif
