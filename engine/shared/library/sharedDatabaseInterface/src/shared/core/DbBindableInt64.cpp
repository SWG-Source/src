// ======================================================================
//
// DBBindableInt64.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableInt64.h"

#include <cstdio>

// ======================================================================

using namespace DB;

BindableInt64::BindableInt64()
{
}

// ----------------------------------------------------------------------

BindableInt64::BindableInt64(int64 value)
{
	setValue(value);
}

// ----------------------------------------------------------------------

int64 BindableInt64::getValue() const
{
	int64 temp;
	char buffer[BindableInt64BufferSize+1];
	BindableString<BindableInt64BufferSize>::getValue(buffer,BindableInt64BufferSize+1);
	sscanf(buffer,INT64_FORMAT_SPECIFIER,&temp);
	return temp;
}

// ----------------------------------------------------------------------

BindableInt64 &BindableInt64::operator=(int64 rhs)
{
	setValue(rhs);
	return *this;
}

// ----------------------------------------------------------------------

void BindableInt64::setValue(int64 rhs)
{
	char buffer[BindableInt64BufferSize+1];
	sprintf(buffer,INT64_FORMAT_SPECIFIER,rhs);
	BindableString<BindableInt64BufferSize>::setValue(buffer);
}

// ======================================================================
