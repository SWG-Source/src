#if !defined (CHATROOM_H_)
#define CHATROOM_H_

#pragma warning (disable : 4786)
#pragma warning (disable : 4620)

#include "ChatEnum.h"

namespace ChatSystem 
{


	class ChatAvatar;
	class ChatRoomCore;
	class AvatarIteratorCore;
	class ModeratorIteratorCore;
	class TemporaryModeratorIteratorCore;
	class VoiceIteratorCore;
	class InviteIteratorCore;
	class BanIteratorCore;
	struct RoomParamsCore;
	struct RoomSummaryCore;


	// Room attribute test macros
	//   example: IS_SET(myRoomAttributes, ROOMATTR_PERSISTENT)
	inline unsigned IS_SET(unsigned var, unsigned bit)
	{
		return (var & bit);
	}

	inline unsigned SET_BIT(unsigned &var, unsigned bit)
	{
		var |= bit;
		return(var);
	}

	inline unsigned REMOVE_BIT(unsigned &var, unsigned bit)
	{
		var &= ~bit;
		return(var);
	}

	enum RoomAttributes
	{
		ROOMATTR_PRIVATE = 1<<0,
		ROOMATTR_MODERATED = 1<<1,
		ROOMATTR_PERSISTENT = 1<<2,
		ROOMATTR_LOCAL_WORLD = 1<<4,
		ROOMATTR_LOCAL_GAME = 1<<5
	};


	// class RoomParams
	//
	// Description:
	//    This class stores the basic room parameters for creating a room.
	//
	// Usage:
	//    A RoomParams object is required for creating a room.
	//    It can be copied and there are get__ and set__ methods for retrieving
	//    and storing values.
	//
	class RoomParams
	{
	public:
		RoomParams();
		RoomParams(const ChatUnicodeString &name, const ChatUnicodeString &topic, unsigned attributes, unsigned maxSize);
		RoomParams(const ChatUnicodeString &name, const ChatUnicodeString &topic, const ChatUnicodeString &password, unsigned attributes, unsigned maxSize);
		RoomParams(const RoomParams &orig);
		~RoomParams();

		RoomParams & operator=(const RoomParams &rhs);

		void setRoomName(const ChatUnicodeString &name);
		void setRoomTopic(const ChatUnicodeString &topic);
		void setRoomPassword(const ChatUnicodeString &name);
		void setRoomAttributes(unsigned attributes);
		void setRoomMaxSize(unsigned size);

		const ChatUnicodeString &getRoomName() const;
		const ChatUnicodeString &getRoomTopic() const;
		const ChatUnicodeString &getRoomPassword() const;
		unsigned getRoomAttributes() const;
		unsigned getRoomMaxSize() const;

	private:
		RoomParamsCore *m_core;
		mutable ChatUnicodeString m_cName;
		mutable ChatUnicodeString m_cTopic;
		mutable ChatUnicodeString m_cPassword;
	};


	// class RoomSummary
	//
	// Description:
	//    This class stores the basic room parameters to describe a room.
	//    It is useful for describing a room with less byte resources than
	//    a full ChatRoom.
	//
	// Usage:
	//    A RoomSummary object is passed through OnGetRoomSummaries().
	//    It can be copied and there are get__ and set__ methods for retrieving
	//    and storing values.
	//
	class RoomSummary
	{
	public:
		RoomSummary();
		RoomSummary(const ChatUnicodeString &name, const ChatUnicodeString &topic, unsigned attributes, unsigned curSize, unsigned maxSize);
		~RoomSummary();

		RoomSummary & operator=(const RoomSummary &rhs);

		void setRoomAddress(const ChatUnicodeString &address);
		void setRoomTopic(const ChatUnicodeString &topic);
		void setRoomAttributes(unsigned attributes);
		void setRoomCurSize(unsigned curSize);
		void setRoomMaxSize(unsigned maxSize);

		const ChatUnicodeString &getRoomAddress() const;
		const ChatUnicodeString &getRoomTopic() const;
		unsigned getRoomAttributes() const;
		unsigned getRoomCurSize() const;
		unsigned getRoomMaxSize() const;

	private:
		RoomSummaryCore *m_core;
		mutable ChatUnicodeString m_cAddress;
		mutable ChatUnicodeString m_cTopic;

	};


