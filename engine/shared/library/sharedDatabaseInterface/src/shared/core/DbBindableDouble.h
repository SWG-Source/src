// ======================================================================
//
// DBBindableDouble.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableDouble_H
#define INCLUDED_DBBindableDouble_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

// ======================================================================

namespace DB {

    /**
	 * Represents a double in C++, bound to a FLOAT(nn) or a NUMBER database column
	 * TODO:  make variations of this for different precisions (when it is determined what precisions we need)
	 */
	class BindableDouble : public Bindable
	{
		double value;
	  public:
		explicit BindableDouble(const double &_value);
		BindableDouble();

		double getValue() const;
		void setValue(const double &_value);
		BindableDouble &operator=(const double &_value);

		void getValue(double &buffer) const;
		void getValue(real &buffer) const;
		
		void *getBuffer();

		virtual std::string outputValue() const;
		
	}; //lint !e1721 !e1509 // no virtual destructor, unusual operator =

}

// ----------------------------------------------------------------------

inline void DB::BindableDouble::getValue(double &buffer) const
{
	buffer=getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableDouble::getValue(real &buffer) const
{
	buffer=static_cast<real>(getValue());
}

// ======================================================================

#endif
