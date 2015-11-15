/* Bindable.h
 *
 * Defines classes based on built-in simple types that know how to bind themselves to 
 * database queries and support null values.
 *
 * Each class supports the following functions:
 * bool isNull    -- test whether the value is null
 * void setToNull -- set the value to null
 *      getValue  -- return the value (make sure you test for null before calling this.  The results
 *                   are undefined if the Bindable is null.)
 *      setValue  -- set the value
 *
 * ODBC version
 *
 * Including this header brings in all the bindable types.
 */

#ifndef _BINDABLE_H
#define _BINDABLE_H

#include "sharedDatabaseInterface/DbBindableBase.h"
#include "sharedDatabaseInterface/DbBindableBitArray.h"
#include "sharedDatabaseInterface/DbBindableBool.h"
#include "sharedDatabaseInterface/DbBindableDouble.h"
#include "sharedDatabaseInterface/DbBindableInt64.h"
#include "sharedDatabaseInterface/DbBindableLong.h"
#include "sharedDatabaseInterface/DbBindableString.h"
#include "sharedDatabaseInterface/DbBindableUnicode.h"

#endif
