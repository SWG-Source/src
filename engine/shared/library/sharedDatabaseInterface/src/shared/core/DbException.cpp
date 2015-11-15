// ======================================================================
//
// DbException.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbException.h"

// ======================================================================

using namespace DB;

// ======================================================================

Exception::Exception(int errorNumber, const std::string &errorDescription, const std::string &contextDescription) :
		m_errorNumber(errorNumber),
		m_errorDescription(errorDescription),
		m_contextDescription(contextDescription)
{
}

// ----------------------------------------------------------------------

Exception::Exception(int errorNumber, const std::string &errorDescription) :
		m_errorNumber(errorNumber),
		m_errorDescription(errorDescription),
		m_contextDescription()
{
}

// ======================================================================
