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

BindableLong::BindableLong(int64_t _value) : Bindable(sizeof(value)), value(_value)
{
}

void *BindableLong::getBuffer()
{
	return &value; //lint !e1536 // exposing private member
}

// ----------------------------------------------------------------------


int64_t BindableLong::getValue() const
{
	return value;
}

// ----------------------------------------------------------------------

BindableLong &BindableLong::operator=(int64_t rhs)
{
	indicator=sizeof(value); 
	value=rhs;
	return *this;
}

// ----------------------------------------------------------------------

void BindableLong::setValue(int64_t rhs)
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
