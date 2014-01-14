// ======================================================================
//
// ComponentExtent.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/ComponentExtent.h"

#include "sharedCollision/ExtentList.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Range.h"

const Tag TAG_CMPT = TAG(C,M,P,T);

// ----------------------------------------------------------------------

ComponentExtent::ComponentExtent()
: CompositeExtent(ET_Component)
{
}

ComponentExtent::~ComponentExtent()
{
}

// ----------------------------------------------------------------------

void ComponentExtent::install ( void )
{
	ExtentList::assignBinding(TAG_CMPT,ComponentExtent::build);
}

void ComponentExtent::remove ( void )
{
}

Extent *     ComponentExtent::build       ( Iff & iff )
{
	ComponentExtent * extent = new ComponentExtent();

	extent->load(iff);

	return extent;
}

// ----------------------------------------------------------------------
// Virtual BaseExtent interface

void ComponentExtent::load ( Iff & iff ) 
{
	iff.enterForm(TAG_CMPT);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

 		default:
			FATAL (true, ("ComponentExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_CMPT);
	
}

// ----------

void ComponentExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_CMPT);

		iff.insertForm(TAG_0000);

			CompositeExtent::write(iff);
		
		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_CMPT);
}

// ----------------------------------------------------------------------

bool ComponentExtent::contains ( Vector const & vector ) const
{
	for(int i = 0; i < getExtentCount(); i++)
	{
		if(getExtent(i)->contains(vector)) return true;
	}

	return false;
}

// this doesn't handle cumulative containment
bool ComponentExtent::contains ( Vector const & begin, Vector const & end ) const
{
	for(int i = 0; i < getExtentCount(); i++)
	{
		if(getExtent(i)->contains(begin,end)) return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool ComponentExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const
{
	float bestTime = REAL_MAX;
	bool hit = false;

	int const numberOfExtents = getExtentCount();
	for (int i = 0; i < numberOfExtents; ++i)
	{
		Vector normal;
		float t;
		Vector * const normalPtr = (surfaceNormal != 0) ? &normal : 0;
		BaseExtent const * const extent = getExtent(i);
		if (extent->intersect(begin, end, normalPtr, &t))
		{
			if (t < bestTime)
			{
				if (surfaceNormal) 
					*surfaceNormal = normal;

				if (time) 
					*time = t;

				bestTime = t;
			}

			hit = true;
		}
	}

	return hit;
}

// ----------------------------------------------------------------------

Range ComponentExtent::rangedIntersect( Line3d const & line ) const
{
	Range range = Range::empty;

	for(int i = 0; i < getExtentCount(); i++)
	{
		Range temp = getExtent(i)->rangedIntersect(line);

		range = Range::enclose(range,temp);
	}

	return range;
}

Range ComponentExtent::rangedIntersect( Ray3d const & ray ) const
{
	Range range = Range::empty;

	for(int i = 0; i < getExtentCount(); i++)
	{
		Range temp = getExtent(i)->rangedIntersect(ray);

		range = Range::enclose(range,temp);
	}

	return range;
}

Range ComponentExtent::rangedIntersect( Segment3d const & seg ) const
{
	Range range = Range::empty;

	for(int i = 0; i < getExtentCount(); i++)
	{
		Range temp = getExtent(i)->rangedIntersect(seg);

		range = Range::enclose(range,temp);
	}

	return range;
}

// ----------------------------------------------------------------------

BaseExtent * ComponentExtent::clone ( void ) const
{
    ComponentExtent * newExtent = new ComponentExtent();

    cloneChildren( newExtent );

    return newExtent;
}

// ----------------------------------------------------------------------












// ======================================================================

void ComponentExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);
		
		CompositeExtent::load(iff);
		
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------
