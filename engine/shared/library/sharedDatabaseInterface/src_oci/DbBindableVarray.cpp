// ======================================================================
//
// DbBindableVarray.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "DbBindableVarray.h"

#include "OciServer.h"
#include "OciSession.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedDatabaseInterface/DbCheckedConversion.h"
#include "sharedLog/Log.h"
#include <oci.h>
#include <iostream>

// ======================================================================

using namespace DB;

// ======================================================================

BindableVarray::BindableVarray() :
		m_initialized(false),
		m_tdo (nullptr),
		m_data (nullptr),
		m_session (nullptr)
{
}

// ----------------------------------------------------------------------

BindableVarray::~BindableVarray()
{
	if (m_initialized)
		free();
}

// ----------------------------------------------------------------------

bool BindableVarray::create(DB::Session *session, const std::string &name, const std::string &schema)
{
	m_initialized = true;
	NOT_NULL(session);
	m_session = session;
	OCISession *localSession = safe_cast<OCISession*>(session);

	ub4 schemaLength = 0;
	ub4 nameLength = 0;
	if (!DB::checkedNarrow(schema.length(), schemaLength, "OCITypeByName schema length", name.c_str())
		|| !DB::checkedNarrow(name.length(), nameLength, "OCITypeByName type name length", name.c_str()))
		return false;
		
	if (! (localSession->m_server->checkerr(*localSession, OCITypeByName (localSession->envhp,
					 localSession->errhp,
					 localSession->svchp,
					 reinterpret_cast<OraText*>(const_cast<char*>(schema.c_str())),
					 schemaLength,
					 reinterpret_cast<OraText*>(const_cast<char*>(name.c_str())),
					 nameLength,
					 nullptr,
					 0,
					 OCI_DURATION_SESSION,
					 OCI_TYPEGET_HEADER,
					 &m_tdo)))) {
        LOG("DatabaseError", ("Could not create BindableVarray (OCITypeByName) for name - %s", name.c_str()));
		return false;
    }

	if (! (localSession->m_server->checkerr(*localSession,
						OCIObjectNew (localSession->envhp,
						localSession->errhp,
						localSession->svchp,
						OCI_TYPECODE_VARRAY,
						m_tdo, nullptr,
						OCI_DURATION_DEFAULT,
						true,
						reinterpret_cast<void**>(&m_data))))) {
        LOG("DatabaseError", ("Could not create BindableVarray (OCIObjectNew) for name - %s", name.c_str()));
        return false;
    }

	NOT_NULL(m_tdo);
	NOT_NULL(m_data);

	return true;
}

// ----------------------------------------------------------------------

void BindableVarray::free()
{
	OCISession *localSession = safe_cast<OCISession*>(m_session);

	if(!(localSession->m_server->checkerr(*localSession,
						OCIObjectFree (localSession->envhp,
						localSession->errhp, m_data, 0)))) {
        LOG("DatabaseError", ("Could not free up DB object"));
	};
	m_initialized = false;
	m_tdo=nullptr;
	m_data=nullptr;
	m_session=nullptr;
}

// ----------------------------------------------------------------------

void BindableVarray::clear()
{
	OCISession *localSession = safe_cast<OCISession*>(m_session);
	sb4 size=0;

	if (! (localSession->m_server->checkerr(*localSession, OCICollSize(localSession->envhp, localSession->errhp, m_data, &size)))) {
        LOG("DatabaseError", ("Could not clear bindablevarray - OCICollSize"));
        return;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollTrim(localSession->envhp, localSession->errhp, size, m_data)))) {
        LOG("DatabaseError", ("Could not clear bindablevarray - OCICollTrim"));
        return;
    }
}

// ----------------------------------------------------------------------

OCIArray** BindableVarray::getBuffer()
{
	return &m_data;
}

// ----------------------------------------------------------------------

OCIType* BindableVarray::getTDO()
{
	return m_tdo;
}

// ----------------------------------------------------------------------