	// class AvatarIterator
	//
	// Description:
	//    This class implements an iterator for the ChatRoom avatar container.
	//
	// Usage:
	//    An AvatarIterator will originate from getFirstAvatar() or findAvatar().
	//    It can be copied, incremented, decremented, and dereferenced to get the
	//    ChatAvatar* it points to. To check the validity of the iterator while
	//    iterating to know whether the end of the container has been reached,
	//    use outOfBounds() method.
	//        example:
	//            AvatarIterator iter;
	//            for (iter = room->getFirstAvatar(); iter.outOfBounds() == false; iter++)
	//            {
	//                ChatAvatar *avatar = (*iter);
	//                // do stuff with avatar info
	//            }
	//
	class AvatarIterator
	{
	public:
		AvatarIterator();
		AvatarIterator(const AvatarIteratorCore &core);
		~AvatarIterator();

		AvatarIterator & operator=(const AvatarIterator &rhs);

		// Bi-directional container navigation
		void operator++();
		void operator++(int dummyForPostfix);
		void operator--();
		void operator--(int dummyForPostfix);

		// Dereference to ChatAvatar* value
		ChatAvatar * operator*();

		// Test movement past beginning or end of container
		bool outOfBounds();

	private:
		AvatarIteratorCore *m_core;
	};

	// class ModeratorIterator
	//
	// Description:
	//    This class implements an iterator for the ChatRoom moderator container.
	//
	// Usage:
	//    A ModeratorIterator will originate from getFirstModerator() or findModerator().
	//    Same as AvatarIterator usage.
	//
	class ModeratorIterator
	{
	public:
		ModeratorIterator();
		ModeratorIterator(const ModeratorIteratorCore &core);
		~ModeratorIterator();

		ModeratorIterator & operator=(const ModeratorIterator &rhs);

		// Bi-directional container navigation
		void operator++();
		void operator++(int dummyForPostfix);
		void operator--();
		void operator--(int dummyForPostfix);

		// Dereference to ChatAvatar* value
		ChatAvatar * operator*();

		// Test movement past beginning or end of container
		bool outOfBounds();

	private:
		ModeratorIteratorCore *m_core;
	};

	// class TemporaryModeratorIterator
	//
	// Description:
	//    This class implements an iterator for the ChatRoom temporary moderator container.
	//
	// Usage:
	//    A TemporaryModeratorIterator will originate from getFirstModerator() or findModerator().
	//    Same as AvatarIterator usage.
	//
	class TemporaryModeratorIterator
	{
	public:
		TemporaryModeratorIterator();
		TemporaryModeratorIterator(const TemporaryModeratorIteratorCore &core);
		~TemporaryModeratorIterator();

		TemporaryModeratorIterator & operator=(const TemporaryModeratorIterator &rhs);

		// Bi-directional container navigation
		void operator++();
		void operator++(int dummyForPostfix);
		void operator--();
		void operator--(int dummyForPostfix);

		// Dereference to ChatAvatar* value
		ChatAvatar * operator*();

		// Test movement past beginning or end of container
		bool outOfBounds();

	private:
		TemporaryModeratorIteratorCore *m_core;
	};

	// class VoiceIterator
	//
	// Description:
	//    This class implements an iterator for the ChatRoom Voice container.
	//
	// Usage:
	//    An VoiceIterator will originate from getFirstVoice() or findVoice().
	//    Same as AvatarIterator usage.
	//
	class VoiceIterator
	{
	public:
		VoiceIterator();
		VoiceIterator(const VoiceIteratorCore &core);
		~VoiceIterator();

		VoiceIterator & operator=(const VoiceIterator &rhs);

		// Bi-directional container navigation
		void operator++();
		void operator++(int dummyForPostfix);
		void operator--();
		void operator--(int dummyForPostfix);

		// Dereference to ChatAvatar* value
		ChatAvatar * operator*();

		// Test movement past beginning or end of container
		bool outOfBounds();

	private:
		VoiceIteratorCore *m_core;
	};

	// class InviteIterator
	//
	// Description:
	//    This class implements an iterator for the ChatRoom invite container.
	//
	// Usage:
	//    An InviteIterator will originate from getFirstInvite() or findInvite().
	//    Same as AvatarIterator usage.
	//
	class InviteIterator
	{
	public:
		InviteIterator();
		InviteIterator(const InviteIteratorCore &core);
		~InviteIterator();

