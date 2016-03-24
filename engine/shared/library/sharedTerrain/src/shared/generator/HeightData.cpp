//===================================================================
//
// HeightData.cpp
// asommers 2001-08-02
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/HeightData.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"

#include <algorithm>
#include <vector>

//===================================================================

static const Tag TAG_ROAD = TAG (R,O,A,D);
static const Tag TAG_SGMT = TAG (S,G,M,T);
static const Tag TAG_HDTA = TAG (H,D,T,A);

//===================================================================

class HeightData::Segment
{
public:

	Segment ();
	~Segment ();
	
	int           getNumberOfPoints () const;
	const Vector& getPoint (int index) const;
	void          setPoint (int index, const Vector& point);
	bool          find (const Vector2d& point, float& result) const;

	void          addPoint (const Vector& point);
	void          clear ();

	void          load (Iff& iff);
	void          save (Iff& iff);

	void          createRoadData ();

private:

	Segment (const Segment&);             //lint -esym (754, Segment::Segment)
	Segment& operator= (const Segment&);  //lint -esym (754, Segment::operator=)

private:

	typedef std::vector<Vector> PointList;
	PointList m_pointList;
};

//-------------------------------------------------------------------

HeightData::Segment::Segment () :
	m_pointList ()
{
}

//-------------------------------------------------------------------

HeightData::Segment::~Segment ()
{
}
	
//-------------------------------------------------------------------

int HeightData::Segment::getNumberOfPoints () const
{
	return static_cast<int> (m_pointList.size ());
}

//-------------------------------------------------------------------

const Vector& HeightData::Segment::getPoint (int index) const
{
	DEBUG_FATAL (index < 0 || index >= static_cast<int> (m_pointList.size ()), ("index out of range %i <= %i < %i", 0, index, m_pointList.size ()));
	return m_pointList [static_cast<uint> (index)];
}

//-------------------------------------------------------------------

void HeightData::Segment::setPoint (int index, const Vector& point)
{
	DEBUG_FATAL (index < 0 || index >= static_cast<int> (m_pointList.size ()), ("index out of range %i <= %i < %i", 0, index, m_pointList.size ()));
	m_pointList [static_cast<uint> (index)] = point;
}

//-------------------------------------------------------------------

bool HeightData::Segment::find (const Vector2d& oposition, float& result) const
{
	Vector2d position = oposition;

	{
		const float x0 = std::min (m_pointList.begin ()->x, m_pointList.back ().x);
		const float z0 = std::min (m_pointList.begin ()->z, m_pointList.back ().z);
		const float x1 = std::max (m_pointList.begin ()->x, m_pointList.back ().x);
		const float z1 = std::max (m_pointList.begin ()->z, m_pointList.back ().z);
		position.x = clamp (x0, position.x, x1);
		position.y = clamp (z0, position.y, z1);
	}

	const float width  = m_pointList.back ().x - m_pointList.begin ()->x;
	const float height = m_pointList.back ().z - m_pointList.begin ()->z;

	if (fabsf (width) >= fabsf (height))
	{
		if (width >= 0)
		{
			DEBUG_FATAL (width == 0, ("this can't happen"));

			//-- go from -x to +x
			PointList::const_iterator current  = m_pointList.begin ();
			PointList::const_iterator previous = current;
			
			for (; current != m_pointList.end () && current->x < position.x; previous = current, ++current)
				;

			DEBUG_FATAL (current == m_pointList.begin (), ("couldn't straddle begin"));
			DEBUG_FATAL (current == m_pointList.end (), ("couldn't straddle end"));

			const float t = (position.x - previous->x) / (current->x - previous->x);
			result = linearInterpolate (previous->y, current->y, t);
		}
		else
		{
			//-- go from +x to -x
			PointList::const_reverse_iterator current  = m_pointList.rbegin ();
			PointList::const_reverse_iterator previous = current;
			
			for (; current != m_pointList.rend () && current->x < position.x; previous = current, ++current)
				;

			DEBUG_FATAL (current == m_pointList.rbegin (), ("couldn't straddle rbegin"));
			DEBUG_FATAL (current == m_pointList.rend (), ("couldn't straddle rend"));

			const float t = (position.x - previous->x) / (current->x - previous->x);
			result = linearInterpolate (previous->y, current->y, t);
		}
	}
	else
	{
		if (height >= 0)
		{
			DEBUG_FATAL (height == 0, ("this can't happen"));

			//-- go from -z to +z
			PointList::const_iterator current  = m_pointList.begin ();
			PointList::const_iterator previous = current;
			
			for (; current != m_pointList.end () && current->z < position.y; previous = current, ++current)
				;

			DEBUG_FATAL (current == m_pointList.begin (), ("couldn't straddle begin"));
			DEBUG_FATAL (current == m_pointList.end (), ("couldn't straddle end"));

			const float t = (position.y - previous->z) / (current->z - previous->z);
			result = linearInterpolate (previous->y, current->y, t);
		}
		else
		{
			//-- go from +z to -z
			PointList::const_reverse_iterator current  = m_pointList.rbegin ();
			PointList::const_reverse_iterator previous = current;
			
			for (; current != m_pointList.rend () && current->z < position.y; previous = current, ++current)
				;

			DEBUG_FATAL (current == m_pointList.rbegin (), ("couldn't straddle rbegin"));
			DEBUG_FATAL (current == m_pointList.rend (), ("couldn't straddle rend"));

			const float t = (position.y - previous->z) / (current->z - previous->z);
			result = linearInterpolate (previous->y, current->y, t);
		}
	}

	return true;
}

