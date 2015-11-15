// ======================================================================
//
// ArrangementDescriptor.h
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ArrangementDescriptor_H
#define INCLUDED_ArrangementDescriptor_H

// ======================================================================

class CrcLowerString;
class Iff;
class SlotId;

// ======================================================================

class ArrangementDescriptor
{
friend class ArrangementDescriptorList;

public:

	typedef stdvector<SlotId>::fwd        SlotIdVector;

public:

	ArrangementDescriptor(Iff &iff, const CrcLowerString &name);

	const CrcLowerString &getName() const;
	int                   getReferenceCount() const;

	void                  fetch() const;
	void                  release() const;

	int                   getArrangementCount() const;
	const SlotIdVector   &getArrangement(int index) const;

private:

	typedef stdvector<SlotIdVector>::fwd  ArrangementVector;

private:

	~ArrangementDescriptor();

	void  load_0000(Iff &iff);

	// disabled
	ArrangementDescriptor(const ArrangementDescriptor&);
	ArrangementDescriptor &operator =(const ArrangementDescriptor&);

private:

	CrcLowerString    *m_name;
	mutable int        m_referenceCount;

	ArrangementVector *m_arrangements;
	
};

// ======================================================================

#endif
