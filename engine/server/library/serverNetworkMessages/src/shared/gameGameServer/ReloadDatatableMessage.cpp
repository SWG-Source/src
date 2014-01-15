// ======================================================================
//
// ReloadDatatableMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ReloadDatatableMessage.h"

const char * const ReloadDatatableMessage::ms_messageName = "ReloadDatatableMessage";

// ======================================================================

ReloadDatatableMessage::ReloadDatatableMessage(const std::string & table) :
	GameNetworkMessage(ms_messageName),
	m_table(table)
{
	addVariable(m_table);
}

// ----------------------------------------------------------------------

ReloadDatatableMessage::ReloadDatatableMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(ms_messageName),
	m_table()
{
	addVariable(m_table);
	unpack(source);
}

// ----------------------------------------------------------------------

ReloadDatatableMessage::~ReloadDatatableMessage()
{
}

// ======================================================================

