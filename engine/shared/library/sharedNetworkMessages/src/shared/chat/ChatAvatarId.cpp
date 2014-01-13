//======================================================================
//
// ChatAvatarId.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "UnicodeUtils.h"

//======================================================================

ChatAvatarId::ChatAvatarId () :
gameCode (),
cluster  (),
name     ()
{
}

//----------------------------------------------------------------------

ChatAvatarId::ChatAvatarId (const std::string & _gameCode, const std::string & _cluster, const std::string & _name) :
gameCode (_gameCode),
cluster  (_cluster),
name     (_name)
{
}

//----------------------------------------------------------------------

ChatAvatarId::ChatAvatarId (const std::string & _cluster, const std::string & _name) :
gameCode (),
cluster  (_cluster),
name     (_name)
{
}

//----------------------------------------------------------------------

ChatAvatarId::ChatAvatarId (const std::string & _name) :
gameCode (),
cluster  (),
name     (_name)
{
	size_t dotpos = _name.rfind ('.');

	if (dotpos != std::string::npos)
	{
		name = _name.substr (dotpos + 1);

		if (dotpos > 0)
		{
			--dotpos;
			const size_t dotpos_2 = _name.rfind ('.', dotpos);
			
			if (dotpos_2 != std::string::npos)
			{
				cluster = _name.substr (dotpos_2 + 1, dotpos - dotpos_2);
				gameCode = _name.substr (0, dotpos_2);
			}
			else
				cluster = _name.substr (0, dotpos + 1);
		}
	}
}

//----------------------------------------------------------------------

const bool ChatAvatarId::operator== (const ChatAvatarId & rhs) const
{
	return !_stricmp (gameCode.c_str (), rhs.gameCode.c_str ()) &&
		!_stricmp (cluster.c_str (), rhs.cluster.c_str ()) &&
		!_stricmp (name.c_str (), rhs.name.c_str ());
}

//----------------------------------------------------------------------

const bool ChatAvatarId::operator!= (const ChatAvatarId & rhs) const
{
	return !(rhs == *this);
}

//-----------------------------------------------------------------------

const bool ChatAvatarId::operator < (const ChatAvatarId & rhs) const
{
	if (gameCode < rhs.gameCode)
		return true;
	
	if (gameCode == rhs.gameCode)
	{
		if (cluster < rhs.cluster)
			return true;
		
		if (cluster == rhs.cluster)
			return (name < rhs.name);
	}

	return false;
}

//----------------------------------------------------------------------

std::string ChatAvatarId::getNameWithNecessaryPrefix(const std::string & localGameCode, const std::string & localCluster) const
{
	std::string nameWithNecessaryPrefix;
	if (_stricmp(gameCode.c_str(), localGameCode.c_str()))
	{
		// avatar is from a different game, so use both the game and cluster name
		nameWithNecessaryPrefix = gameCode + std::string(".") + cluster + std::string(".") + name;
	}
	else if (_stricmp(cluster.c_str(), localCluster.c_str()))
	{
		// avatar is from a different cluster, so the cluster name
		nameWithNecessaryPrefix = cluster + std::string(".") + name;
	}
	else
	{
		// avatar is from same cluster, so just use the name
		nameWithNecessaryPrefix = name;
	}

	return nameWithNecessaryPrefix;
}

//----------------------------------------------------------------------

std::string ChatAvatarId::getFullName () const
{
	return gameCode + '.' + cluster + '.' + name;
}

//----------------------------------------------------------------------

void ChatAvatarId::clear       ()
{
	gameCode.clear ();
	cluster.clear  ();
	name.clear     ();
}

//----------------------------------------------------------------------

Unicode::String ChatAvatarId::getAPIAddress() const
{
	return Unicode::narrowToWide( "SOE+" + gameCode + '+' + cluster );
}

//----------------------------------------------------------------------

Unicode::String ChatAvatarId::getName() const
{
	return Unicode::narrowToWide( name );
}

//======================================================================
