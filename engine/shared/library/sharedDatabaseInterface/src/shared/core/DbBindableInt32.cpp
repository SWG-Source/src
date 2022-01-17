// ======================================================================
//
// DBBindableInt32.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableInt32.h"

#include <string>

// ======================================================================

using namespace DB;

BindableInt32::BindableInt32() : Bindable(), value(-999)
{
}

BindableInt32::BindableInt32(int32_t _value) : Bindable(sizeof(value)), value(_value)
{
}

void *BindableInt32::getBuffer()
{
	return &value; //lint !e1536 // exposing private member
}

// ----------------------------------------------------------------------


int32_t BindableInt32::getValue() const
{
	return value;
}

// ----------------------------------------------------------------------

BindableInt32 &BindableInt32::operator=(int32_t rhs)
{
	indicator=sizeof(value); 
	value=rhs;
	return *this;
}

// ----------------------------------------------------------------------

void BindableInt32::setValue(int32_t rhs)
{
	indicator=sizeof(value); 
	value=rhs;
}

// ----------------------------------------------------------------------

std::string BindableInt32::outputValue() const
{
	char temp[255];
	snprintf(temp,sizeof(temp),"%li",value);
	temp[sizeof(temp)-1]='\0';
	return std::string(temp);
}

// ======================================================================
