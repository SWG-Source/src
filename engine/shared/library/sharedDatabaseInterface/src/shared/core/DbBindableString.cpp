// ======================================================================
//
// DBBindableString.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbBindableString.h"

// ======================================================================

std::string DB::BindableStringBase::outputValue() const
{
	const std::string quote("\"");
	return quote + std::string(m_buffer) + quote;
}

// ======================================================================
