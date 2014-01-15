// ConnectionServerDown.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConnectionServerDown_H
#define	_INCLUDED_ConnectionServerDown_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ConnectionServerDown : public GameNetworkMessage
{
public:
	explicit ConnectionServerDown(int id);
	ConnectionServerDown(Archive::ReadIterator & source);
	~ConnectionServerDown();

	int       getId() const;

private:
	Archive::AutoVariable<int>      m_id;
};

//-----------------------------------------------------------------------

inline int ConnectionServerDown::getId() const
{
	return m_id.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConnectionServerDown_H
