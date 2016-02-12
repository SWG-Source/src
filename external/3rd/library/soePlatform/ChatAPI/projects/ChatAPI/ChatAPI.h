#if !defined (CHATAPI_H_)
#define CHATAPI_H_

#define CHAT_API_VERSION "080107"
// update this on each code drop, or even better, on each ChatAPI.h modification

#include "ChatEnum.h"

namespace ChatSystem 
{
	class ChatAPICore;
	class ChatAvatar;
	class ChatRoom;
	class ChatFriendStatus;
	class ChatIgnoreStatus;
    class AvatarListItem;
	class RoomParams;
	class RoomSummary;
	class RoomParams;
	class PersistentHeader;
	class PersistentMessage;
	class AvatarSnoopPair;

// class ChatAPI
//
// Description:
//    This class is the interface to the ChatSystem.  It must be derived by 
//    another class so that its callback functions may be overridden.
//
//    Key features include:
//      - Single-threaded API.
//      - Rooms.  Rooms support moderators, moderation (non-moderators muted),
//          kick, ban, invite (private rooms), room messaging, changing of
//          room parameters.
//      - Messaging.  Instant, Room, Broadcast, and Persistent messaging.
//      - Friends/Ignores.  Friend on/offline notification and avatar ignore.
//      - Searching.  Room objects returned through callbacks offer iterators
//          for their avatar, moderator, ban and invite lists.  API calls
//          include requests for a chat avatar, a chat room, or a list of
//          summarized chat rooms.
//      - Tracking.  Each request is associated with a void *user, which
//          is returned in the callback.
//
// Usage:
//    Derive the ChatAPI and pass the constructor the hostname and port of the
//    ChatSystem you wish to connect the API to.
//
//    Implement the callback functions in your derived class, so that you can
//    receive room and avatar specific information, as well as status of the
//    completion of your Request function calls.
//
//    Continuously call the Process() method to guarantee prompt sending and
//    receipt of messages to and from the ChatServer.  This can be done in
//    a simple worker process whose job is just to call Process().
//
//    ChatAvatar objects returned from callbacks are returned as follows:
//      1) Request callbacks, On___(), always return the locally cached
//         avatar's pointer, the same one returned from request callback
//         OnLoginAvatar().  This pointer is valid until avatar logs out.
//      2) Message callbacks, OnReceive___(), will return ChatAvatar
//         pointers that cannot be guaranteed after the callback returns.
//         Typically, the avatar information from the callback is not
//         needed beyond the life of the callback anyway, but the equal-
//         operator can be used to copy a ChatAvatar object to another.
//
//    ChatRoom objects returned from callbacks are returned as follows:
//      1) Any callback that returns a ChatRoom pointer, except for
//         OnReceiveDestroyRoom() and OnUnregisterRoom(), returns a
//         pointer to the locally cached room and that pointer will
//         be valid until either of the preceding two callbacks are
//         called and return.
//      2) ChatRoom objects do not have the equal-operator overloaded,
//         therefore they cannot be copied, but all the data is
//         accessible from the pointer returned by the callback.
//
//    See the comments for each Request function and callback method for
//    specific information about each.
//
// ChatSystem Details:
//	  The ChatSystem is an arbitrary hierarchy of nodes allowing for deployment
//	  of chat systems in remote data centersr that can be linked to the central
//	  chat tree. Messages only do a long distance hop when they get routed out
//	  of a given data center. Node organization from the gamecode name down
//	  is entirely up to the game team, and can be reorganized to allow for further
//    chat segmentation should subtrees become overloaded with traffic
//																									->Guild Channel #1 (room)	
//																				->Guild Channels	->Guild Channel #2 (room)
//												->EQ		->EQ WestCoast #1	->Zone Channels		->Zone channel #1 (room)
//					-> West Coast Datacenter	->SWG		->SWG WestCoast #1
//		SOE (root)											->SWG WestCoast #2
//					-> East Coast Dataceneter	->EQ		->EQ EastCoast #1
//												->SWG		->SWG EastCoast #1
//															->SWG EastCoast #2	-> Chat Channels	-> SWG Chat #1 (room)
//
// Chat addresses are built from the names of the nodes using a '+' character for a delimiter. so Guild Channel #1 above would be
// SOE+West Coast Datacenter+EQ+EQ WestCoast #1+GuildChannels+Guild Channel #1

	class ChatAPI
	{
	public:
		// ---- CONSTRUCTOR methods ----
		
		// ChatAPI
		//    Constructs a ChatAPI from a hostname and port of the ChatServer it
		//    will be connected to, as well as the hostname and port of the Registrar
		//    ChatServer (which will automatically reroute the ChatAPI connection to 
		//    a hotspare ChatServer if the provided choice is unavailable).
		//    nullptr pointers are NOT valid input.
		ChatAPI(const char *registrar_host, short registrar_port, const char *server_host, short server_port);
		virtual ~ChatAPI();

		// setRequestTimeout
		//	  This overrides the default request timeout of 180 seconds for outstanding
		//    client requests. It will be used for all new requests, but will not change
		//    the timeout of previously submitted requests. 
		void setRequestTimeout(unsigned requestTimeout);

		// ---- AVATAR creation/destruction/find methods ----

		// RequestLoginAvatar
		//    Logs in an avatar from station ID, avatar name, and avatar address.
		//    If the avatar's first login, the ChatSystem implicitly creates the 
		//	  Chat avatar.  Returns a trackID.
		unsigned RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user);

