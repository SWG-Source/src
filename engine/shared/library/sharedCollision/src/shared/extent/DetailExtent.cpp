// ======================================================================
//
// DetailExtent.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/DetailExtent.h"

#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Range.h"
#include "sharedMath/Ray3d.h"

#include <vector>

const Tag TAG_DTAL = TAG(D,T,A,L);

// ======================================================================

DetailExtent::DetailExtent()
: CompositeExtent(ET_Detail)
{
}

DetailExtent::~DetailExtent()
{
}

// ----------------------------------------------------------------------

void DetailExtent::install ( void )
{
	ExtentList::assignBinding(TAG_DTAL,DetailExtent::build);
}

void DetailExtent::remove ( void )
{
}

Extent *     DetailExtent::build       ( Iff & iff )
{
	DetailExtent * extent = new DetailExtent();

	extent->load(iff);

	return extent;
}

// ----------------------------------------------------------------------

void DetailExtent::load ( Iff & iff )
{
	iff.enterForm(TAG_DTAL);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

 		default:
			FATAL (true, ("DetailExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_DTAL);

}

// ----------------------------------------------------------------------

void DetailExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_DTAL);

		iff.insertForm(TAG_0000);

			CompositeExtent::write(iff);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_DTAL);
}

// ----------

void DetailExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);

		CompositeExtent::load(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool DetailExtent::contains ( Vector const & vector ) const
{
	for(int i = 0; i < getExtentCount(); i++)
	{
		if(!getExtent(i)->contains(vector)) return false;
	}

	return true;
}

bool DetailExtent::contains ( Vector const & begin, Vector const & end ) const
{
	for(int i = 0; i < getExtentCount(); i++)
	{
		if(!getExtent(i)->contains(begin,end)) return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool DetailExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const
{
	float bestTime = REAL_MAX;
	bool hit = false;

	int const numberOfExtents = getExtentCount();
	if (numberOfExtents)
	{
		//-- Check the bounding box for the entire composite extent
		Vector direction(end - begin);
		if (direction.normalize())
		{
			Ray3d const ray(begin, direction);

			Intersect3d::ResultData * const data = 0;
			if (Intersect3d::intersectRayABoxWithData(ray, m_box, data))
			{
				//-- If the global bounding intersection succeeds, we need to test the "child" extents
				for (int i = 1; i < numberOfExtents; ++i)
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
			}
		}
	}

	return hit;
}

// ----------------------------------------------------------------------

Range DetailExtent::rangedIntersect ( Line3d const & line ) const
{
	Range R = Range::empty;

	for(int i = 0; i < getExtentCount(); i++)
	{
		R = getExtent(i)->rangedIntersect(line);

		if(R.isEmpty()) return R;
	}

	return R;
}

Range DetailExtent::rangedIntersect ( Ray3d const & ray ) const
{
	Range range = Range::empty;

	for(int i = 0; i < getExtentCount(); i++)
	{
		range = getExtent(i)->rangedIntersect(ray);

		if(range.isEmpty()) return range;
	}

	return range;
}

Range DetailExtent::rangedIntersect ( Segment3d const & seg ) const
{
	Range range = Range::empty;

	for(int i = 0; i < getExtentCount(); i++)
	{
		range = getExtent(i)->rangedIntersect(seg);

		if(range.isEmpty()) return range;
	}

	return range;
}

// ----------------------------------------------------------------------

BaseExtent * DetailExtent::clone ( void ) const
{
    DetailExtent * newExtent = new DetailExtent();

    cloneChildren( newExtent );

    return newExtent;
}

void DetailExtent::updateBounds ( void )
{
	m_sphere = m_extents->back()->getBoundingSphere();
	m_box = m_extents->back()->getBoundingBox();
}

// ----------------------------------------------------------------------

int countExtents ( BaseExtent const * extent )
{
	if(extent == nullptr) return 0;

	CompositeExtent const * composite = dynamic_cast<CompositeExtent const *>(extent);

	if(composite == nullptr) return 1;

	int accum = 0;

	for(int i = 0; i < composite->getExtentCount(); i++)
	{
		accum += countExtents(composite->getExtent(i));
	}

	return accum;
}

bool DetailExtent::validate ( void ) const
{
	int extentCount = getExtentCount();

	if(extentCount < 2)
	{
		DEBUG_WARNING(true,("DetailExtent::validate - Detail extent has less than 2 sub-extents"));

		return false;
	}

	int lastCount = -1;

	for(int i = 0; i < extentCount; i++)
	{
		int nextCount = countExtents(getExtent(i));

		if(nextCount <= lastCount)
		{
			DEBUG_WARNING(true,("DetailExtent::validate - Detail extent's sub-extents are in the wrong order"));

			return false;
		}

		lastCount = nextCount;
	}

	return true;
}
