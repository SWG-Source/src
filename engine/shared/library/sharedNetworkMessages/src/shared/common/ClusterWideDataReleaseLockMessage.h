// ======================================================================
//
// ClusterWideDataReleaseLockMessage.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataReleaseLockMessage_H
#define INCLUDED_ClusterWideDataReleaseLockMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

// ======================================================================

class ClusterWideDataReleaseLockMessage : public GameNetworkMessage
{
public: //ctor/dtor
	ClusterWideDataReleaseLockMessage(std::string const & managerName, unsigned long lockKey);
	explicit ClusterWideDataReleaseLockMessage(Archive::ReadIterator & source);
	~ClusterWideDataReleaseLockMessage();

public: // methods
	std::string const &                  getManagerName() const;
	unsigned long                        getLockKey() const;

public:
	// message name
	static std::string const                             ms_messageName;

private: 
	Archive::AutoVariable<std::string>                   m_managerName;
	Archive::AutoVariable<uint32>                        m_lockKey;

	// Disabled.
	ClusterWideDataReleaseLockMessage();
	ClusterWideDataReleaseLockMessage(ClusterWideDataReleaseLockMessage const &);
	ClusterWideDataReleaseLockMessage &operator =(ClusterWideDataReleaseLockMessage const &);
};

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataReleaseLockMessage::getManagerName() const
{
	return m_managerName.get();
}

//-----------------------------------------------------------------------

inline unsigned long ClusterWideDataReleaseLockMessage::getLockKey() const
{
	return static_cast<unsigned long>(m_lockKey.get());
}

// ======================================================================

#endif
