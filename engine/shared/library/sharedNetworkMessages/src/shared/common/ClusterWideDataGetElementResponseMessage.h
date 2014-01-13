// ======================================================================
//
// ClusterWideDataGetElementResponseMessage.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataGetElementResponseMessage_H
#define INCLUDED_ClusterWideDataGetElementResponseMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

// ======================================================================

class ValueDictionary;

// ======================================================================

class ClusterWideDataGetElementResponseMessage : public GameNetworkMessage
{
public: //ctor/dtor
	ClusterWideDataGetElementResponseMessage(std::string const & managerName, std::string const & elementNameRegex, std::vector<std::string> const & elementNameList, std::vector<ValueDictionary> const & elementDictionaryList, unsigned long requestId, unsigned long lockKey);
	explicit ClusterWideDataGetElementResponseMessage(Archive::ReadIterator & source);
	~ClusterWideDataGetElementResponseMessage();

public: // methods
	std::string const &                  getManagerName() const;
	std::string const &                  getElementNameRegex() const;
	unsigned long                        getRequestId() const;
	unsigned long                        getLockKey() const;
	std::vector<std::string> const &     getElementNameList() const;
	std::vector<ValueDictionary> const & getElementDictionaryList() const;

public:
	// message name
	static std::string const                             ms_messageName;

private: 
	Archive::AutoVariable<std::string>                   m_managerName;
	Archive::AutoVariable<std::string>                   m_elementNameRegex;
	Archive::AutoVariable<uint32>                        m_requestId;
	Archive::AutoVariable<uint32>                        m_lockKey;
	Archive::AutoArray<std::string>                      m_elementNameList;
	Archive::AutoArray<ValueDictionary>                  m_elementDictionaryList;

	// Disabled.
	ClusterWideDataGetElementResponseMessage();
	ClusterWideDataGetElementResponseMessage(ClusterWideDataGetElementResponseMessage const &);
	ClusterWideDataGetElementResponseMessage &operator =(ClusterWideDataGetElementResponseMessage const &);
};

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataGetElementResponseMessage::getManagerName() const
{
	return m_managerName.get();
}

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataGetElementResponseMessage::getElementNameRegex() const
{
	return m_elementNameRegex.get();
}

//-----------------------------------------------------------------------

inline unsigned long ClusterWideDataGetElementResponseMessage::getRequestId() const
{
	return static_cast<unsigned long>(m_requestId.get());
}

//-----------------------------------------------------------------------

inline unsigned long ClusterWideDataGetElementResponseMessage::getLockKey() const
{
	return static_cast<unsigned long>(m_lockKey.get());
}

//-----------------------------------------------------------------------

inline std::vector<std::string> const & ClusterWideDataGetElementResponseMessage::getElementNameList() const
{
	return m_elementNameList.get();
}

//-----------------------------------------------------------------------

inline std::vector<ValueDictionary> const & ClusterWideDataGetElementResponseMessage::getElementDictionaryList() const
{
	return m_elementDictionaryList.get();
}

// ======================================================================

#endif
