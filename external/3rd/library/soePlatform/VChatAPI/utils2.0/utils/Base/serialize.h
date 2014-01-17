#ifndef SOE__SERIALIZE_H
#define SOE__SERIALIZE_H

// this file has been split into two sections 
// to enable separate inclusion in problematic cases
// with GCC 3.4.4

#include "serializeClasses.h"

#ifdef USE_SERIALIZE_STRING_VECTOR
#include "serializeStringVector.h"
#endif

#include "serializeTemplates.h"

#endif
