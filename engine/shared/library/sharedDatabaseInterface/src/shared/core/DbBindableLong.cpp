// ======================================================================
//
// DBBindableLong.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableLong.h"

#include <string>

// ======================================================================

using namespace DB;

BindableLong::BindableLong() : Bindable(), value(-999)
{
}

BindableLong::BindableLong(int _value) : Bindable(sizeof(value)), value(_value)
{
}

// ----------------------------------------------------------------------

BindableLong::BindableLong(long _value) : Bindable(sizeof(value)), value(_value)
{
}

void *BindableLong::getBuffer()
{
	return &value; //lint !e1536 // exposing private member
}

// ----------------------------------------------------------------------


long BindableLong::getValue() const
{
	return value;
}

// ----------------------------------------------------------------------

BindableLong &BindableLong::operator=(int rhs)
{
	setValue(rhs);
	return *this;
}

// ----------------------------------------------------------------------

BindableLong &BindableLong::operator=(long rhs)
{
	indicator=sizeof(value); 
	value=rhs;
	return *this;
}

// ----------------------------------------------------------------------

void BindableLong::setValue(int rhs)
{
	setValue(static_cast<long>(rhs));
}

// ----------------------------------------------------------------------

void BindableLong::setValue(long rhs)
{
	indicator=sizeof(value); 
	value=rhs;
}

// ----------------------------------------------------------------------

std::string BindableLong::outputValue() const
{
	char temp[255];
	snprintf(temp,sizeof(temp),"%li",value);
	temp[sizeof(temp)-1]='\0';
	return std::string(temp);
}

// ======================================================================
