// ======================================================================
//
// ClusterWideDataGetElementMessage.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataGetElementMessage_H
#define INCLUDED_ClusterWideDataGetElementMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

// ======================================================================

class ClusterWideDataGetElementMessage : public GameNetworkMessage
{
public: //ctor/dtor
	ClusterWideDataGetElementMessage(std::string const & managerName, std::string const & elementNameRegex, bool lockElements);
	explicit ClusterWideDataGetElementMessage(Archive::ReadIterator & source);
	~ClusterWideDataGetElementMessage();

public: // methods
	std::string const & getManagerName() const;
	std::string const & getElementNameRegex() const;
	bool                getLockElements() const;
	unsigned long       getRequestId() const;

public:
	// message name
	static std::string const             ms_messageName;

private: 
	Archive::AutoVariable<std::string>   m_managerName;
	Archive::AutoVariable<std::string>   m_elementNameRegex;
	Archive::AutoVariable<bool>          m_lockElements;
	Archive::AutoVariable<uint32>        m_requestId;

	// for assigning m_requestId
	static unsigned long                 ms_requestIdCounter;

	// Disabled.
	ClusterWideDataGetElementMessage();
	ClusterWideDataGetElementMessage(ClusterWideDataGetElementMessage const &);
	ClusterWideDataGetElementMessage &operator =(ClusterWideDataGetElementMessage const &);
};

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataGetElementMessage::getManagerName() const
{
	return m_managerName.get();
}

//-----------------------------------------------------------------------

inline std::string const & ClusterWideDataGetElementMessage::getElementNameRegex() const
{
	return m_elementNameRegex.get();
}

//-----------------------------------------------------------------------

inline bool ClusterWideDataGetElementMessage::getLockElements() const
{
	return m_lockElements.get();
}

//-----------------------------------------------------------------------

inline unsigned long ClusterWideDataGetElementMessage::getRequestId() const
{
	return static_cast<unsigned long>(m_requestId.get());
}

// ======================================================================

#endif
