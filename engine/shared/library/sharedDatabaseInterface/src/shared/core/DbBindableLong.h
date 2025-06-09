// ======================================================================
//
// DBBindableLong.h
// copyright (c) 2001 Sony Online Entertainment
//
// NOTE: This class needs to be flexible with the compiler such that
// when compiling using 32 bit OCI libraries this class is 4 bytes
// in size.  When compiling using 64 bit OCI libraries, this class
// must be 8 bytes in size simply because the OCI libraries read
// the data type from the database of a NUMBER field and convert to
// either 4 bytes or 8 bytes depending on architecture used.
// ======================================================================

#ifndef INCLUDED_DBBindableLong_H
#define INCLUDED_DBBindableLong_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

// ======================================================================

namespace DB {

    /**
	 * Represents a long in C++, bound to an INTEGER or NUMBER database column
     */
	class BindableLong : public Bindable
	{
		public:
		BindableLong();
		explicit BindableLong(long _value);

		long getValue() const;
		void setValue(const long rhs);
		BindableLong &operator=(const long rhs);

		// following alternate getValue's are provided for convenience, particularly in
		// the auto-generated code:
		void getValue(int8_t &buffer) const;
		void getValue(uint8_t &buffer) const;
		void getValue(int16_t &buffer) const;
		void getValue(uint16_t &buffer) const;
		void getValue(int &buffer) const;
		void getValue(unsigned int &buffer) const;
		void getValue(long &buffer) const;
		void getValue(unsigned long &buffer) const;
		
		void *getBuffer();

		virtual std::string outputValue() const;
		
	  private:
		long value;
	}; //lint !e1721 !e1509 // no virtual destructor, unusual operator =

}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(int8_t &buffer) const
{
	buffer=static_cast<int8_t>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(uint8_t &buffer) const
{
	buffer=static_cast<uint8_t>(getValue());
}
// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(int16_t &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(uint16_t &buffer) const
{
	buffer=static_cast<uint16_t>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(int &buffer) const
{
	buffer=static_cast<int>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(unsigned int &buffer) const
{
	buffer=static_cast<unsigned int>(getValue());
}
// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(long &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(unsigned long &buffer) const
{
	buffer=static_cast<unsigned long>(getValue());
}

// ======================================================================

#endif
