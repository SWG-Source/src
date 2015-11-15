// ======================================================================
//
// Overlap2d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Overlap2d_H
#define INCLUDED_Overlap2d_H

class Line3d;
class Segment3d;
class Circle;
class AxialBox;

namespace Overlap2d
{

// ----------------------------------------------------------------------

bool TestPointCircle  ( Vector const & V, Circle const & C );
bool TestLineCircle   ( Line3d const & S, Circle const & C );
bool TestCircleCircle ( Circle const & A, Circle const & B );
bool TestSegCircle    ( Segment3d const & S, Circle const & C );
bool TestSegABox      ( Segment3d const & S, AxialBox const & B );
bool TestPointTri     ( Vector const & V, Triangle3d const & T );
bool TestSegSeg       ( Vector const & A, Vector const & B1, Vector const & A2, Vector const & B2 );

inline bool Test ( Segment3d const & S, AxialBox const & B )    { return TestSegABox(S,B); }
inline bool Test ( Segment3d const & S, Circle const & C )      { return TestSegCircle(S,C); }

// ----------------------------------------------------------------------

} // namespace Overlap2d

#endif

