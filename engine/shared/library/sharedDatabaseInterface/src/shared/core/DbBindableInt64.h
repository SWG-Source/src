// ======================================================================
//
// DBBindableInt64.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableInt64_H
#define INCLUDED_DBBindableInt64_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableString.h"

// ======================================================================

#define BindableInt64BufferSize 21 // big enough to hold -(2^63)

namespace DB {

    /**
	 * Represents an int64 in C++
     */
	class BindableInt64 : public BindableString<BindableInt64BufferSize> 
	{
	  public:
		BindableInt64();
		explicit BindableInt64(int64 value);

		int64 getValue() const;
		void setValue(const int64 rhs);
		BindableInt64 &operator=(const int64 rhs); 

		void getValue(int64 &buffer) const;
		void getValue(uint64 &buffer) const;
	};

}

// ----------------------------------------------------------------------

inline void DB::BindableInt64::getValue(int64 &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableInt64::getValue(uint64 &buffer) const
{
	buffer=static_cast<uint64>(getValue());
}

// ======================================================================

#endif
