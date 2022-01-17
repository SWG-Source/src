// ======================================================================
//
// DBBindableInt32.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableInt32_H
#define INCLUDED_DBBindableInt32_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

// ======================================================================

namespace DB {

    /**
	 * Represents a long in C++, bound to an INTEGER or NUMBER database column
     */
	class BindableInt32 : public Bindable
	{
		public:
		BindableInt32();
		explicit BindableInt32(int32_t _value);

		int32_t getValue() const;
		void setValue(const int32_t rhs);
		BindableInt32 &operator=(const int32_t rhs);

		// following alternate getValue's are provided for convenience, particularly in
		// the auto-generated code:
		void getValue(int8_t &buffer) const;
		void getValue(uint8_t &buffer) const;
		void getValue(int16_t &buffer) const;
		void getValue(uint16_t &buffer) const;
		void getValue(int32_t &buffer) const;
		void getValue(uint32_t &buffer) const;
		
		void *getBuffer();

		virtual std::string outputValue() const;
		
	  private:
		int32_t value;
	}; //lint !e1721 !e1509 // no virtual destructor, unusual operator =

}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(int8_t &buffer) const
{
	buffer=static_cast<int8_t>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(uint8_t &buffer) const
{
	buffer=static_cast<uint8_t>(getValue());
}
// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(int16_t &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(uint16_t &buffer) const
{
	buffer=static_cast<uint16_t>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(int32_t &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(uint32_t &buffer) const
{
	buffer=static_cast<uint32_t>(getValue());
}

// ======================================================================

#endif
