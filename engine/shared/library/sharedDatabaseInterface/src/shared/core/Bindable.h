/* Bindable.h
 *
 * Defines classes based on built-in simple types that know how to bind themselves to 
 * database queries and support nullptr values.
 *
 * Each class supports the following functions:
 * bool isNull    -- test whether the value is nullptr
 * void setToNull -- set the value to nullptr
 *      getValue  -- return the value (make sure you test for nullptr before calling this.  The results
 *                   are undefined if the Bindable is nullptr.)
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
