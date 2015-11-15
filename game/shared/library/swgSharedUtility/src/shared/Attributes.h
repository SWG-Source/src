// ======================================================================
//
// Attributes.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Attributes_H
#define INCLUDED_Attributes_H

// ======================================================================

class StringId;

// ======================================================================
/**
 * Support utilities for working with Attributes.
 */

namespace Attributes
{
	const std::string & getAttributeName         (int attribute);
	const StringId &    getAttributeStringId     (int attribute);
	const StringId &    getAttributeDescStringId (int attribute);
}

// ======================================================================

#endif
