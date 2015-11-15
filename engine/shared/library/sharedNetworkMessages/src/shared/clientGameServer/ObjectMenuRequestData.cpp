// ======================================================================
//
// ObjectMenuRequestData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h"
#include "LocalizationManager.h"

//----------------------------------------------------------------------

const uint8 ObjectMenuRequestData::F_enabled      = 0x0001;
const uint8 ObjectMenuRequestData::F_serverNotify = 0x0002;
const uint8 ObjectMenuRequestData::F_outOfRange   = 0x0004;

//----------------------------------------------------------------------

void ObjectMenuRequestData::setOutOfRange  (bool b)
{
	if (b)
		m_flags |= F_outOfRange;
	else
		m_flags &= ~F_outOfRange;
}

//-----------------------------------------------------------------------

void ObjectMenuRequestData::setServerNotify  (bool b)
{
	if (b)
		m_flags |= F_serverNotify;
	else
		m_flags &= ~F_serverNotify;
}
//----------------------------------------------------------------------