// Every integer overload binds its argument at the argument's own width, so
// no value is narrowed or widened on the way to the database.
template <typename T>
bool BindableVarrayNumber::appendInteger(bool IsNULL, T value)
{
	OCINumber buffer;

	OCIInd buffer_indicator = IsNULL ? OCI_IND_NULL : OCI_IND_NOTNULL;

	OCISession *localSession = safe_cast<OCISession*>(m_session);

	// A NULL element has no value to convert; the indicator marks it NULL.
	if (IsNULL)
		OCINumberSetZero(localSession->errhp, &buffer);
	else if (! (localSession->m_server->checkerr(*localSession, OCINumberFromInt(localSession->errhp, &value, sizeof(value), OCI_NUMBER_SIGNED, &buffer)))) {
		LOG("DatabaseError", ("Could not push back BindableVarrayNumber %d-byte value OCINumberFromInt - %lld", static_cast<int>(sizeof(value)), static_cast<long long>(value)));
		return false;
	}
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, &buffer, &buffer_indicator, m_data)))) {
		LOG("DatabaseError", ("Could not push back BindableVarrayNumber %d-byte value OCICollAppend - %lld", static_cast<int>(sizeof(value)), static_cast<long long>(value)));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool BindableVarrayNumber::push_back(int32_t value) { return appendInteger(false, value); }
bool BindableVarrayNumber::push_back(int64_t value) { return appendInteger(false, value); }

bool BindableVarrayNumber::push_back(bool IsNULL, int32_t value) { return appendInteger(IsNULL, value); }
bool BindableVarrayNumber::push_back(bool IsNULL, int64_t value) { return appendInteger(IsNULL, value); }

// ----------------------------------------------------------------------

bool BindableVarrayNumber::push_back(double value)
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
		
	OCINumber buffer;

	OCIInd buffer_indicator (OCI_IND_NOTNULL);

	OCISession *localSession = safe_cast<OCISession*>(m_session);

	if (! (localSession->m_server->checkerr(*localSession, OCINumberFromReal(localSession->errhp, &value, sizeof(value), &buffer)))) {
        LOG("DatabaseError", ("Could not push back BindableVArray double value - %g", value));
        return false;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, &buffer, &buffer_indicator, m_data)))) {
        LOG("DatabaseError", ("Could not push back BindableVArray double value - %g", value));
        return false;
    }

	return true;
}

// ----------------------------------------------------------------------

