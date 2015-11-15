// ======================================================================
//
// CollisionEnums.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionEnums.h"

namespace Containment
{

// Used to turn containment results for two orthogonal axes into a single containment result

ContainmentResult    ComposeAxisTests ( ContainmentResult A, ContainmentResult B )
{
	if(A == B)
	{
		return A;
	}
	else if((A == CR_Outside) || (B == CR_Outside))
	{
		return CR_Outside;
	}
	else if ((A == CR_TouchingOutside) || (B == CR_TouchingOutside))
	{
		return CR_TouchingOutside;
	}
	else if ((A == CR_Overlap) || (B == CR_Overlap))
	{
		if((A == CR_Boundary) || (B == CR_Boundary))
		{
			// One exception to this pattern - if the test volume overlaps the boundary
			// along one axis and is in the boundary along the other axis, then the test
			// volume touches the outside of the boundary

			return CR_TouchingOutside;
		}
		else
		{
			return CR_Boundary;
		}
	}
	else if((A == CR_Boundary) || (B == CR_Boundary))
	{
		return CR_Boundary;
	}
	else if((A == CR_TouchingInside) || (B == CR_TouchingInside))
	{
		return CR_TouchingInside;
	}
	else
	{
		return CR_Inside;
	}
}

ContainmentResult    ComposeAxisTests ( ContainmentResult A, ContainmentResult B, ContainmentResult C )
{
	return Containment::ComposeAxisTests(ComposeAxisTests(A,B),C);
}

// ----------------------------------------------------------------------

} // namespace Containment
