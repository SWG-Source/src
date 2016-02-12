// ======================================================================
//
// DBBindableBool.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableBool_H
#define INCLUDED_DBBindableBool_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"
#include <string>

// ======================================================================

namespace DB {

/** Represents a bool in C++, bound to a CHAR(1) column.  The column can have the values 'Y' or 'N'.
 * TODO: use a boolean type in the database, if we can find one that allows nullptr's and is fairly
 * portable. (May not exist -- according to the docs, Oracle does not have a boolean datatype.)
 */
	class BindableBool : public Bindable
	{
		char value[2];
	  public:
		explicit BindableBool(bool _value);
		BindableBool();

		std::string getValueASCII() const;
		bool getValue() const;
		void getValue(bool &buffer) const;
		void setValue(bool _value);
		BindableBool &operator=(bool _value);

		void *getBuffer();

		virtual std::string outputValue() const;		
	}; //lint !e1721 !e1509 // no virtual destructor, unusual operator =

}

// ----------------------------------------------------------------------

/**
 * There are places where it's more convenient to have a getValue() that
 * takes a reference, for consistancy with more complicated types.
 */
inline void DB::BindableBool::getValue(bool &buffer) const
{
	buffer=getValue();
}


// ======================================================================

#endif
