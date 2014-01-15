// ============================================================================
// 
// CommunityManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_CommunityManager_H
#define INCLUDED_CommunityManager_H

class ChatOnChangeFriendStatus;
class ChatOnGetFriendsList;
class ChatOnChangeIgnoreStatus;
class ChatOnGetIgnoreList;

//-----------------------------------------------------------------------------
class CommunityManager
{
public:

	static void handleMessage(ChatOnChangeFriendStatus const &message);
	static void handleMessage(ChatOnGetFriendsList const &message);
	static void handleMessage(ChatOnChangeIgnoreStatus const &message);
	static void handleMessage(ChatOnGetIgnoreList const &message);

private:

	// Disable

	CommunityManager();
	~CommunityManager();
	CommunityManager(CommunityManager const &);
	CommunityManager &operator =(CommunityManager const &);
};

// ============================================================================

#endif // INCLUDED_CommunityManager_H
