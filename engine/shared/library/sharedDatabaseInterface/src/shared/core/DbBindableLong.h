// ======================================================================
//
// DBBindableLong.h
// copyright (c) 2001 Sony Online Entertainment
//
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
		explicit BindableLong(int32_t _value);

		int32_t getValue() const;
		void setValue(const int32_t rhs);
		BindableLong &operator=(const int32_t rhs);

		// following alternate getValue's are provided for convenience, particularly in
		// the auto-generated code:
		void getValue(int8 &buffer) const;
		void getValue(uint8 &buffer) const;
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

inline void DB::BindableLong::getValue(int8 &buffer) const
{
	buffer=static_cast<int8>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(uint8 &buffer) const
{
	buffer=static_cast<uint8>(getValue());
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

inline void DB::BindableLong::getValue(int32_t &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(uint32 &buffer) const
{
	buffer=static_cast<uint32>(getValue());
}



// ======================================================================

#endif
