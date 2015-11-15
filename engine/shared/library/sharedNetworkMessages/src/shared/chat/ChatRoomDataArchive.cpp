//======================================================================
//
// ChatRoomDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatRoomDataArchive.h"

#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "unicodeArchive/UnicodeArchive.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatRoomData & target)
	{
		get (source, target.id);
		get (source, target.roomType);
		get (source, target.moderated);
		get (source, target.path);	
		get (source, target.owner);	
		get (source, target.creator);	
		get (source, target.title);	
		int moderatorCount;
		get (source, moderatorCount);
		int i;
		target.moderators.clear();
		for(i = 0; i < moderatorCount; ++i)
		{
			ChatAvatarId id;
			get(source, id);
			target.moderators.push_back(id);
		}
		int inviteCount;
		get(source, inviteCount);
		for(i = 0; i < inviteCount; ++i)
		{
			ChatAvatarId id;
			get(source, id);
			target.invitees.push_back(id);
		}
	}

	void put (ByteStream & target, const ChatRoomData & source)
	{
		put (target, source.id);
		put (target, source.roomType);
		put (target, source.moderated);
		put (target, source.path);
		put (target, source.owner);
		put (target, source.creator);
		put (target, source.title);	
		int c = static_cast<int>(source.moderators.size());
		put (target, c);
		std::vector<ChatAvatarId>::const_iterator i;
		for(i = source.moderators.begin(); i != source.moderators.end(); ++i)
		{
			put(target, (*i));
		}

		c = static_cast<int>(source.invitees.size());
		put(target, c);
		for(i = source.invitees.begin(); i != source.invitees.end(); ++i)
		{
			put(target, (*i));
		}
	}
}

//======================================================================
