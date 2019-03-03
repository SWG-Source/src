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

		template <typename T>
		inline void getValue(T &buffer) const {
			buffer = static_cast<T>(getValue());
		}
		
		void *getBuffer();

		virtual std::string outputValue() const;
		
	  private:
		long int value;
	}; //lint !e1721 !e1509 // no virtual destructor, unusual operator =

}

#endif
