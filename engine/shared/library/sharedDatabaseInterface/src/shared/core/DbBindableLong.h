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
		explicit BindableLong(long int _value);

		long int getValue() const;
		void setValue(const long int rhs);
		BindableLong &operator=(const long int rhs); 

		// following alternate getValue's are provided for convenience, particularly in
		// the auto-generated code:
		void getValue(unsigned int &buffer) const;
		void getValue(uint32 &buffer) const; // for some reason, our compiler is convinced that uint32 != unsigned int
		void getValue(long int &buffer) const;
		void getValue(int &buffer) const;
		void getValue(int8 &buffer) const;
		void getValue(uint8 &buffer) const;
		
		void *getBuffer();

		virtual std::string outputValue() const;
		
	  private:
		long int value;
	}; //lint !e1721 !e1509 // no virtual destructor, unusual operator =

}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(unsigned int &buffer) const
{
	buffer=static_cast<unsigned int>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(uint32 &buffer) const
{
	buffer=static_cast<uint32>(getValue());
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(long int &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableLong::getValue(int &buffer) const
{
	buffer=getValue();
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

// ======================================================================

#endif
