//======================================================================
//
// ChatAvatarId.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatAvatarId_H
#define INCLUDED_ChatAvatarId_H

#include <string>
#include "Unicode.h"

//======================================================================

struct ChatAvatarId
{
	std::string gameCode;
	std::string cluster;
	std::string name;

	         ChatAvatarId ();
	explicit ChatAvatarId (const std::string & name);
	         ChatAvatarId (const std::string & cluster, const std::string & name);
	         ChatAvatarId (const std::string & gameCode, const std::string & cluster, const std::string & name);

	const bool operator== (const ChatAvatarId & rhs) const;
	const bool operator!= (const ChatAvatarId & rhs) const;
	const bool operator < (const ChatAvatarId & rhs) const;

	std::string getNameWithNecessaryPrefix(const std::string & localGameCode, const std::string & localCluster) const;
	std::string getFullName () const;
	void        clear       ();
	Unicode::String  getAPIAddress  () const;
	Unicode::String  getName        () const;
};

//======================================================================

#endif
