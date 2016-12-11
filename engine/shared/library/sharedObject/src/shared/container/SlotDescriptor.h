// ======================================================================
//
// SlotDescriptor.h
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlotDescriptor_H
#define INCLUDED_SlotDescriptor_H

// ======================================================================

class CrcLowerString;
class Iff;
class SlotId;

// ======================================================================
/**
 * Describes a collection of slots.
 *
 * ObjectTemplate instances that have slotted containers will reference
 * a SlotDescriptor instance.  The SlotDescriptor instance will list
 * the slots available in the slotted container.
 *
 * This class is a shared resource since (1) both server and client
 * need access to this data, (2) we don't want to need to send this
 * data around needlessly, and (3) several ObjectTemplate instances
 * may reference a single SlotDescriptor instance.  For example,
 * most (if not all) player species have the same slots available
 * for equipping.
 */

class SlotDescriptor
{
friend class SlotDescriptorList;

public:

	typedef std::vector<SlotId>  SlotIdVector;

public:

	explicit SlotDescriptor(Iff &iff, const CrcLowerString &name);

	const CrcLowerString &getName() const;
	const SlotIdVector   &getSlots() const;

	void                  fetch() const;
	void                  release() const;
	int                   getReferenceCount() const;

private:

	~SlotDescriptor();

	void load_0000(Iff &iff);

	// disabled
	SlotDescriptor(const SlotDescriptor&);
	SlotDescriptor &operator =(const SlotDescriptor&);

private:

	CrcLowerString *m_name;
	SlotIdVector   *m_slots;

	mutable int     m_referenceCount;

};

// ======================================================================

#endif
