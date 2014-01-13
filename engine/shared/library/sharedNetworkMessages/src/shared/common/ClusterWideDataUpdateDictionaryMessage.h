// ======================================================================
//
// ClusterWideDataUpdateDictionaryMessage.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataUpdateDictionaryMessage_H
#define INCLUDED_ClusterWideDataUpdateDictionaryMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include "sharedUtility/ValueDictionary.h"

#include <string>

// ======================================================================

class ClusterWideDataUpdateDictionaryMessage : public GameNetworkMessage
{
public: //ctor/dtor
	ClusterWideDataUpdateDictionaryMessage(std::string const & managerName, std::string const & elementNameRegex, ValueDictionary const & dictionary, bool replaceDictionary, bool autoRemove, unsigned long lockKey);
	explicit ClusterWideDataUpdateDictionaryMessage(Archive::ReadIterator & source);
	~ClusterWideDataUpdateDictionaryMessage();

public: // methods
	std::string const &                  getManagerName() const;
	std::string const &                  getElementNameRegex() const;
	ValueDictionary const &              getDictionary() const;
	bool                                 getReplaceDictionary() const;
	bool                                 getAutoRemove() const;
	unsigned long                        getLockKey() const;

public:
	// message name
	static std::string const                             ms_messageName;

private: 
	Archive::AutoVariable<std::string>                   m_managerName;
	Archive::AutoVariable<std::string>                   m_elementNameRegex;
	Archive::AutoVariable<ValueDictionary>               m_dictionary;
	Archive::AutoVariable<bool>                          m_replaceDictionary;
	Archive::AutoVariable<bool>                          m_autoRemove;
	Archive::AutoVariable<uint32>                        m_lockKey;

	// Disabled.
	ClusterWideDataUpdateDictionaryMessage();
	ClusterWideDataUpdateDictionaryMessage(ClusterWideDataUpdateDictionaryMessage const &);
	ClusterWideDataUpdateDictionaryMessage &operator =(ClusterWideDataUpdateDictionaryMessage const &);
};

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataUpdateDictionaryMessage::getManagerName() const
{
	return m_managerName.get();
}

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataUpdateDictionaryMessage::getElementNameRegex() const
{
	return m_elementNameRegex.get();
}

//-----------------------------------------------------------------------

inline ValueDictionary const & ClusterWideDataUpdateDictionaryMessage::getDictionary() const
{
	return m_dictionary.get();
}

//-----------------------------------------------------------------------

inline bool ClusterWideDataUpdateDictionaryMessage::getReplaceDictionary() const
{
	return m_replaceDictionary.get();
}

//-----------------------------------------------------------------------

inline bool ClusterWideDataUpdateDictionaryMessage::getAutoRemove() const
{
	return m_autoRemove.get();
}

//-----------------------------------------------------------------------

inline unsigned long ClusterWideDataUpdateDictionaryMessage::getLockKey() const
{
	return static_cast<unsigned long>(m_lockKey.get());
}

// ======================================================================

#endif
