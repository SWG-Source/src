// ======================================================================
//
// DBBindableBool.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableBool.h"

// ======================================================================

using namespace DB;

BindableBool::BindableBool() : Bindable()
{
	value[1]='\0';
}

BindableBool::BindableBool(bool _value) :
    Bindable(1)
{
	value[0]=_value?'Y':'N';
	value[1]='\0';
}


std::string BindableBool::getValueASCII() const
{
	if (isNull())
	{
		return std::string(); // TODO should this be defaulted to N
	}
	else
	{
		return std::string(1,value[0]);
	}
}

bool BindableBool::getValue() const
{
	return (value[0]=='Y');
}

void BindableBool::setValue(bool _value)
{
	indicator=1;
	value[0]=_value?'Y':'N';
}

BindableBool &BindableBool::operator=(bool _value)
{
	setValue(_value);
	return *this;
}

void *BindableBool::getBuffer()
{
	return value; //lint !e1536 // exposing private member (because DB interface will set it directly)
}

std::string BindableBool::outputValue() const
{
	if (getValue())
		return std::string("true");
	else
		return std::string("false");
}

// ======================================================================
