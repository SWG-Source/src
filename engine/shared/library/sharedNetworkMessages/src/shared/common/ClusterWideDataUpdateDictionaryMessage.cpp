// ======================================================================
//
// ClusterWideDataUpdateDictionaryMessage.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClusterWideDataUpdateDictionaryMessage.h"

#include "sharedUtility/ValueDictionaryArchive.h"

// ======================================================================

std::string const ClusterWideDataUpdateDictionaryMessage::ms_messageName = "ClusterWideDataUpdateDictionaryMessage";

// ======================================================================

ClusterWideDataUpdateDictionaryMessage::ClusterWideDataUpdateDictionaryMessage(std::string const & managerName, std::string const & elementNameRegex, ValueDictionary const & dictionary, bool const replaceDictionary, bool const autoRemove, unsigned long const lockKey) :
	GameNetworkMessage(ms_messageName),
	m_managerName(managerName),
	m_elementNameRegex(elementNameRegex),
	m_dictionary(dictionary),
	m_replaceDictionary(replaceDictionary),
	m_autoRemove(autoRemove),
	m_lockKey(static_cast<uint32>(lockKey))
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_dictionary);
	addVariable(m_replaceDictionary);
	addVariable(m_autoRemove);
	addVariable(m_lockKey);
}

// ----------------------------------------------------------------------

ClusterWideDataUpdateDictionaryMessage::ClusterWideDataUpdateDictionaryMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(ms_messageName),
	m_managerName(),
	m_elementNameRegex(),
	m_dictionary(),
	m_replaceDictionary(false),
	m_autoRemove(false),
	m_lockKey(0)
{
	addVariable(m_managerName);
	addVariable(m_elementNameRegex);
	addVariable(m_dictionary);
	addVariable(m_replaceDictionary);
	addVariable(m_autoRemove);
	addVariable(m_lockKey);

	unpack(source);
}

// ----------------------------------------------------------------------

ClusterWideDataUpdateDictionaryMessage::~ClusterWideDataUpdateDictionaryMessage()
{
}

// ======================================================================
