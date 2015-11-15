// ======================================================================
//
// SlotId.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_SlotId_H
#define	INCLUDED_SlotId_H

class SlotId
{
public:

	static const SlotId invalid;

public:

	SlotId();
	explicit SlotId(int slot);
	~SlotId();

	int getSlotId() const;
	SlotId& operator=(const SlotId&);
	bool operator==(const SlotId&) const;
	bool operator!=(const SlotId&) const;
	bool operator<(const SlotId&) const;

	bool render() const;

private:

	int     m_slot;

};

// ------------------------------------------------------------------------

inline int SlotId::getSlotId() const
{
	return m_slot;
}

// ------------------------------------------------------------------------

inline bool SlotId::operator==(const SlotId& rhs) const
{
	return (m_slot == rhs.getSlotId());
}

// ------------------------------------------------------------------------

inline bool SlotId::operator!=(const SlotId& rhs) const
{
	return !(rhs == *this);
}

// ------------------------------------------------------------------------

inline bool SlotId::operator<(const SlotId& rhs) const
{
	return (m_slot < rhs.getSlotId());
}

// ------------------------------------------------------------------------

/** Query the slot to find out if we should render its contents.
*/
inline bool SlotId::render() const
{
	return true;
}

// -------------------------------------------------------------------------
#endif