		// RequestLoginAvatar
		//	  Logs in avatar as above, but sends login location, which will be set on avatar object,
		//	  in order to provide tracking information to game teams.
		unsigned RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &avatarLoginLocation, void *user);

		// RequestLoginAvatar
		//	  Logs in avatar as above, but sends login priority as well, which will be set on avatar object.
		//	  Login priority must be <= login priority of logged-in avatar in order to force logout. For use with external vs. in-game chat.
		unsigned RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &avatarLoginLocation, int avatarLoginPriority, void *user);

        // RequestLoginAvatar
        //    Logs in avatar as above, but also sends avatar attributes to be applied at login.
        unsigned RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &avatarLoginLocation, int avatarLoginPriority, int avatarLoginAttributes, void *user);

        // RequestTemporaryAvatar
        //    Logs in a temporary avatar.  A temporary avatar has no friends and is never recorded
        //    in the database.  There is no login priority (the first one must be logged out before another can be logged in).
        //    This method is intended primarily for providing universal friends a way to 'hook' into the message routing scheme.
        unsigned RequestTemporaryAvatar(unsigned userID, const ChatUnicodeString& avatarName, const ChatUnicodeString& avatarAddress, const ChatUnicodeString& avatarLoginLocation, void* user );

		// RequestLogoutAvatar
		//    Logs out an avatar, given the avatar's information.  Returns a 
		//    trackID.
		unsigned RequestLogoutAvatar(const ChatAvatar *srcAvatar, void *user);

		// RequestDestroyAvatar
		//    Logs out an avatar if logged in and destroys all ChatSystem records
		//    of the avatar.  Returns a trackID.
		unsigned RequestDestroyAvatar(const ChatAvatar *srcAvatar, void *user);

		// RequestGetAvatar
		//    Requests a ChatAvatar object for the avatar with the given name and
		//    address.  Returns a trackID.
		unsigned RequestGetAvatar(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user);

        // RequestGetAnyAvatar
        //    As above, but also returns an avatar which may be offline. Useful
        //    for checking if an avatar exists.  Returns a trackID.
        unsigned RequestGetAnyAvatar(const ChatUnicodeString& avatarName, const ChatUnicodeString& avatarAddress, void* user);

        // RequestAvatarList
        //    Returns all avatars owned by the given userID, regardless if
        //    the avatar is online or not.  Note that this does not work across
        //    servers, i.e., only avatars created on this ChatAPI's server will 
        //    be returned.  This is a somewhat expensive call and is not intended
        //    to be used frequently.
        unsigned RequestAvatarList(unsigned userID, void* user);

        //      
		// RequestSetAvatarAttributes
		//    Sets the attributes (e.g. invisibility) for a given avatar.
		//    Returns a trackID.
		unsigned RequestSetAvatarAttributes(const ChatAvatar *srcAvatar, unsigned long avatarAttributes, void *user);

        // RequestSetAvatarSessionAttributes
        //    Sets the attributes (e.g. invisibility) for a given avatar, as above.
        //    However, these attributes are not persistent and last only for the duration of the avatar's session.
        unsigned RequestSetAvatarSessionAttributes(const ChatAvatar *srcAvatar, unsigned long avatarAttributes, void *user);

        // RequestSetAvatarStatusMessage
        //    Sets a 'status' message for the given avatar. This is similar to, but much more flexible than the
        //    'attributes' methods. The status message can be any string and is scoped to the avatar's session.
        //    This method takes the 'Universal' out of 'Universal Chat' because it does not define or require a common
        //    set of strings to be used - that's sort of the point really.
        unsigned RequestSetAvatarStatusMessage(const ChatAvatar* srcAvatar, const ChatUnicodeString& statusMessage, void* user);

		// RequestSetAvatarForwardingEmail
		//    Sets the forwading email address for an avatar. A string with length < 2 indicates no forwarding.
		//    However, the actual address used will always be the Station email account, for security reasons.
		//    Returns a trackID.
		unsigned RequestSetAvatarForwardingEmail(const ChatAvatar *srcAvatar, const ChatUnicodeString &avatarForwardingEmail, void *user);

		// RequestSetAvatarInboxLimit
		//    Sets the inbox limit (max. number of persistent messages) for a given avatar.
		//    Returns a trackID.
		unsigned RequestSetAvatarInboxLimit(const ChatAvatar *srcAvatar, unsigned long avatarInboxLimit, void *user);

		// RequestFindAvatarByUID
		//    Requests a list of ChatAvatar logins by user (station) ID. Returns
		//    a trackID (note that this only returns avatrs which are online).
		unsigned RequestFindAvatarByUID(unsigned userID, void *user);

        //    
		// getAvatar
		//    Requests immediate return of a ChatAvatar object that this API
		//    has cached due to a RequestLoginAvatar.  Request does not go 
		//    to ChatServer.  Returns nullptr if API does not have object locally.
		ChatAvatar *getAvatar(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress);
		ChatAvatar *getAvatar(unsigned avatarID);

		// isUID
		//    Requests if the string is a valid game/namespace Chat node in the ChatSystem
		//      example: isUID("SOE+SWG") is good form, isUID("SWG") will fail
		bool isUID(const ChatUnicodeString &uidNode);

		// setDefaultLoginLocation
		//		Sets a default value to be used as the login location when the 
		//		version of RequestLoginAvatar is called that does not include that argument.
		void setDefaultLoginLocation(const ChatUnicodeString &defaultLoginLocation);

		// setDefaultLoginPriority
		//		Sets a default value to be used as the login priority when the 
		//		version of RequestLoginAvatar is called that does not include that argument.
		void setDefaultLoginPriority(int defaultLoginPriority);

		// ---- AVATAR transfer methods ----

		// RequestTransferAvatar
		//    Transfers an avatar, which optionally changes name, address and 
		//    userID (stationID).  Do not call this individually for each change, 
		//    instead for all required changes at once.  The avatar must be 
		//    logged out of Chat.  The game is required to first guarantee that
		//    the destination avatar is valid/unused, as Chat will overwrite any
		//    existing destination data. Persistent messages are not transferred by default,
		//	  but the version that includes the transferPersistentMessages argument allows it.
		//    Returns a trackID.
		unsigned RequestTransferAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned newUserID, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &newAvatarAddress, void *user);
		unsigned RequestTransferAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned newUserID, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &newAvatarAddress, bool transferPersistentMessages, void *user);

		// ---- AVATAR keyword manipulation methods ----
		
		// RequestSetAvatarKeywords
		//    Sets the keywords on the specified avatar allowing them to be searched on within the chat system
		unsigned RequestSetAvatarKeywords(const ChatAvatar *srcAvatar, const ChatUnicodeString *keywordList, unsigned keywordLength, void *user);
		
		// RequestGetAvatarKeywords
		//    Gets the keywords previously set on the specified avatar
		unsigned RequestGetAvatarKeywords(const ChatAvatar *srcAvatar, void *user);

		// RequestSearchAvatarKeywords
		//    Searches the keywords on within the chat system and returns a list of avatars that match
		unsigned RequestSearchAvatarKeywords(const ChatUnicodeString &nodeAddress, const ChatUnicodeString *keywordList, unsigned keywordLength, void *user);

		// ---- FANCLUB handle lookup method ----
		
		// RequestFanClubHandle
		//    Obtains a fan club handle and relevant information given the
		//    user's station ID and fan club code.
		unsigned RequestFanClubHandle(unsigned stationID, unsigned fanClubCode, void *user);

		// ---- ROOM creation/destruction/params methods ----

		// RequestCreateRoom
		//    Creates a room for user srcAvatar, from the room parameters (includes
		//    room name) and the parent node's address.  The new room's address
		//    will be "parentAddress+roomName". Returns a trackID.
		unsigned RequestCreateRoom(const ChatAvatar *srcAvatar, const RoomParams &roomParams, const ChatUnicodeString &parentAddress, void *user);
		
		// RequestDestroyRoom
		//    Destroys a room for user srcAvatar, using roomAddress.
		//    The avatar must have created the room to destroy it.  Returns a 
		//    trackID.
		unsigned RequestDestroyRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, void *user);
		
		// RequestGetRoom
		//    Requests a ChatRoom object for the room with the given address.
		//    Returns a trackID.
		unsigned RequestGetRoom(const ChatUnicodeString &roomAddress, void *user);

		// getRoom
		//    Requests immediate return of a ChatRoom object that this API
		//    has cached due to a RequestGetRoom, RequestCreateRoom, or 
		//    RequestEnterRoom.  Request does not go to ChatServer.  Returns 
		//    nullptr if API does not have object locally.
		ChatRoom *getRoom(const ChatUnicodeString &roomAddress);
		ChatRoom *getRoom(unsigned roomID);

		// RequestGetRoomSummaries
		//    Requests a list of RoomSummary objects for the rooms matching the
		//    given room name filter and at or below the node startNodeAddress.
		//    Returns a trackID.
		unsigned RequestGetRoomSummaries(const ChatUnicodeString &startNodeAddress, const ChatUnicodeString &roomFilter, void *user);
		
		// getDefaultRoomParams
		//    Requests the ChatAPI's current default RoomParams object. Used for
		//    the EnterRoom call. If nullptr, EnterRoom will not do passive room
		//    creation if the room to enter does not yet exist. Initial value
		//    is nullptr (no RoomParams object defined).
		const RoomParams *getDefaultRoomParams(void);

		// setDefaultRoomParams
		//    Sets the ChatAPI's current default RoomParams object. Passing a nullptr
		//    pointer will cause the default params not to be used by EnterRoom.
		void setDefaultRoomParams(RoomParams *roomParams);
		
		// ---- MESSAGING methods ----

		// RequestSendInstantMessage
		//    Sends an instant message from srcAvatar to the avatar of the given
		//    name and address.  Supports out-of-band data which the ChatSystem
		//    does not use but delivers to provide additional content transfer.
		//    Returns a trackID.
		unsigned RequestSendInstantMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user);
		
		// RequestSendRoomMessage
		//    Sends a room message from srcAvatar to the room of given 
		//    room address.  Also supports out-of-band data.  Returns a trackID.
		unsigned RequestSendRoomMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user);

		// RequestSendBroadcastMessage
		//    Sends a broadcast message from srcAvatar to the nodes at and below
		//    the node located at destAddress.  Also supports out-of-band data.
		//    Returns a trackID.
		unsigned RequestSendBroadcastMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destNodeAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user);

		// RequestFilterMessage
		//    Filters the message 
		//    Returns a trackID.
		unsigned RequestFilterMessage(const ChatUnicodeString &in_msg, void *user);

		// RequestFilterMessageEx
		//    Filters the message 
		//    Returns a trackID.
		unsigned RequestFilterMessageEx(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &in_msg, void *user);

		// ---- FRIEND methods ----

		// RequestAddFriend
		//    Adds a friend for srcAvatar for the avatar described by destName and
		//    destAddress.  If parameter "confirm" is set to true, then the destAvatar
		//    is given opportunity to accept/decline the request.  When that avatar's
		//    online status changes, notification is sent to srcAvatar's connected API.
		//    Returns a trackID.
		unsigned RequestAddFriend(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, bool confirm, void *user);
		unsigned RequestAddFriendReciprocate(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, bool confirm, void *user);

		// RequestAddFriend
		//    Adds a friend as above, and also set the "friend comment" field, requested by SWG.
		unsigned RequestAddFriend(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &friendComment, bool confirm, void *user);
		unsigned RequestAddFriendReciprocate(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &friendComment, bool confirm, void *user);
		
		// RequestSetFriendComment
		//    Sets the "friend comment" field, requested by SWG, for the avatar.
		//	  The friend must already exist.
		unsigned RequestSetFriendComment(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &friendComment, void *user);
		
		// RequestRemoveFriend
		//    Removes a friend for srcAvatar for the avatar described by destName
		//    and destAddress.  Online status change notifications for the 
		//    described avatar will cease being delivered to srcAvatar.  Returns a
		//    trackID.
		unsigned RequestRemoveFriend(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);
		unsigned RequestRemoveFriendReciprocate(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);

		// RequestFriendStatus
		//    Requests the friend list for avatar srcAvatar.  Returns a trackID.
		unsigned RequestFriendStatus(const ChatAvatar *srcAvatar, void *user);
		
		// RequestFriendConfirm
		//    Acknowledges a RequestAddFriend call from another avatar. Call after
		//    receipt of OnReceiveFriendConfirmRequest callback.  Submitting a
		//    value of "true" for parameter "confirm" will cause friend association
		//    to be made.  srcAvatar is the avatar on this API that was requested
		//    friendship from destAvatar.  This means the srcAvatar in above callback
		//    is the destAvatar in this request.  Remember that the validity of
		//    srcAvatar in the callback ends when the callback ends, thus should be
		//    copied first or this should be called within that callback to assure
		//    passing a valid ChatAvatar as destAvatar.
		unsigned RequestFriendConfirm(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, bool confirm, void *user);
		unsigned RequestFriendConfirmReciprocate(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, bool confirm, void *user);

		// ---- IGNORE methods ----

		// RequestAddIgnore
		//    Adds an ignore for srcAvatar for any messages originating from the
		//    avatar described by destName and destAddress.  Returns a trackID.
		unsigned RequestAddIgnore(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);

		// RequestRemoveIgnore
		//    Removes an ignore for srcAvatar for any messages originating from
		//    the avatar described by destName and destAddress. Returns a trackID.
		unsigned RequestRemoveIgnore(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);

		// RequestIgnoreStatus
		//   Requests the ignore list for the avatar srcAvatar. Returns a trackID.
		unsigned RequestIgnoreStatus(const ChatAvatar *srcAvatar, void *user);

		// ---- ROOM enter/leave methods ----
		
		// RequestEnterRoom
		//    Enters srcAvatar into the room defined by destRoomAddress.  If the
		//    room does not yet exist and the default RoomParams object is defined
		//    by setDefaultRoomParams(), then the ChatSystem will attempt to
		//    passively create the room.  Returns a trackID.
		unsigned RequestEnterRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestEnterRoom
		//    Enters room, as above, but provides a password,
		//    to be used if room is private and has a password.
		unsigned RequestEnterRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destRoomPassword, void *user);

		// RequestEnterRoom
		//    Enters room, as above, but provides a Boolean value stating whether delayed entry may occur
		//	  if the room is private and the owner chooses to allow the avatar to enter.
		unsigned RequestEnterRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destRoomPassword, bool requestEntry, void *user);

		// RequestAllowRoomEntry
		//	  Allows an avatar that has previously attempted to enter a room with requestEntry set to true
		//    to enter the specified room, or deny such entry to the avatar if desired. 
		unsigned RequestAllowRoomEntry(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, bool allow, void *user);

		// RequestLeaveRoom
		//    Removes srcAvatar from the room defined by destRoomAddress.  Returns
		//    a trackID.
		unsigned RequestLeaveRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, void *user);

		// setDefaultEntryType
		//		Sets a default value to be used as the requestEntry value when the 
		//		version of RequestEnterRoom is called that does not include that argument.
		void setDefaultEntryType(bool defaultEntryType);

		// ---- ROOM admin/moderator methods ----

		// RequestAddModerator
		//    Adds destAvatar as a moderator to room described by 
		//    destRoomAddress.  The srcAvatar must be the creator or a GM.  Returns a 
		//    trackID.
		unsigned RequestAddModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestRemoveModerator
		//    Removes destAvatar as a moderator to room described by 
		//    destRoomAddress.  The srcAvatar must be the creator or a GM.  Returns a
		//    trackID.
		unsigned RequestRemoveModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestAddTemporaryModerator
		//    Adds destAvatar as a temporary moderator to room described by 
		//    destRoomAddress.  This status will be lost when the affected avatar leaves the room.
		//	  The srcAvatar must be the creator or a GM.  Returns a trackID.
		unsigned RequestAddTemporaryModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestRemoveTemporaryModerator
		//    Removes destAvatar as a temporary moderator to room described by 
		//    destRoomAddress.  The srcAvatar must be the creator or a GM.  Returns a
		//    trackID.
		unsigned RequestRemoveTemporaryModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestAddBan
		//    Bans and kicks destAvatar from the room described by 
		//    destRoomAddress.  The srcAvatar must be the creator.  Returns a
		//    trackID.
		unsigned RequestAddBan(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestRemoveBan
		//    Unbans destAvatar from the room described by 
		//    destRoomAddress.  The srcAvatar must be the creator.  Returns a
		//    trackID.
		unsigned RequestRemoveBan(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestAddInvite
		//    Adds an invitation for destAvatar for the room described by 
		//    destRoomAddress.  The srcAvatar must be the creator
		//    or a moderator.  This only applies to private rooms.  Returns a
		//    trackID.
		unsigned RequestAddInvite(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestRemoveInvite
		//    Removes an invitation for destAvatar for the room described by
		//    destRoomAddress.  The srcAvatar must be the creator
		//    or a moderator.  This only applies to private rooms.  Removing 
		//    invitation indicates that destAvatar will not succeed on any 
		//    further EnterRoom requests, but does not kick destAvatar if in-room.
		//    Returns a trackID.
		unsigned RequestRemoveInvite(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestGrantVoice
		//    Grants the destAvatar the right to speak in the room described by 
		//    destRoomAddress.  The srcAvatar must be the creator, a moderator, or a GM.
		//	  This only applies to moderated rooms.  Returns a trackID.
		unsigned RequestGrantVoice(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestRevokeVoice
		//    Revokes the right of destAvatar to speak in the room described by
		//    destRoomAddress.  The srcAvatar must be the creator, a GM, 
		//    or a moderator.  This only applies to moderated rooms.  Revoking 
		//    voice indicates that destAvatar will not succeed on any 
		//    further SendRoomMessage requests.
		//    Returns a trackID.
		unsigned RequestRevokeVoice(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestKickAvatar
		//    Kicks destAvatar from the room described by 
		//    destRoomAddress.  The srcAvatar must be the creator or a moderator.
		//    Returns a trackID.
		unsigned RequestKickAvatar(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user);

		// RequestSetRoomParams
		//    Sets new room parameters for the room described by 
		//    destRoomAddress.  The name parameter cannot be changed and is
		//    ignored from the RoomParams object.  Returns a trackID.
		unsigned RequestSetRoomParams(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const RoomParams *params, void *user);

		// RequestChangeRoomOwner
		//	  Changes the owner of a room to the specified avatar,
		//	  if the avatar exists. srcAvatar must be current owner or GM.
		//    Returns a trackID.
		unsigned RequestChangeRoomOwner(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);

		// ---- PERSISTENT MESSAGE methods ----

		// RequestSendPersistentMessage
		//    Sends a persistent message from srcAvatar to a destination name and address
		//	  "category" is an optional argument, stored with the message for classification purposes
		//	  The "enforceInboxLimit" specifies whether this message should be subject to inbox size limits; it defaults to "true" in the calls that don't include that parameter.
		unsigned RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user);
		unsigned RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user);
		unsigned RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user);
		unsigned RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user);

		// RequestSendPersistentMessage
		//	  Sends a persistent message from an arbitrary source (such as an in game NPC) to a destination avatar
		//		NOTE that this message cannot be replyed to
		//	  "category" is an optional argument, stored with the message for classification purposes
		//	  The "enforceInboxLimit" specifies whether this message should be subject to inbox size limits; it defaults to "true" in the calls that don't include that parameter.
		unsigned RequestSendPersistentMessage(const ChatUnicodeString &srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user);
		unsigned RequestSendPersistentMessage(const ChatUnicodeString &srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user);
		unsigned RequestSendPersistentMessage(const ChatUnicodeString &srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user);
		unsigned RequestSendPersistentMessage(const ChatUnicodeString &srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user);

		// RequestSendMultiplePersistentMessages
		//    Sends the same persistent message from srcAvatar to the destination names and addresses
		//	  The "enforceInboxLimit" specifies whether this message should be subject to inbox size limits; it defaults to "true" in the calls that don't include that parameter.
		unsigned RequestSendMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user);
		unsigned RequestSendMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user);
		unsigned RequestSendMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user);

		// RequestSendMultiplePersistentMessages
		//	  Sends the same persistent message from an arbitrary source (such as an in game NPC) to the destination avatars
		//		NOTE that this message cannot be replyed to
		//	  The "enforceInboxLimit" specifies whether this message should be subject to inbox size limits; it defaults to "true" in the calls that don't include that parameter.
		unsigned RequestSendMultiplePersistentMessages(const ChatUnicodeString &srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user);
		unsigned RequestSendMultiplePersistentMessages(const ChatUnicodeString &srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user);
		unsigned RequestSendMultiplePersistentMessages(const ChatUnicodeString &srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user);

		// RequestAlterPersistentMessage
		//	  Alters various fields in a persistent message, based on the specified attributes from PersistentAlterationTypes.
		unsigned RequestAlterPersistentMessage(const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID, unsigned alterationAttributes, const ChatUnicodeString &newSenderName, const ChatUnicodeString &newSenderAddress,  const ChatUnicodeString &newSubject, const ChatUnicodeString &newMsg, const ChatUnicodeString &newOOB, const ChatUnicodeString &newCategory, unsigned newSentTime, void *user);

		// RequestGetPersistentHeaders
		//	  Retrieves persistent message headers for the given avatar.
		//	  "category" field is optional; no category or empty category matches all messages
		unsigned RequestGetPersistentHeaders(const ChatAvatar *srcAvatar, void *user);
		unsigned RequestGetPersistentHeaders(const ChatAvatar *srcAvatar, const ChatUnicodeString &category, void *user);

		// RequestGetPartialPersistentHeaders
		//	  Retrieves up to maxHeaders (or all header is this value is zero) 
		//	  persistent message headers for the given avatar,
		//	  either up to sentTimeStart or after sentTimeStart, depending on the value of 
		//    inDescendingOrder. Setting sentTimeStart to zero will retrieve either the 
		//    most recent or oldest messages, depending on the value of inDescendingOrder.
		//	  "category" field is optional; no category or empty category matches all messages
		unsigned RequestGetPartialPersistentHeaders(const ChatAvatar *srcAvatar, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, void *user);
		unsigned RequestGetPartialPersistentHeaders(const ChatAvatar *srcAvatar, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category, void *user);

		// RequestCountPersistentMessages
		//	  Retrieves the number persistent messages for the given avatar.
		//	  "category" field is optional; no category or empty category matches all messages
		unsigned RequestCountPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user);
		unsigned RequestCountPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category, void *user);

		// RequestGetPersistentMessage
		//	  Retrieves the message body and out of band data for a given message header
		unsigned RequestGetPersistentMessage(const ChatAvatar *srcAvatar, unsigned messageID, void *user);

		// RequestGetMultiplePersistentMessages
		//	  Retrieves ALL message bodies for messages matching the given criteria, 
		//	  which are the same as for RequestGetPartialPersistentHeaders
		unsigned RequestGetMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category, void *user);

		// RequestUpdatePersistentMessage
		//	  Updates a persistent message's status.  Note that all statuses are applied
		//    except for PERSISTENT_DELETED, which causes immediate and permanent removal
		//    of the message.
		unsigned RequestUpdatePersistentMessage(const ChatAvatar *srcAvatar, unsigned messageID, PersistentStatus newStatus, void *user);

		// RequestUpdatePersistentMessages
		//    Changes status of all persistent messages received by srcAvatar that are 
		//    set at currentStatus and updates their status to newStatus.  If newStatus
		//    is PERSISTENT_DELETED, those messages are immediately and permanently
		//    removed.  This is particularly useful for emptying the PERSISTENT_TRASH
		//    messages, but can be used for any statuses except from PERSISTENT_DELETED.
		//	  "category" field is optional; no category or empty category matches all messages
		unsigned RequestUpdatePersistentMessages(const ChatAvatar *srcAvatar, PersistentStatus currentStatus, PersistentStatus newStatus, void *user);
		unsigned RequestUpdatePersistentMessages(const ChatAvatar *srcAvatar, PersistentStatus currentStatus, PersistentStatus newStatus, const ChatUnicodeString &category, void *user);

		// RequestClassifyPersistentMessages
		//	  Updates the folder for a set of persistent messages. Can be called with a ChatAvater pointer or a name and address, 
		//	  but the avatar must be local to the server that the API is connected to (i.e. not a different game/world)
		unsigned RequestClassifyPersistentMessages(const ChatAvatar *srcAvatar, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder, void *user);
		unsigned RequestClassifyPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder, void *user);

		// RequestDeleteAllPersistentMessages
		//    Deletes all persistent messages received by srcAvatar.
		//	  "category" and "olderThan" fields are optional; not supplying these matches all messages.
		//    Supplying olderThan field deletes only messages older than the given date-time.
		unsigned RequestDeleteAllPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user);
		unsigned RequestDeleteAllPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category, unsigned olderThan, void *user);

		// ---- ROOM CACHING methods ----

		// RequestUnregisterRoom
		//    To be called after receiving the callback OnReadyToUnregisterRoom(),
		//    which indicated no more avatars from this API are logged in to a
		//    given room.  When there is no further reason to keep the ChatRoom*
		//    valid and to receive room updates from that room, then this should
		//    be called to unregister the room, stopping room updates.
		unsigned RequestUnregisterRoom(const ChatUnicodeString &roomAddress, void *user);

		// ---- SNOOPING methods ----

		// RequestBeginSnoopingAvatar
		//   Begins snooping the destAvatar by srcAvatar.  The srcAvatar must have the
		//   rights to do so, as determined by avatar attrbiutes: the GM attribute allows 
		//   snooping within the same game/project namespace (i.e. SOE+SWG), and the 
		//   superSnoop attribute allows snooping outside of one's game/project namespace.
		//   Returns a trackID.
		unsigned RequestBeginSnoopingAvatar(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);
		
		// RequestStopSnoopingAvatar
		//   Stops snooping the destAvatar by srcAvatar.  If destAvatar has already logged
		//   out, then this call is not necessary and you will receive an OnReceiveSnoopMessage
		//   with snoopType SNOOP_END which indicates the snooping has automatically stopped.
		//   Returns a trackID.
		unsigned RequestStopSnoopingAvatar(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user);
		
		// RequestBeginSnoopingRoom
		//   Begins snooping the room by srcAvatar.  The srcAvatar must have the
		//   rights to do so, as determined by avatar attrbiutes: the GM attribute allows 
		//   snooping within the same game/project namespace (i.e. SOE+SWG), and the 
		//   superSnoop attribute allows snooping outside of one's game/project namespace.
		//   Returns a trackID.
		unsigned RequestBeginSnoopingRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user);

		// RequestStopSnoopingRoom
		//   Stops snooping the room by srcAvatar.  If the room has already destructed,
		//   then this call is not necessary and you will receive an OnReceiveSnoopMessage
		//   with snoopType SNOOP_END which indicates the snooping has automatically stopped.
		//   Returns a trackID.
		unsigned RequestStopSnoopingRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user);

		// RequestGetSnoopList
		//   Requests the snooping list for the avatar srcAvatar. Returns a trackID.
		unsigned RequestGetSnoopList(const ChatAvatar *srcAvatar, void *user);


		// ---- PROCESS method ----

		// Process
		//    Must be called to process incoming and outgoing messages between the
		//    API and the ChatServer.
		void Process();

		// ---- CHATAPI connect/disconnect/failover callback methods ----

		// OnConnect
		//    Called when the ChatAPI has established connection to the ChatServer
		//    and is ready to accept requests.
		virtual void OnConnect();

		// OnDisconnect
		//    Called when the ChatAPI has lost connection to the ChatServer.
		//    This may be followed by an OnFailoverBegin command if connection
		//    is re-established and failover mode is initiated.
		virtual void OnDisconnect();

		// OnFailoverBegin
		//    Called when the ChatAPI has restored connection to the ChatServer and
		//    is attempting failover to restore validity of its state.  It is 
		//    retaining its queued requests for server submission upon 
		//    OnFailoverComplete.  OnConnect is not called during failover attempt.
		virtual void OnFailoverBegin();

		// OnFailoverComplete
		//    Called when the ChatAPI has restored its connection to the
		//    ChatServer and restored its state.  It is submitting the queued
		//    requests and ready to accept new requests.
		virtual void OnFailoverComplete();

		// ---- REQUEST CALLBACK methods ----
		// All callbacks are results of their related Request__ methods.
		// Check parameter "result" for value 0 (success) before using any
		//   returned data.

		virtual void OnLoginAvatar(unsigned track, unsigned result, const ChatAvatar *newAvatar, void *user);
		virtual void OnLoginAvatar(unsigned track, unsigned result, const ChatAvatar *newAvatar, int requiredLoginPriority, void *user);
        virtual void OnTemporaryAvatar(unsigned track, unsigned result, const ChatAvatar* newAvatar, void* user );
		virtual void OnLogoutAvatar(unsigned track, unsigned result, const ChatAvatar *oldAvatar, void *user);
		virtual void OnDestroyAvatar(unsigned track, unsigned result, const ChatAvatar *oldAvatar, void *user);
		virtual void OnGetAvatar(unsigned track, unsigned result, const ChatAvatar *foundAvatar, void *user);
        virtual void OnGetAnyAvatar(unsigned track, unsigned result, const ChatAvatar *foundAvatar, bool loggedIn, void *user);
        virtual void OnAvatarList(unsigned track, unsigned result, unsigned listLength, const AvatarListItem* avatarList, void* user);
		virtual void OnSetAvatarAttributes(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user);
        virtual void OnSetAvatarStatusMessage(unsigned track, unsigned result, const ChatAvatar* changedAvatar, void* user );
		virtual void OnSetAvatarForwardingEmail(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user);
		virtual void OnSetAvatarInboxLimit(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user);
		virtual void OnFindAvatarByUID(unsigned track, unsigned result, unsigned numFoundAvatarsOnline, ChatAvatar **foundAvatars, void *user);

		virtual void OnTransferAvatar(unsigned track, unsigned result, unsigned oldUserID, unsigned newUserID, const ChatUnicodeString &oldName, const ChatUnicodeString &newName, const ChatUnicodeString &oldAddress, const ChatUnicodeString &newAddress, void *user);

		virtual void OnFanClubHandle(unsigned track, unsigned result, const ChatUnicodeString &handle, unsigned stationID, unsigned fanClubCode, void *user);

		virtual void OnCreateRoom(unsigned track,unsigned result, const ChatRoom *newRoom, void *user);
		virtual void OnDestroyRoom(unsigned track, unsigned result, void *user);
		virtual void OnEnterRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnAllowRoomEntry(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user);
		virtual void OnLeaveRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);

		virtual void OnGetRoom(unsigned track, unsigned result, const ChatRoom *room, void *user);
		virtual void OnGetRoomSummaries(unsigned track, unsigned result, unsigned numFoundRooms, RoomSummary *foundRooms, void *user);

		virtual void OnSendInstantMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnSendRoomMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnSendBroadcastMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &address, void *user);
		virtual void OnFilterMessage(unsigned track, unsigned result, const ChatUnicodeString &filteredMsg, void *user);
		virtual void OnFilterMessageEx(unsigned track, unsigned result, const ChatUnicodeString &filteredMsg, void *user);

		virtual void OnAddFriend(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnAddFriendReciprocate(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnSetFriendComment(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment, void *user);
		virtual void OnRemoveFriend(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnRemoveFriendReciprocate(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnFriendStatus(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned listLength, const ChatFriendStatus *friendList, void *user);
		virtual void OnFriendConfirm(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnFriendConfirmReciprocate(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);

		virtual void OnAddIgnore(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnRemoveIgnore(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnIgnoreStatus(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned listLength, const ChatIgnoreStatus *ignoreList, void *user);

		virtual void OnAddModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnRemoveModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnAddTemporaryModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnRemoveTemporaryModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnAddBan(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnRemoveBan(unsigned track,unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnAddInvite(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnRemoveInvite(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnGrantVoice(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnRevokeVoice(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnKickAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnSetRoomParams(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		virtual void OnChangeRoomOwner(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user);
		
		virtual void OnSendPersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnSendPersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned messageID, void *user);
		virtual void OnSendMultiplePersistentMessages(unsigned track, unsigned result, unsigned numSubResults, const unsigned *subResults, const ChatAvatar *srcAvatar, void *user);
		virtual void OnSendMultiplePersistentMessages(unsigned track, unsigned result, unsigned numSubResults, const unsigned *subResults, const unsigned *messageIDs, const ChatAvatar *srcAvatar, void *user);
		virtual void OnAlterPersistentMessage(unsigned track, unsigned result, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID);
		virtual void OnGetPersistentHeaders(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, const PersistentHeader *list, void *user);
		virtual void OnGetPartialPersistentHeaders(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, const PersistentHeader *list, void *user);
		virtual void OnCountPersistentMessages(unsigned track, unsigned result, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numberOfMessages, void *user);
		virtual void OnGetPersistentMessage(unsigned track, unsigned result, ChatAvatar *destAvatar, const PersistentHeader *header, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user);
		virtual void OnGetMultiplePersistentMessages(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, PersistentMessage ** const list, void *user);
		virtual void OnUpdatePersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnUpdatePersistentMessages(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnClassifyPersistentMessages(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnDeleteAllPersistentMessages(unsigned track, unsigned result, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numberDeleted, void *user);

		virtual void OnUnregisterRoom(unsigned track, unsigned result, const ChatRoom *destRoom, void *user);

		virtual void OnSetAvatarKeywords(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user);
		virtual void OnGetAvatarKeywords(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString *keywordList, unsigned listLength, void *user);
		virtual void OnSearchAvatarKeywords(unsigned track, unsigned result, ChatAvatar **avatarMatches, unsigned numAvatarMatches, void *user);

		virtual void OnBeginSnoopingAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnStopSnoopingAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user);
		virtual void OnBeginSnoopingRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user);
		virtual void OnStopSnoopingRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user);
		virtual void OnGetSnoopList(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned avatarSnoopListLength, AvatarSnoopPair **avatarSnoops, unsigned roomSnoopListLength, ChatUnicodeString **roomSnoops, void *user);
		
		// getErrorString
		//    This is a helper function that returns a brief description of the result code
		//    returned by the server.
		static ChatUnicodeString getErrorString(unsigned resultCode);

		// ---- RECEIVE CALLBACK methods ----
		// All callbacks are results of messages from the ChatServer, such as a
		//   room update, a message receipt, or an avatar kick. See each's comments
		//   for information about when it is generated.
		// Do not use returned data pointers after callback completes.  Instead,
		//   copy the data inside the callback to local structures.

		// OnReceiveUnregisterRoomReady
		//    No more avatars connected to this API are participating in the
		//    given room, thus it should be unregistered to prevent further
		//    room updates by calling RequestUnregisterRoom(), unless the API
		//    needs to keep the ChatRoom* for this room valid and to continue
		//    receiving room updates.
		virtual void OnReceiveUnregisterRoomReady(const ChatRoom *destRoom);
		
		// OnReceiveInstantMessage
		//    Called when destAvatar, connected to this API, receives an instant
		//    message from srcAvatar.
		virtual void OnReceiveInstantMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &msg, const ChatUnicodeString &oob);
		
		// OnReceiveRoomMessage
		//    Called when srcAvatar sends a room message to destRoom.  This is
		//    called for each destAvatar on this API that should receive this
		//    message. Both versions will be called; handle only ONE with server logic.
		virtual void OnReceiveRoomMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom, const ChatUnicodeString &msg, const ChatUnicodeString &oob);
		virtual void OnReceiveRoomMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom, const ChatUnicodeString &msg, const ChatUnicodeString &oob, unsigned messageID);

		// OnReceiveBroadcastMessage
		//    Called when srcAvatar sends a broadcast message to node srcAddress
		//    and its nodes below.  This is called for each destAvatar on this API
		//    that should receive this message.
		virtual void OnReceiveBroadcastMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar, const ChatUnicodeString &msg, const ChatUnicodeString &oob);

		// OnReceiveFilterMessage
		virtual void OnReceiveFilterMessage(const ChatUnicodeString &msg);

		// OnReceiveFriendLogin
		//    Called when srcAvatar has logged in.  The destAvatar is on this API
		//    and has requested to receive notifications about srcAvatar.  The
		//    srcAddress is the AvatarID node sending the notification.
		virtual void OnReceiveFriendLogin(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar);

		// OnReceiveFriendLogout
		//    Called when srcAvatar has logged out.  The destAvatar is on this API
		//    and has requested to receive notifications about srcAvatar.  The
		//    srcAddress is the AvatarID node sending the notification.
		virtual void OnReceiveFriendLogout(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar);

        // OnReceiveFriendStatusChange
        //    Called when a friend of destAvatar has changed their status message.
        //    The destAvatar is on this API and has requestd to receive notifications
        //    about srcAvatar.
        virtual void OnReceiveFriendStatusChange(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar);

		// OnReceiveFriendConfirmRequest
		//    Called when srcAvatar is asking destAvatar for confirmation to be added to srcAvatar's friend list
		virtual void OnReceiveFriendConfirmRequest(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar);

		// OnReceiveFriendConfirmResponse
		//    Called when srcAvatar responds to destAvatar's request for friend confirmation. Contains whether or not the attempt to add
		// the friend was successful or not
		virtual void OnReceiveFriendConfirmResponse(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, bool confirmed);

		// OnReceiveFriendConfirmRequest
		//    Called when srcAvatar is asking destAvatar for confirmation to be added to srcAvatar's friend list
		virtual void OnReceiveFriendConfirmReciprocateRequest(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar);

		// OnReceiveFriendConfirmResponse
		//    Called when srcAvatar responds to destAvatar's request for friend confirmation. Contains whether or not the attempt to add
		// the friend was successful or not
		virtual void OnReceiveFriendConfirmReciprocateResponse(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, bool confirmed);

		// OnReceiveKickRoom
		//    Called when destAvatar has been kicked by srcAvatar from destRoom
		//    and this API has avatars that are in destRoom.
		virtual void OnReceiveKickRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveKickAvatar
		//    Called when destAvatar has been kicked by srcAvatar from the room
		//    described by roomName and roomAddress, and destAvatar is on
		//    this API.
		virtual void OnReceiveKickAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRoomEntryRequest
		//		Called when srcAvatar has attempted to enter the room at destRoomAddress with
		//		m_requestingEntry set to true and has been denied entry because the room is private.
		//		The API through which the room's owner (destAvatar) is logged in will receive this callback.
		virtual void OnReceiveRoomEntryRequest(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &destRoomAddress);

		// OnReceiveDelayedRoomEntry
		//		Called when srcAvatar has attempted to enter the room at destRoomAddress with
		//		m_requestingEntry set to true, was initiallly denied entry because the room is private,
		//		but has been allowed into the room by the owner. This is essentially a delayed OnEnterRoom callback, 
		//		and it will be followed by an OnReceiveEnterRoom notification.
		virtual void OnReceiveDelayedRoomEntry(const ChatAvatar *srcAvatar, const ChatRoom *destRoom);

		// OnReceiveDeniedRoomEntry
		//		Called when srcAvatar has attempted to enter the room at destRoomAddress with
		//		m_requestingEntry set to true, was initiallly denied entry because the room is private,
		//		and has been denied again room by the owner. The avatar will not be in the room.
		virtual void OnReceiveDeniedRoomEntry(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress);

		// OnReceiveEnterRoom
		//    Called when srcAvatar has entered destRoom and this API has avatars
		//    that are in destRoom. If srcAvatar is on this API and no others from
		//    this API are in the room, this is not called.
		virtual void OnReceiveEnterRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom);

		// OnReceiveLeaveRoom
		//    Called when srcAvatar has left destRoom and this API has avatars
		//    that are in destRoom.
		virtual void OnReceiveLeaveRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom);

		// OnReceiveDestroyRoom
		//    Called when srcAvatar has destroyed destRoom and this API has avatars
		//    that were in destRoom.
		virtual void OnReceiveDestroyRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom);

		// OnReceiveAddModeratorRoom
		//    Called when srcAvatar has granted destAvatar moderator privileges in
		//    destRoom and this API has avatars that are in destRoom.
		virtual void OnReceiveAddModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveAddModeratorAvatar
		//    Called when srcAvatar has granted destAvatar moderator privileges in
		//    the room described by roomName and roomAddress, and destAvatar
		//    is on this API.
		virtual void OnReceiveAddModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRemoveModeratorRoom
		//    Called when srcAvatar has removed destAvatar moderator privileges in
		//    destRoom and this API has avatars that are in destRoom.
		virtual void OnReceiveRemoveModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveRemoveModeratorAvatar
		//    Called when srcAvatar has removed destAvatar moderator privileges from
		//    the room described by roomName and roomAddress, and destAvatar is on
		//    this API.
		virtual void OnReceiveRemoveModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveAddTemporaryModeratorRoom
		//    Called when srcAvatar has granted destAvatar temporary moderator privileges in
		//    destRoom and this API has avatars that are in destRoom.
		virtual void OnReceiveAddTemporaryModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveAddTemporaryModeratorAvatar
		//    Called when srcAvatar has granted destAvatar temporary moderator privileges in
		//    the room described by roomName and roomAddress, and destAvatar
		//    is on this API.
		virtual void OnReceiveAddTemporaryModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRemoveTemporaryModeratorRoom
		//    Called when srcAvatar has removed destAvatar temporary moderator privileges in
		//    destRoom and this API has avatars that are in destRoom.
		virtual void OnReceiveRemoveTemporaryModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveRemoveTemporaryModeratorAvatar
		//    Called when srcAvatar has removed destAvatar temporary moderator privileges from
		//    the room described by roomName and roomAddress, and destAvatar is on
		//    this API.
		virtual void OnReceiveRemoveTemporaryModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveAddBanRoom
		//    Called when srcAvatar has banned destAvatar from destRoom and this
		//    API has avatars that are in destRoom.
		virtual void OnReceiveAddBanRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveAddBanAvatar
		//    Called when srcAvatar has banned destAvatar from the room described by
		//    roomName and roomAddress, and destAvatar is on this API.
		virtual void OnReceiveAddBanAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRemoveBanRoom
		//    Called when srcAvatar has removed the ban on destAvatar from
		//    destRoom and this API has avatars that are in destRoom.
		virtual void OnReceiveRemoveBanRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveRemoveBanAvatar
		//    Called when srcAvatar has removed the ban on destAvatar from
		//    the room described by roomName and roomAddress, and destAvatar is on
		//    this API.
		virtual void OnReceiveRemoveBanAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveAddInviteRoom
		//    Called when srcAvatar has invited destAvatar to destRoom and this
		//    API has avatars that are in this room.
		virtual void OnReceiveAddInviteRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveAddInviteAvatar
		//    Called when srcAvatar has invited destAvatar to the room described
		//    by roomName and roomAddress, and destAvatar is on this API.
		virtual void OnReceiveAddInviteAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRemoveInviteRoom
		//    Called when srcAvatar has uninvited destAvatar from destRoom and
		//    this API has avatars that are in this room.
		virtual void OnReceiveRemoveInviteRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveRemoveInviteAvatar
		//    Called when srcAvatar has uninvited destAvatar from the room 
		//    described by roomName and roomAddress, and destAvatar is on 
		//    this API.
		virtual void OnReceiveRemoveInviteAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveGrantVoiceRoom
		//    Called when srcAvatar has granted voice to destAvatar in destRoom and this
		//    API has avatars that are in the room.
		virtual void OnReceiveGrantVoiceRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveGrantVoiceAvatar
		//    Called when srcAvatar has granter voice to destAvatar in the room described
		//    by roomName and roomAddress, and destAvatar is on this API.
		virtual void OnReceiveGrantVoiceAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRevokeVoiceRoom
		//    Called when srcAvatar has revoked the voice privilege for destAvatar in destRoom and
		//    this API has avatars that are in this room.
		virtual void OnReceiveRevokeVoiceRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom);

		// OnReceiveRevokeVoiceAvatar
		//    Called when srcAvatar has revoked voice for destAvatar in the room 
		//    described by roomName and roomAddress, and destAvatar is on 
		//    this API.
		virtual void OnReceiveRevokeVoiceAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress);

		// OnReceiveRoomParams
		//    Called when the room parameters have been changed for destRoom by
		//    srcAvatar and this API has avatars that are in this room.
		virtual void OnReceiveRoomParams(const ChatAvatar *srcAvatar, const ChatRoom *destRoom, const RoomParams *params, const RoomParams *oldParams);

		// OnReceivePersistentMessage
		//	  Called when a persistent message is sent to an avatar who is logged in
		//    at the time it's received
		virtual void OnReceivePersistentMessage(const ChatAvatar *destAvatar, const PersistentHeader *header);

		// OnReceiveForcedLogout
		//    Called when an avatar is logged in from a different API.
		//    The first instance of the avatar will be logged out
		virtual void OnReceiveForcedLogout(const ChatAvatar *oldAvatar);

		// OnReceiveRoomOwnerChange
		//	  Called when the owner of a room has been changed. 
		virtual void OnReceiveRoomOwnerChange(const ChatRoom *destRoom);

		// OnReceiveSnoopMessage
		//    Called when a message is sniffed during transmission to/from
		//    an avatar or room that is currently being snooped by an avatar
		//    logged in through this API.
		//    NOTE: types are defined in ChatEnum.h and SNOOP_END should be
		//       caught to detect snoop stream end due to destAvatar logout or
		//       destRoom destruction
		virtual void OnReceiveSnoopMessage(unsigned snoopType,
										   const ChatUnicodeString &snooperName,
										   const ChatUnicodeString &snooperAddress,
										   const ChatUnicodeString &srcName,     // msg sender
										   const ChatUnicodeString &srcAddress,  // msg sender
										   const ChatUnicodeString &destName,    // dest avatar, invalid if room is destination
										   const ChatUnicodeString &destAddress, // dest avatar or room address
										   const ChatUnicodeString &message);

		// OnReceiveNotifyFriendIsRemoved
		//    Called when destAvatar has been removed by srcAvatar from the friend list
		//    described by destAvatarID, destName and destAddress is on this API.
		virtual void OnReceiveNotifyFriendIsRemoved(ChatAvatar * avatar, const ChatUnicodeString &friendName, const ChatUnicodeString &friendAddress);

		// OnReceiveNotifyFriendsListChange
		// Called After an AvatarTransfer.  If the avatar had any friends this will be
		// called for each friend. 
		// avatar - the friend that should be updated
		// originalName - the name before the transfer
		// originalAddress - the address before the transfer
		// newName - the name after the transfer
		// newAddress - the address after the transfer
		virtual void OnReceiveNotifyFriendsListChange(ChatAvatar * avatar,
													  const ChatUnicodeString &originalName,     
													  const ChatUnicodeString &originalAddress,  
													  const ChatUnicodeString &newName,  
													  const ChatUnicodeString &newAddress);

	protected:
		// Do not use this constructor
		ChatAPI(ChatAPICore *core);

	private:
		ChatAPICore *m_core;
		RoomParams *m_defaultRoomParams;
		ChatUnicodeString m_defaultLoginLocation;
		int m_defaultLoginPriority;
		bool m_defaultEntryType;
	};
};

#endif

