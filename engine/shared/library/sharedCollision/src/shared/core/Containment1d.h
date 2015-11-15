// ======================================================================
//
// Containment1d.h
// copyright (c) 2001 Sony Online Entertainment
//
// Containment tests, bounding volume enclosure functions
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Containment1d_H
#define INCLUDED_Containment1d_H

#include "sharedCollision/CollisionEnums.h"

class Range;

namespace Containment1d
{

// ----------------------------------------------------------------------

ContainmentResult  TestFloatLess    ( float f, float v );
ContainmentResult  TestFloatGreater ( float f, float v );
ContainmentResult  TestFloatRange   ( float f, Range const & R );

ContainmentResult  TestRangeLess    ( Range const & R, float v );
ContainmentResult  TestRangeGreater ( Range const & R, float v );
ContainmentResult  TestRangeRange   ( Range const & A, Range const & B );

// ----------

Range EncloseRanges   ( Range const & A, Range const & B );
Range EncloseRanges   ( Range const & A, Range const & B, Range const & C );
Range EncloseRanges   ( Range const & A, Range const & B, Range const & C, Range const & D );

// ----------------------------------------------------------------------

}; // namespace Containment1d

#endif

