//===================================================================
//
// HeightData.h
// asommers 2001-08-02
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_HeightData_H
#define INCLUDED_HeightData_H

//===================================================================

class Iff;
class Vector;
class Vector2d;

//===================================================================

class HeightData
{
public:

	HeightData ();
	~HeightData ();

	void          load (Iff& iff);
	void          save (Iff& iff) const;

	//-- creation
	void          clear ();
	void          addSegment ();
	void          addPoint (const Vector& point);

	//-- query
	int           getNumberOfSegments () const;
	int           getNumberOfPoints (int segmentIndex) const;
	const Vector& getPoint (int segmentIndex, int pointIndex) const;
	bool          find (int segmentIndex, const Vector2d& position, float& result) const;

	//-- call one of these once all the points have been added
	void          createRoadData ();
	void          createRiverData ();

private:

	void          recalculateExtents ();

private:

	void          load_0000 (Iff& iff);
	void          load_0001 (Iff& iff);

private:

	HeightData (const HeightData&);
	HeightData& operator= (const HeightData&);

private:

	class Segment;
	typedef std::vector<Segment*> SegmentList;
	SegmentList*  m_segmentList;

	bool          m_needsRecalculateExtents;
};

//===================================================================

#endif