bool BindableVarrayNumber::push_back(bool IsNULL, double value)
{
	if (!IsNULL && finite(value)==0)
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
	
	OCINumber buffer;

	OCIInd buffer_indicator;

	if ( IsNULL )
	{
		buffer_indicator = OCI_IND_NULL;
	}
	else
	{
		buffer_indicator = OCI_IND_NOTNULL;
	}

	OCISession *localSession = safe_cast<OCISession*>(m_session);

	// A NULL element has no value to convert; the indicator marks it NULL.
	if (IsNULL)
		OCINumberSetZero(localSession->errhp, &buffer);
	else if (! (localSession->m_server->checkerr(*localSession, OCINumberFromReal(localSession->errhp, &value, sizeof(value), &buffer)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayNumber double value - %g", value));
        return false;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, &buffer, &buffer_indicator, m_data)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayNumber double value - %g", value));
        return false;
    }

	return true;
}

// ======================================================================

BindableVarrayString::BindableVarrayString() :
		BindableVarray(),
		m_maxLength(0)
{
}

// ----------------------------------------------------------------------

std::string BindableVarrayNumber::outputValue() const
{
	OCISession *localSession = safe_cast<OCISession*>(m_session);
	sb4 size;
	std::string result("[");

	if (! (localSession->m_server->checkerr(*localSession, OCICollSize(localSession->envhp, localSession->errhp, m_data, &size)))) {
        LOG("DatabaseError", ("Could not get outputValue - OCICollSize - %d", size));
		return "[*ERROR*]";
    }

	for (sb4 i=0; i < size; ++i)
	{
		OCINumber *element;
		OCIInd *indicator;
		boolean exists;
		
		if (i!=0)
			result += ", ";
		
		if (localSession->m_server->checkerr(*localSession, OCICollGetElem(localSession->envhp, localSession->errhp, m_data, i, &exists, reinterpret_cast<dvoid**>(&element), reinterpret_cast<dvoid**>(&indicator))))
		{
			if (exists)
			{
				if (*indicator == OCI_IND_NULL)
					result += "nullptr";
				else
				{
					double value;
					if (localSession->m_server->checkerr(*localSession, OCINumberToReal(localSession->errhp, element, sizeof(value), &value)))
					{
						char buffer[100];
						snprintf(buffer,sizeof(buffer),"%f",value);
						buffer[sizeof(buffer)-1]='\0';
						result += buffer;
					}
					else {
                        LOG("DatabaseError", ("Could not get output value from OCINumberToReal - %f", value));
						result += "*ERROR*";
                    }
				}
			}
			else
				result += "*NO ELEMENT*";			
		}
		else {
            LOG("DatabaseError", ("Could not get output value from OCICollGetElem"));
            result += "*ERROR*";
        }
	}
	result+=']';
	return result;
}

// ----------------------------------------------------------------------

bool BindableVarrayString::create(DB::Session *session, const std::string &name, const std::string &schema, size_t maxLength)
{
	m_maxLength=maxLength;
	return BindableVarray::create(session,name,schema);
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(const Unicode::String &value)
{
	std::string str;
	str = Unicode::wideToUTF8(value, str);
	return push_back(str);
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(const std::string &value)
{
	OCIString *buffer = nullptr;
	OCIInd buffer_indicator (OCI_IND_NOTNULL);
	OCISession *localSession = safe_cast<OCISession*>(m_session);

	size_t effectiveLength=value.length();
	if (effectiveLength > m_maxLength)
	{
		if (DB::Server::getFatalOnDataError())
		{
			FATAL(true,("DatabaseError:  Attempt to save too long a string to the database:  \"%s\"",value.c_str()));
		}
		else
		{
			WARNING_STACK_DEPTH(true,(INT_MAX, "DatabaseError:  Attempt to save too long a string to the database.  (Text is in the log output.)"));
			LogManager::logLongText("DatabaseError",std::string("String from previous error is \"")+value+"\"");
			effectiveLength = m_maxLength;
		}
	}
	
	// effectiveLength is at most m_maxLength, the column's size; OCI takes it as a ub4.
	ub4 textLength = 0;
	if (!DB::checkedNarrow(effectiveLength, textLength, "OCIStringAssignText length", value.c_str()))
		return false;

	if (! (localSession->m_server->checkerr(*localSession, OCIStringAssignText(localSession->envhp, localSession->errhp, reinterpret_cast<OraText*>(const_cast<char*>(value.c_str())), textLength, &buffer)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayString string value (OCIStringAssignText) - %s", value.c_str()));
        return false;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, buffer, &buffer_indicator, m_data)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayString string value (OCICollAppend) - %s", value.c_str()));
        return false;
    }

	return true;
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool bvalue)
{
	std::string value;
	if (bvalue)
		value = "Y";
	else
		value = "N";
	
	OCIString *buffer = nullptr;
	OCIInd buffer_indicator (OCI_IND_NOTNULL);
	OCISession *localSession = safe_cast<OCISession*>(m_session);

	if (! (localSession->m_server->checkerr(*localSession, OCIStringAssignText(localSession->envhp, localSession->errhp, reinterpret_cast<OraText*>(const_cast<char*>(value.c_str())), 1, &buffer)))) { // "Y" or "N"
        LOG("DatabaseError", ("Could not push back BindableVarrayString string value (OCIStringAssignText) - %s", value.c_str()));
        return false;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, buffer, &buffer_indicator, m_data)))) {
		LOG("DatabaseError", ("Could not push back BindableVarrayString string value (OCICollAppend) - %s", value.c_str()));
        return false;
    }

	return true;
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(const NetworkId &value)
{
	return push_back(value.getValueString());
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool IsNULL, const Unicode::String &value)
{
	std::string str;
	str = Unicode::wideToUTF8(value, str);
	return push_back(IsNULL, str);
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool IsNULL, const std::string &value)
{
	OCIString *buffer = nullptr;
	OCIInd buffer_indicator;
	if ( IsNULL )
	{
		buffer_indicator = OCI_IND_NULL;
	}
	else
	{
		buffer_indicator = OCI_IND_NOTNULL;
	}

	size_t effectiveLength=value.length();
	if (effectiveLength > m_maxLength)
	{
		if (DB::Server::getFatalOnDataError())
		{
			FATAL(true,("DatabaseError:  Attempt to save too long a string to the database:  \"%s\"",value.c_str()));
		}
		else
		{
			WARNING_STACK_DEPTH(true,(INT_MAX, "DatabaseError:  Attempt to save too long a string to the database.  (Text is in the log output.)"));
			LogManager::logLongText("DatabaseError",std::string("String from previous error is \"")+value+"\"");
			effectiveLength = m_maxLength;
		}
	}

	OCISession *localSession = safe_cast<OCISession*>(m_session);

	// effectiveLength is at most m_maxLength, the column's size; OCI takes it as a ub4.
	ub4 textLength = 0;
	if (!DB::checkedNarrow(effectiveLength, textLength, "OCIStringAssignText length", value.c_str()))
		return false;

	if (! (localSession->m_server->checkerr(*localSession, OCIStringAssignText(localSession->envhp, localSession->errhp, reinterpret_cast<OraText*>(const_cast<char*>(value.c_str())), textLength, &buffer)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayString string value (OCIStringAssignText) - %s", value.c_str()));
        return false;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, buffer, &buffer_indicator, m_data)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayString string value (OCICollAppend) - %s", value.c_str()));
        return false;
    }

	return true;
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool IsNULL, bool bvalue)
{
	std::string value;
	if (bvalue)
		value = "Y";
	else
		value = "N";
	
	OCIString *buffer = nullptr;
	OCIInd buffer_indicator;
	if ( IsNULL )
	{
		buffer_indicator = OCI_IND_NULL;
	}
	else
	{
		buffer_indicator = OCI_IND_NOTNULL;
	}

	OCISession *localSession = safe_cast<OCISession*>(m_session);

	if (! (localSession->m_server->checkerr(*localSession, OCIStringAssignText(localSession->envhp, localSession->errhp, reinterpret_cast<OraText*>(const_cast<char*>(value.c_str())), 1, &buffer)))) { // "Y" or "N"
        LOG("DatabaseError", ("Could not push back BindableVarrayString boolean value (OCIStringAssignText) - %s", value.c_str()));
        return false;
    }
	if (! (localSession->m_server->checkerr(*localSession, OCICollAppend(localSession->envhp, localSession->errhp, buffer, &buffer_indicator, m_data)))) {
        LOG("DatabaseError", ("Could not push back BindableVarrayString boolean value (OCICollAppend) - %s", value.c_str()));
        return false;
    }
	return true;
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool IsNULL, const NetworkId &value)
{
	return push_back(IsNULL, value.getValueString());
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool IsNULL, BindableNetworkId const & column)
{
	if (IsNULL)
		return push_back(true, std::string());
	return push_back(false, column.getValue().getValueString());
}

// ----------------------------------------------------------------------

bool BindableVarrayString::push_back(bool IsNULL, BindableBool const & column)
{
	if (IsNULL)
		return push_back(true, std::string());
	return push_back(false, column.getValue());
}

// ----------------------------------------------------------------------

std::string BindableVarrayString::outputValue() const
{
	OCISession *localSession = safe_cast<OCISession*>(m_session);
	sb4 size;
	std::string result("[");

	if (! (localSession->m_server->checkerr(*localSession, OCICollSize(localSession->envhp, localSession->errhp, m_data, &size)))) {
        LOG("DatabaseError", ("Could not get outputValue BindableVarrayString (OCICollSize)"));
		return "[*ERROR*]";
    }

	for (sb4 i=0; i < size; ++i)
	{
		OCIString **element;
		OCIInd *indicator;
		boolean exists;
		
		if (i!=0)
			result += ", ";
		
		if (localSession->m_server->checkerr(*localSession, OCICollGetElem(localSession->envhp, localSession->errhp, m_data, i, &exists, reinterpret_cast<dvoid**>(&element), reinterpret_cast<dvoid**>(&indicator))))
		{
			if (exists)
			{
				if (*indicator == OCI_IND_NULL)
					result += "nullptr";
				else
				{
					OraText * text = OCIStringPtr(localSession->envhp, *element);
					if (text)
						result += '"' + std::string(reinterpret_cast<char*>(text)) + '"';
					else
						result += "nullptr";
				}
			}
			else
				result += "*NO ELEMENT*";			
		}
		else {
            LOG("DatabaseError", ("Could not get outputValue BindableVarrayString OCICollGetElem"));
            return "[*ERROR*]";
        }
	}
	result+=']';
	return result;
}

// ======================================================================
