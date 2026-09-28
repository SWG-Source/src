// ======================================================================
//
// DbCheckedConversion.cpp
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbCheckedConversion.h"

#include "sharedLog/Log.h"

// ======================================================================

void DB::CheckedConversionNamespace::reportFailure(std::string const &value, bool targetSigned, unsigned int targetBits, char const *what, char const *where)
{
	char const * const safeWhat = what ? what : "(unnamed value)";
	char const * const safeWhere = where ? where : "(unknown context)";

	LOG("DatabaseError", ("%s = %s does not fit the %s %u-bit destination (%s). The operation fails; the value is not clamped.",
		safeWhat, value.c_str(), targetSigned ? "signed" : "unsigned", targetBits, safeWhere));
	WARNING(true, ("DatabaseError: %s = %s does not fit the %s %u-bit destination (%s). The operation fails; the value is not clamped.",
		safeWhat, value.c_str(), targetSigned ? "signed" : "unsigned", targetBits, safeWhere));
}

// ======================================================================
