// ======================================================================
//
// LessPointerComparator.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef LESS_POINTER_COMPARATOR_H
#define LESS_POINTER_COMPARATOR_H

// ======================================================================

class LessPointerComparator
{
public:
	template <typename PointerType>
	bool operator ()(PointerType lhs, PointerType rhs) const
		{
			return *lhs < *rhs;
		}
};

//----------------------------------------------------------------------

template <typename T> class LessPointerComparatorStatic
{
public:
	static bool compare(T * a, T * b)
	{
		return *a < *b;
	}
};

// ======================================================================

#endif
