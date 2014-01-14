//===================================================================
//
// StructureFootprint.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_StructureFootprint_H
#define INCLUDED_StructureFootprint_H

//===================================================================

#include "sharedObject/LotType.h"
#include "sharedMath/Rectangle2d.h"

class Iff;

//===================================================================

class StructureFootprint
{
public:

	StructureFootprint ();
	~StructureFootprint ();

	void     load (const char* filename);

	int      getWidth () const;
	int      getHeight () const;

	Rectangle2d const & getBoxTestRect () const;

	int      getPivotX () const;
	int      getPivotZ () const;
	LotType  getLotType (int x, int z) const;
	float    getHardReservationTolerance () const;
	float    getStructureReservationTolerance () const;
	int      getNumberOfReservations () const;
	int      getNumberOfHardReservations () const;
	int      getNumberOfStructureReservations () const;
	void     getBorder (int x, int z, bool& top, bool& bottom, bool& left, bool& right) const;

private:

	void     load (Iff& iff);
	void     load_0000 (Iff& iff);

private:

	StructureFootprint (const StructureFootprint&);
	StructureFootprint& operator= (const StructureFootprint&);

private:

	int      m_width;
	int      m_height;
	Rectangle2d m_boxTestRect;
	int      m_pivotX;
	int      m_pivotZ;
	LotType* m_data;
	float    m_hardReservationTolerance;
	float    m_structureReservationTolerance;
	int      m_numberOfReservations;
	int      m_numberOfHardReservations;
	int      m_numberOfStructureReservations;
};

//===================================================================

#endif
