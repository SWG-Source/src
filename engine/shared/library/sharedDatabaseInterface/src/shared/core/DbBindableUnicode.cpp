// ======================================================================
//
// DBBindableUnicode.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableUnicode.h"

// ======================================================================

std::string DB::BindableUnicodeBase::outputValue() const
{
	const std::string quote("\"");
	return quote + std::string(m_buffer) + quote;
}

// ======================================================================
