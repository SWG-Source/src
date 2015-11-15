// ======================================================================
//
// ClusterWideDataGetElementResponseMessage.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClusterWideDataGetElementResponseMessage.h"

#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueDictionaryArchive.h"

// ======================================================================

std::string const ClusterWideDataGetElementResponseMessage::ms_messageName = "ClusterWideDataGetElementResponseMessage";

// ======================================================================

ClusterWideDataGetElementResponseMessage::ClusterWideDataGetElementResponseMessage(std::string const & managerName, std::string const & elementNameRegex, std::vector<std::string> const & elementNameList, std::vector<ValueDictionary> const & elementDictionaryList, unsigned long const requestId, unsigned long const lockKey) :
	GameNetworkMessage(ms_messageName),
	m_managerName(managerName),
	m_elementNameRegex(elementNameRegex),
	m_requestId(static_cast<uint32>(requestId)),
	m_lockKey(static_cast<uint32>(lockKey)),
	m_elementNameList(),
	m_elementDictionaryList()
{
	m_elementNameList.set(elementNameList);
	m_elementDictionaryList.set(elementDictionaryList);

	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_requestId);
	addVariable(m_lockKey);
	addVariable(m_elementNameList);
	addVariable(m_elementDictionaryList);
}

// ----------------------------------------------------------------------

ClusterWideDataGetElementResponseMessage::ClusterWideDataGetElementResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(ms_messageName),
	m_managerName(),
	m_elementNameRegex(),
	m_requestId(0),
	m_lockKey(0),
	m_elementNameList(),
	m_elementDictionaryList()
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_requestId);
	addVariable(m_lockKey);
	addVariable(m_elementNameList);
	addVariable(m_elementDictionaryList);

	unpack(source);
}

// ----------------------------------------------------------------------

ClusterWideDataGetElementResponseMessage::~ClusterWideDataGetElementResponseMessage()
{
}

// ======================================================================