//-------------------------------------------------------------------

void HeightData::Segment::addPoint (const Vector& point)
{
	m_pointList.push_back (point);
}

//-------------------------------------------------------------------

void HeightData::Segment::clear ()
{
	m_pointList.clear ();
}

//-------------------------------------------------------------------

void HeightData::Segment::load (Iff& iff)
{
	iff.enterChunk (TAG_SGMT);

		const int n = iff.getChunkLengthLeft (static_cast<int> (sizeof (Vector)));
		int i;
		for (i = 0; i < n; ++i)
			m_pointList.push_back (iff.read_floatVector ());

	iff.exitChunk (TAG_SGMT);
}

//-------------------------------------------------------------------

void HeightData::Segment::save (Iff& iff)
{
	iff.insertChunk (TAG_SGMT);

		uint i;
		for (i = 0; i < m_pointList.size (); ++i)
			iff.insertChunkFloatVector (m_pointList [i]);

	iff.exitChunk (TAG_SGMT);
}

//-------------------------------------------------------------------

void HeightData::Segment::createRoadData ()
{
	//-- road data is an averaged original data
	if (m_pointList.size () > 3)
	{
		std::vector<Vector> newPointList;
		newPointList.push_back (m_pointList [0]);

		unsigned int i;
		for (i = 1; i < m_pointList.size () - 1; ++i)
		{
			const float y = (m_pointList [i - 1].y + m_pointList [i].y + m_pointList [i + 1].y) / 3.f;

			newPointList.push_back (Vector (m_pointList [i].x, y, m_pointList [i].z));
		}	

		newPointList.push_back (m_pointList [i]);

		m_pointList.swap (newPointList);
	}
}

//===================================================================

HeightData::HeightData () :
	m_segmentList (NON_NULL (new SegmentList)),
	m_needsRecalculateExtents (true)
{
}

//-------------------------------------------------------------------

HeightData::~HeightData ()
{
	clear ();
	delete m_segmentList;
	m_segmentList = 0;
}

//-------------------------------------------------------------------

void HeightData::load (Iff& iff)
{
	if (! (iff.enterForm (TAG_ROAD, true) || iff.enterForm (TAG_HDTA, true)))
		return;

	clear ();

	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	default:
		FATAL (true, ("invalid form"));
	}

	iff.exitForm ();

	recalculateExtents ();
}

//-------------------------------------------------------------------

void HeightData::load_0000 (Iff& iff)
{
	addSegment ();

	iff.enterChunk (TAG_0000);

		const int n = iff.getChunkLengthLeft (static_cast<int> (sizeof (Vector)));
		int i;
		for (i = 0; i < n; ++i)
			addPoint (iff.read_floatVector ());

	iff.exitChunk (TAG_0000);
}

//-------------------------------------------------------------------

