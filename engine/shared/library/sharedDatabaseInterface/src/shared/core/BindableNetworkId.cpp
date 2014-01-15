// ======================================================================
//
// BindableNetworkId.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"

using namespace DB;

// ======================================================================

BindableNetworkId::BindableNetworkId()
{
}

// ----------------------------------------------------------------------

BindableNetworkId::BindableNetworkId (const NetworkId &rhs) : BindableInt64(rhs.getValue())
{
}

// ----------------------------------------------------------------------

void BindableNetworkId::setValue(const NetworkId &buffer)
{
	BindableInt64::setValue(buffer.getValue());
}

// ======================================================================
