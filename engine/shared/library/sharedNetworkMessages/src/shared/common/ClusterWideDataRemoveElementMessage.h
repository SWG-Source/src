// ======================================================================
//
// ClusterWideDataRemoveElementMessage.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataRemoveElementMessage_H
#define INCLUDED_ClusterWideDataRemoveElementMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

// ======================================================================

class ClusterWideDataRemoveElementMessage : public GameNetworkMessage
{
public: //ctor/dtor
	ClusterWideDataRemoveElementMessage(std::string const & managerName, std::string const & elementNameRegex, unsigned long lockKey);
	explicit ClusterWideDataRemoveElementMessage(Archive::ReadIterator & source);
	~ClusterWideDataRemoveElementMessage();

public: // methods
	std::string const &                  getManagerName() const;
	std::string const &                  getElementNameRegex() const;
	unsigned long                        getLockKey() const;

public:
	// message name
	static std::string const                             ms_messageName;

private: 
	Archive::AutoVariable<std::string>                   m_managerName;
	Archive::AutoVariable<std::string>                   m_elementNameRegex;
	Archive::AutoVariable<uint32>                        m_lockKey;

	// Disabled.
	ClusterWideDataRemoveElementMessage();
	ClusterWideDataRemoveElementMessage(ClusterWideDataRemoveElementMessage const &);
	ClusterWideDataRemoveElementMessage &operator =(ClusterWideDataRemoveElementMessage const &);
};

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataRemoveElementMessage::getManagerName() const
{
	return m_managerName.get();
}

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataRemoveElementMessage::getElementNameRegex() const
{
	return m_elementNameRegex.get();
}

//-----------------------------------------------------------------------

inline unsigned long ClusterWideDataRemoveElementMessage::getLockKey() const
{
	return static_cast<unsigned long>(m_lockKey.get());
}

// ======================================================================

#endif