void HeightData::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		int numberOfSegments = iff.getNumberOfBlocksLeft ();
		while (numberOfSegments-- > 0)
		{
			addSegment ();

			m_segmentList->back ()->load (iff);
		}

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void HeightData::save (Iff& iff) const
{
	iff.insertForm (TAG_HDTA);

		iff.insertForm (TAG_0001);

			uint i;
			for (i = 0; i < m_segmentList->size (); ++i)
				 (*m_segmentList) [i]->save (iff);

		iff.exitForm (TAG_0001);

	iff.exitForm (TAG_HDTA);
}

//-------------------------------------------------------------------

void HeightData::clear ()
{
	std::for_each (m_segmentList->begin (), m_segmentList->end (), PointerDeleter ());
	m_segmentList->clear ();
}

//-------------------------------------------------------------------

void HeightData::addSegment ()
{
	m_segmentList->push_back (new Segment ());
}

//-------------------------------------------------------------------

void HeightData::addPoint (const Vector& point)
{
	m_segmentList->back ()->addPoint (point);

	m_needsRecalculateExtents = true;
}

//-------------------------------------------------------------------

int HeightData::getNumberOfSegments () const
{
	return static_cast<int> (m_segmentList->size ());
}

//-------------------------------------------------------------------

int HeightData::getNumberOfPoints (int segmentIndex) const
{
	if (m_segmentList->size ())
	{
		DEBUG_FATAL (segmentIndex < 0 || segmentIndex >= static_cast<int> (m_segmentList->size ()), ("index out of range %i <= %i < %i", 0, segmentIndex, m_segmentList->size ()));
		return (*m_segmentList) [static_cast<uint> (segmentIndex)]->getNumberOfPoints ();
	}

	return 0;
}

//-------------------------------------------------------------------

const Vector& HeightData::getPoint (int segmentIndex, int pointIndex) const
{
	if (m_segmentList->size ())
	{
		DEBUG_FATAL (segmentIndex < 0 || segmentIndex >= static_cast<int> (m_segmentList->size ()), ("index out of range %i <= %i < %i", 0, segmentIndex, m_segmentList->size ()));
		return (*m_segmentList) [static_cast<uint> (segmentIndex)]->getPoint (pointIndex);
	}

	return Vector::zero;
}

//-------------------------------------------------------------------

bool HeightData::find (int segmentIndex, const Vector2d& position, float& result) const
{
	if (m_segmentList->size ())
	{
		DEBUG_FATAL (segmentIndex < 0 || segmentIndex >= static_cast<int> (m_segmentList->size ()), ("index out of range %i <= %i < %i", 0, segmentIndex, m_segmentList->size ()));
		return (*m_segmentList) [static_cast<uint> (segmentIndex)]->find (position, result);
	}

	return false;
}

//-------------------------------------------------------------------

void HeightData::recalculateExtents ()
{
	m_needsRecalculateExtents = false;
}

//-------------------------------------------------------------------

void HeightData::createRoadData ()
{
	uint i;
	for (i = 1; i < m_segmentList->size (); ++i)
		 (*m_segmentList) [i]->setPoint (0, (*m_segmentList) [i - 1]->getPoint ((*m_segmentList) [i - 1]->getNumberOfPoints () - 1));

	std::for_each (m_segmentList->begin (), m_segmentList->end (), VoidMemberFunction (&Segment::createRoadData));
}

//-------------------------------------------------------------------

void HeightData::createRiverData ()
{
	//-- river data is a downward-terraced original data
	const float epsilon = sin (convertDegreesToRadians (5.f)) * 4.f;

	float minimum = FLT_MAX - epsilon;

	uint i;
	for (i = 0; i < m_segmentList->size (); ++i)
	{
		Segment* const segment = (*m_segmentList) [i];

		int j;
		for (j = 0; j < segment->getNumberOfPoints (); ++j)
		{
			Vector point = segment->getPoint (j);

			if (j == 0)
			{
				if (i == 0)
					minimum = point.y;
				else
				{
					point.y = minimum;
					segment->setPoint (j, point);
				}
			}
			else
				if (point.y >= minimum + epsilon)
				{
					minimum += epsilon;
					point.y = minimum;
					segment->setPoint (j, point);
				}
				else
					if (point.y < minimum)
						minimum = point.y;
					else
					{
						point.y = minimum;
						segment->setPoint (j, point);
					}
		}
	}
}

//===================================================================
