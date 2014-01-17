#if !defined (CHATROOMCORE_H_)
#define CHATROOMCORE_H_

#pragma warning (disable : 4786)

#include <set>
#include <map>

#include <Base/Archive.h>
#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

#include "ChatRoom.h"

namespace ChatSystem 
{
	class ChatAvatar;
	class ChatAvatarCore;
	class AvatarIteratorCore;
	class ModeratorIteratorCore;
	class TemporaryModeratorIteratorCore;
	class VoiceIteratorCore;
	class InviteIteratorCore;
	class BanIteratorCore;
	class RoomParams;


	class ChatRoomCore
	{
	public:
		ChatRoomCore();
		ChatRoomCore(Base::ByteStream::ReadIterator &iter);
		ChatRoomCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address, const Plat_Unicode::String &topic, const unsigned attributes, const unsigned maxSize);

		~ChatRoomCore();

		const Plat_Unicode::String &getCreatorName() const { return m_creatorName; }
		const Plat_Unicode::String &getCreatorAddress() const { return m_creatorAddress; }
		unsigned getCreatorID() const { return m_creatorID; }
		
		unsigned getRoomID() const { return m_roomID; }
		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomTopic() const { return m_roomTopic; }
		const Plat_Unicode::String &getRoomPassword() const { return m_roomPassword; }
		unsigned getRoomAttributes() const { return m_roomAttributes; }
		unsigned getMaxRoomSize() const { return m_maxRoomSize; }
		const Plat_Unicode::String &getAddress() const { return m_roomAddress; }
		unsigned getNodeLevel() const { return m_nodeLevel; }

		unsigned getAvatarCount() const;
		unsigned getBanCount() const;
		unsigned getInviteCount() const;
		unsigned getModeratorCount() const;
		unsigned getTemporaryModeratorCount() const;
		unsigned getVoiceCount() const;


		void setName(const Plat_Unicode::String &name) { m_roomName = name; }
		void setTopic(const Plat_Unicode::String &topic) { m_roomTopic = topic; }
		void setPassword(const Plat_Unicode::String &password) { m_roomPassword = password; }
		void setAttributes(unsigned attributes) { m_roomAttributes = attributes; }
		void setMaxRoomSize(unsigned size) { m_maxRoomSize = size; }
		void setID(unsigned id) { m_roomID = id; }
		void setCreator(unsigned creatorID, const Plat_Unicode::String &creatorName, const Plat_Unicode::String &creatorAddress);

		void setRoomMessageID(unsigned roomMessageID) { m_roomMessageID = roomMessageID; }
		
		ChatAvatarCore *getAvatarCore(unsigned avatarID);
		ChatAvatar *getAvatar(unsigned avatarID);
		ChatAvatarCore *getModeratorCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatar *getModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatarCore *getTemporaryModeratorCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatar *getTemporaryModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatarCore *getBannedCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatar *getBanned(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatarCore *getInvitedCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatar *getInvited(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatarCore *getVoiceCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		ChatAvatar *getVoice(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		ChatAvatarCore *addAvatar(ChatAvatarCore *newAvatar, bool local);
		void addLocalAvatar(unsigned avatarID);
		ChatAvatarCore *removeAvatar(unsigned avatarID);

		ChatAvatarCore *addAdministrator(ChatAvatarCore *newAvatar);
		ChatAvatarCore *removeAdministrator(unsigned avatarID);

		ChatAvatarCore *addModerator(ChatAvatarCore *newAvatar);
		ChatAvatarCore *removeModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		ChatAvatarCore *addTempModerator(ChatAvatarCore *newAvatar);
		ChatAvatarCore *removeTempModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		ChatAvatarCore *addBan(ChatAvatarCore *newAvatar);
		ChatAvatarCore *removeBan(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		ChatAvatarCore *addInvite(ChatAvatarCore *newAvatar);
		ChatAvatarCore *removeInvite(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		ChatAvatarCore *addVoice(ChatAvatarCore *newAvatar);
		ChatAvatarCore *removeVoice(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		AvatarIteratorCore getFirstAvatar();
		AvatarIteratorCore findAvatar(unsigned avatarID);
		AvatarIteratorCore findAvatar(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		ModeratorIteratorCore getFirstModerator();
		ModeratorIteratorCore findModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		TemporaryModeratorIteratorCore getFirstTemporaryModerator();
		TemporaryModeratorIteratorCore findTemporaryModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		BanIteratorCore getFirstBanned();
		BanIteratorCore findBanned(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		InviteIteratorCore getFirstInvited();
		InviteIteratorCore findInvited(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		VoiceIteratorCore getFirstVoice();
		VoiceIteratorCore findVoice(const Plat_Unicode::String &name, const Plat_Unicode::String &address);

		void serializeWithLocalAvatarsOnly(Base::ByteStream &msg);

	private:
		Plat_Unicode::String m_creatorName;
		Plat_Unicode::String m_creatorAddress;
		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomTopic;
		Plat_Unicode::String m_roomPassword;
		Plat_Unicode::String m_roomPrefix;
		Plat_Unicode::String m_roomAddress;
		
		unsigned m_creatorID;
		unsigned m_roomAttributes;
		unsigned m_maxRoomSize;
		unsigned m_roomID;
		unsigned m_createTime;
		unsigned m_nodeLevel;
		unsigned m_roomMessageID;

		std::map<unsigned, ChatAvatarCore *> m_inroomAvatarsCore;
		std::map<unsigned, ChatAvatar *> m_inroomAvatars;
		std::set<unsigned> m_inroomAvatarsLocal;
		std::map<unsigned, ChatAvatarCore *> m_adminAvatarsCore;
		std::map<unsigned, ChatAvatar *> m_adminAvatars;
		std::set<ChatAvatarCore *> m_moderatorAvatarsCore;
		std::set<ChatAvatarCore *> m_tempModeratorAvatarsCore;
		std::set<ChatAvatar *> m_moderatorAvatars;
		std::set<ChatAvatar *> m_tempModeratorAvatars;
		std::set<ChatAvatarCore *> m_banAvatarsCore;
		std::set<ChatAvatar *> m_banAvatars;
		std::set<ChatAvatarCore *> m_inviteAvatarsCore;
		std::set<ChatAvatar *> m_inviteAvatars;
		std::set<ChatAvatarCore *> m_voiceAvatarsCore;
		std::set<ChatAvatar *> m_voiceAvatars;
	};
};
#endif

