// ======================================================================
//
// RestartServerMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_RestartServerMessage_H
#define	_INCLUDED_RestartServerMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class RestartServerMessage: public GameNetworkMessage
{
public:
	RestartServerMessage(std::string const & scene, int x, int z);
	RestartServerMessage(Archive::ReadIterator &source);
	~RestartServerMessage();

	std::string const & getScene() const;
	int getX() const;
	int getZ() const;

private:
	RestartServerMessage(RestartServerMessage const &);
	RestartServerMessage &operator=(RestartServerMessage const &);

private:
	Archive::AutoVariable<std::string> m_scene;
	Archive::AutoVariable<int> m_x;
	Archive::AutoVariable<int> m_z;
};

// ======================================================================

#endif