		InviteIterator & operator=(const InviteIterator &rhs);

		// Bi-directional container navigation
		void operator++();
		void operator++(int dummyForPostfix);
		void operator--();
		void operator--(int dummyForPostfix);

		// Dereference to ChatAvatar* value
		ChatAvatar * operator*();

		// Test movement past beginning or end of container
		bool outOfBounds();

	private:
		InviteIteratorCore *m_core;
	};

	// class BanIterator
	//
	// Description:
	//    This class implements an iterator for the ChatRoom ban container.
	//
	// Usage:
	//    A BanIterator will originate from getFirstBan() or findBan().
	//    Same as AvatarIterator usage.
	//
	class BanIterator
	{
	public:
		BanIterator();
		BanIterator(const BanIteratorCore &core);
		~BanIterator();

		BanIterator & operator=(const BanIterator &rhs);

		// Bi-directional container navigation
		void operator++();
		void operator++(int dummyForPostfix);
		void operator--();
		void operator--(int dummyForPostfix);

		// Dereference to ChatAvatar* value
		ChatAvatar * operator*();

		// Test movement past beginning or end of container
		bool outOfBounds();

	private:
		BanIteratorCore *m_core;
	};

	// class ChatRoom
	//
	// Description:
	//    This class stores the information about a room in the ChatSystem that
	//    is relevant to the API.
	//
	// Usage:
	//    A ChatRoom will originate from an API callback method.
	//    The ChatRoom object cannot be copied however all of its member data can
	//    be retrived either by get__ methods or iterator methods (see comments
	//    for AvatarIterator class).
	//
	class ChatRoom
	{
	public:
		ChatRoom();
		ChatRoom(ChatRoomCore *core);
		~ChatRoom();

		const ChatUnicodeString &getCreatorName() const;
		const ChatUnicodeString &getCreatorAddress() const;
		unsigned                 getCreatorID() const;

		unsigned                 getRoomID() const;
		const ChatUnicodeString &getRoomName() const;
		const ChatUnicodeString &getRoomTopic() const;
		const ChatUnicodeString &getRoomPassword() const;
		unsigned                 getRoomAttributes() const;
		unsigned                 getMaxRoomSize() const;
		const ChatUnicodeString &getAddress() const;

		unsigned		  getAvatarCount() const;
		unsigned		  getBanCount() const;
		unsigned		  getInviteCount() const;
		unsigned		  getModeratorCount() const;
		unsigned		  getTemporaryModeratorCount() const;
		unsigned		  getVoiceCount() const;

		AvatarIterator    getFirstAvatar() const;
		AvatarIterator    findAvatar(unsigned avatarID) const;
		AvatarIterator    findAvatar(const ChatUnicodeString &name, const ChatUnicodeString &address) const;

		ModeratorIterator getFirstModerator() const;
		ModeratorIterator findModerator(const ChatUnicodeString &name, const ChatUnicodeString &address) const;

		TemporaryModeratorIterator getFirstTemporaryModerator() const;
		TemporaryModeratorIterator findTemporaryModerator(const ChatUnicodeString &name, const ChatUnicodeString &address) const;

		BanIterator       getFirstBanned() const;
		BanIterator       findBanned(const ChatUnicodeString &name, const ChatUnicodeString &address) const;

		InviteIterator    getFirstInvited() const;
		InviteIterator    findInvited(const ChatUnicodeString &name, const ChatUnicodeString &address) const;

		VoiceIterator     getFirstVoice() const;
		VoiceIterator     findVoice(const ChatUnicodeString &name, const ChatUnicodeString &address) const;

		void setRoomMessageID(unsigned roomMessageID);

	private:
		ChatRoomCore *m_core;
		mutable ChatUnicodeString m_cCreatorAddress;
		mutable ChatUnicodeString m_cCreatorName;
		mutable ChatUnicodeString m_cRoomName;
		mutable ChatUnicodeString m_cRoomTopic;
		mutable ChatUnicodeString m_cRoomPassword;
		mutable ChatUnicodeString m_cRoomAddress;
	};

};

#endif

