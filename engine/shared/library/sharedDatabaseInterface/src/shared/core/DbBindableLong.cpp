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

BindableLong::BindableLong(long int _value) : Bindable(sizeof(value)), value(_value)
{
}

void *BindableLong::getBuffer()
{
	return &value; //lint !e1536 // exposing private member
}

// ----------------------------------------------------------------------

long int BindableLong::getValue() const
{
	return value;
}

// ----------------------------------------------------------------------

BindableLong &BindableLong::operator=(long int rhs)
{
	indicator=sizeof(value); 
	value=rhs;
	return *this;
}

// ----------------------------------------------------------------------

void BindableLong::setValue(long int rhs)
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
