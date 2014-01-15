// ======================================================================
//
// DBBindableBase.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableBase.h"

// ======================================================================

using namespace DB;

// ----------------------------------------------------------------------

Bindable::Bindable() : indicator(-1)
{
}

// ----------------------------------------------------------------------

Bindable::~Bindable()
{
}

// ----------------------------------------------------------------------

Bindable::Bindable(int _indicator) :
		indicator(_indicator)
{
}

// ----------------------------------------------------------------------

bool Bindable::isNull() const
{
	return (indicator==-1);
}

// ----------------------------------------------------------------------

void Bindable::setNull()
{
	indicator=-1;
}

// ----------------------------------------------------------------------

int *Bindable::getIndicator()
{
	return &indicator; //lint !e1536 // Exposing private member.  This is done because various functions in the DB interfaces want to set the indicator directly.
}

// ======================================================================
