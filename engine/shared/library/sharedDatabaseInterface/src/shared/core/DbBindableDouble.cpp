// ======================================================================
//
// DBBindableDouble.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableDouble.h"

#include "sharedDatabaseInterface/DbServer.h"

#include <string>
#include <climits>

// ======================================================================

using namespace DB;

BindableDouble::BindableDouble() : Bindable(), value(-999.0)
{
}

BindableDouble::BindableDouble(const double &_value) :
    Bindable(sizeof(value)),
	value(_value)
{
	if (finite(value)==0)
	{
		if (DB::Server::getFatalOnDataError())
		{
			FATAL(true,("DatabaseError:  Attempt to save a non-finite value to the database."));
		}
		else
		{
			value=0;
			WARNING_STACK_DEPTH(true,(INT_MAX,"DatabaseError:  Attempt to save a non-finite value to the database."));
		}
	}
}

double BindableDouble::getValue() const
{
	return value;
}

void BindableDouble::setValue(const double &_value)
{
	indicator=sizeof(value);
	value=_value;
	if (finite(value)==0)
	{
		if (DB::Server::getFatalOnDataError())
		{
			FATAL(true,("DatabaseError:  Attempt to save a non-finite value to the database."));
		}
		else
		{
			value=0;
			WARNING_STACK_DEPTH(true,(INT_MAX,"DatabaseError:  Attempt to save a non-finite value to the database."));
		}
	}
}

BindableDouble &BindableDouble::operator=(const double &_value)
{
	setValue(_value);
	return *this;
}

void *BindableDouble::getBuffer()
{
	return &value; //lint !e1536 // exposing private member (because DB interface will set it directly)
}

std::string BindableDouble::outputValue() const
{
	char temp[255];
	snprintf(temp,sizeof(temp),"%f",value);
	temp[sizeof(temp)-1]='\0';
	return std::string(temp);
}

// ======================================================================
