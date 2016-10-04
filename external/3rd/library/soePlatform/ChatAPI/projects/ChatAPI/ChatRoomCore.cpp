#include "ChatRoomCore.h"
#include "ChatAvatar.h"
#include "ChatAvatarCore.h"
#include "AvatarIteratorCore.h"

namespace ChatSystem
{
	using namespace std;
	using namespace Base;
	using namespace Plat_Unicode;

	ChatRoomCore::ChatRoomCore()
		: m_creatorID(0),
		m_roomAttributes(0),
		m_maxRoomSize(0),
		m_roomID(0),
		m_createTime(0),
		m_nodeLevel(0),
		m_roomMessageID(0)
	{
	}

	ChatRoomCore::ChatRoomCore(ByteStream::ReadIterator &iter)
	{
		unsigned avatarCount;
		unsigned administratorCount;
		unsigned moderatorCount;
		unsigned tempModeratorCount;
		unsigned bannedCount;
		unsigned invitedCount;
		unsigned voiceCount;
		ChatAvatarCore *avatar;
		unsigned i;

		ASSERT_VALID_STRING_LENGTH(get(iter, m_creatorName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_creatorAddress));
		get(iter, m_creatorID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomTopic));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomPrefix));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomPassword));
		get(iter, m_roomAttributes);
		get(iter, m_maxRoomSize);
		get(iter, m_roomID);
		get(iter, m_createTime);
		get(iter, m_nodeLevel);

		get(iter, avatarCount);
		for (i = 0; i != avatarCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addAvatar(avatar, false);
		}

		get(iter, administratorCount);
		for (i = 0; i != administratorCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addAdministrator(avatar);
		}

		get(iter, moderatorCount);
		for (i = 0; i != moderatorCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addModerator(avatar);
		}

		get(iter, tempModeratorCount);
		for (i = 0; i != tempModeratorCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addTempModerator(avatar);
		}

		get(iter, bannedCount);
		for (i = 0; i != bannedCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addBan(avatar);
		}

		get(iter, invitedCount);
		for (i = 0; i != invitedCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addInvite(avatar);
		}

		get(iter, voiceCount);
		for (i = 0; i != voiceCount; i++)
		{
			avatar = new ChatAvatarCore(iter);
			this->addVoice(avatar);
		}
	}

	ChatRoomCore::ChatRoomCore(const String &name, const String &address, const String &topic, const unsigned attributes, const unsigned maxSize)
		: m_roomName(name),
		m_roomTopic(topic),
		m_roomAddress(address),
		m_creatorID(0),
		m_roomAttributes(attributes),
		m_maxRoomSize(maxSize),
		m_roomID(0),
		m_createTime(0),
		m_nodeLevel(0),
		m_roomMessageID(0)
	{
	}

	ChatRoomCore::~ChatRoomCore()
	{
		map<unsigned, ChatAvatarCore *>::iterator iterCore;
		map<unsigned, ChatAvatar *>::iterator iter;

		set<ChatAvatarCore *>::iterator setIterCore;
		set<ChatAvatar *>::iterator setIter;

		for (iterCore = m_inroomAvatarsCore.begin(); iterCore != m_inroomAvatarsCore.end(); ++iterCore)
		{
			delete (iterCore->second);
		}

		for (iter = m_inroomAvatars.begin(); iter != m_inroomAvatars.end(); ++iter)
		{
			delete (iter->second);
		}

		for (iterCore = m_adminAvatarsCore.begin(); iterCore != m_adminAvatarsCore.end(); ++iterCore)
		{
			delete (iterCore->second);
		}

		for (iter = m_adminAvatars.begin(); iter != m_adminAvatars.end(); ++iter)
		{
			delete (iter->second);
		}

		for (setIterCore = m_moderatorAvatarsCore.begin(); setIterCore != m_moderatorAvatarsCore.end(); ++setIterCore)
		{
			delete (*setIterCore);
		}

		for (setIter = m_moderatorAvatars.begin(); setIter != m_moderatorAvatars.end(); ++setIter)
		{
			delete (*setIter);
		}

		for (setIterCore = m_tempModeratorAvatarsCore.begin(); setIterCore != m_tempModeratorAvatarsCore.end(); ++setIterCore)
		{
			delete (*setIterCore);
		}

		for (setIter = m_tempModeratorAvatars.begin(); setIter != m_tempModeratorAvatars.end(); ++setIter)
		{
			delete (*setIter);
		}

		for (setIterCore = m_banAvatarsCore.begin(); setIterCore != m_banAvatarsCore.end(); ++setIterCore)
		{
			delete (*setIterCore);
		}

		for (setIter = m_banAvatars.begin(); setIter != m_banAvatars.end(); ++setIter)
		{
			delete (*setIter);
		}

		for (setIterCore = m_inviteAvatarsCore.begin(); setIterCore != m_inviteAvatarsCore.end(); ++setIterCore)
		{
			delete (*setIterCore);
		}

		for (setIter = m_inviteAvatars.begin(); setIter != m_inviteAvatars.end(); ++setIter)
		{
			delete (*setIter);
		}

		for (setIterCore = m_voiceAvatarsCore.begin(); setIterCore != m_voiceAvatarsCore.end(); ++setIterCore)
		{
			delete (*setIterCore);
		}

		for (setIter = m_voiceAvatars.begin(); setIter != m_voiceAvatars.end(); ++setIter)
		{
			delete (*setIter);
		}
	}

	ChatAvatarCore *ChatRoomCore::getAvatarCore(unsigned avatarID)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		map<unsigned, ChatAvatarCore *>::iterator iterCore = m_inroomAvatarsCore.find(avatarID);

		if (iterCore != m_inroomAvatarsCore.end())
		{
			returnAvatar = (*iterCore).second;
		}
		return(returnAvatar);
	}

	ChatAvatar *ChatRoomCore::getAvatar(unsigned avatarID)
	{
		ChatAvatar *returnAvatar = nullptr;

		map<unsigned, ChatAvatar *>::iterator iter = m_inroomAvatars.find(avatarID);

		if (iter != m_inroomAvatars.end())
		{
			returnAvatar = (*iter).second;
		}
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::getModeratorCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_moderatorAvatarsCore.begin();

		for (; iterCore != m_moderatorAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_moderatorAvatarsCore.end())
		{
			returnAvatar = (*iterCore);
		}
		return(returnAvatar);
	}

	ChatAvatar *ChatRoomCore::getModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatar *returnAvatar = nullptr;

		set<ChatAvatar *>::iterator iter = m_moderatorAvatars.begin();

		for (; iter != m_moderatorAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		if (iter != m_moderatorAvatars.end())
		{
			returnAvatar = (*iter);
		}
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::getBannedCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_banAvatarsCore.begin();

		for (; iterCore != m_banAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_banAvatarsCore.end())
		{
			returnAvatar = (*iterCore);
		}
		return(returnAvatar);
	}

	ChatAvatar *ChatRoomCore::getBanned(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatar *returnAvatar = nullptr;

		set<ChatAvatar *>::iterator iter = m_banAvatars.begin();

		for (; iter != m_banAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		if (iter != m_banAvatars.end())
		{
			returnAvatar = (*iter);
		}
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::getInvitedCore(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_inviteAvatarsCore.begin();

		for (; iterCore != m_inviteAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_inviteAvatarsCore.end())
		{
			returnAvatar = (*iterCore);
		}
		return(returnAvatar);
	}

	ChatAvatar *ChatRoomCore::getInvited(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatar *returnAvatar = nullptr;

		set<ChatAvatar *>::iterator iter = m_inviteAvatars.begin();

		for (; iter != m_inviteAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		if (iter != m_inviteAvatars.end())
		{
			returnAvatar = (*iter);
		}
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addAvatar(ChatAvatarCore *newAvatar, bool local)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< map<unsigned, ChatAvatarCore*>::iterator, bool > result;

			result = m_inroomAvatarsCore.insert(pair<unsigned, ChatAvatarCore *>(newAvatar->getAvatarID(), newAvatar));

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_inroomAvatars.insert(pair<unsigned, ChatAvatar *>(avatar->getAvatarID(), avatar));

				// keep track of avatars connected to this API that are in room
				if (local)
				{
					m_inroomAvatarsLocal.insert(newAvatar->getAvatarID());
				}

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	void ChatRoomCore::addLocalAvatar(unsigned avatarID)
	{
		m_inroomAvatarsLocal.insert(avatarID);
	}

	ChatAvatarCore *ChatRoomCore::removeAvatar(unsigned avatarID)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		map<unsigned, ChatAvatarCore *>::iterator iterCore = m_inroomAvatarsCore.find(avatarID);

		if (iterCore != m_inroomAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore).second;

			m_inroomAvatarsCore.erase(iterCore);

			// because the addAvatar() made the ChatAvatar, removeAvatar() deletes it.
			map<unsigned, ChatAvatar *>::iterator iter = m_inroomAvatars.find(avatarID);
			delete (*iter).second;
			m_inroomAvatars.erase(avatarID);

			// remove from cache of local API avatars logged into this room, if nec.
			std::set<unsigned>::iterator localIter = m_inroomAvatarsLocal.find(avatarID);
			if (localIter != m_inroomAvatarsLocal.end())
				m_inroomAvatarsLocal.erase(localIter);
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addBan(ChatAvatarCore *newAvatar)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< set<ChatAvatarCore*>::iterator, bool > result;

			result = m_banAvatarsCore.insert(newAvatar);

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_banAvatars.insert(avatar);

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	ChatAvatarCore *ChatRoomCore::removeBan(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_banAvatarsCore.begin();

		for (; iterCore != m_banAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_banAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore);
			m_banAvatarsCore.erase(iterCore);

			// because the addBan() made the ChatAvatar, removeBan() deletes it.
			set<ChatAvatar *>::iterator iter = m_banAvatars.begin();

			for (; iter != m_banAvatars.end(); ++iter)
			{
				ChatAvatar *avatar = (*iter);
				Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
				Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
				if (caseInsensitiveCompare(name, avatarName) &&
					caseInsensitiveCompare(address, avatarAddress, true))
				{
					break;
				}
			}

			if (iter != m_banAvatars.end())
			{
				delete (*iter);
				m_banAvatars.erase(iter);
			}
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addAdministrator(ChatAvatarCore *newAvatar)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< map<unsigned, ChatAvatarCore*>::iterator, bool > result;

			result = this->m_adminAvatarsCore.insert(pair<unsigned, ChatAvatarCore *>(newAvatar->getAvatarID(), newAvatar));

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_adminAvatars.insert(pair<unsigned, ChatAvatar *>(avatar->getAvatarID(), avatar));

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	ChatAvatarCore *ChatRoomCore::removeAdministrator(unsigned avatarID)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		map<unsigned, ChatAvatarCore *>::iterator iterCore = m_adminAvatarsCore.find(avatarID);

		if (iterCore != m_adminAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore).second;

			m_adminAvatarsCore.erase(iterCore);

			// because the addAvatar() made the ChatAvatar, removeAvatar() deletes it.
			map<unsigned, ChatAvatar *>::iterator iter = m_adminAvatars.find(avatarID);
			delete (*iter).second;
			m_adminAvatars.erase(avatarID);
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addModerator(ChatAvatarCore *newAvatar)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< set<ChatAvatarCore*>::iterator, bool > result;

			result = this->m_moderatorAvatarsCore.insert(newAvatar);

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_moderatorAvatars.insert(avatar);

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	ChatAvatarCore *ChatRoomCore::removeModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_moderatorAvatarsCore.begin();

		for (; iterCore != m_moderatorAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_moderatorAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore);
			m_moderatorAvatarsCore.erase(iterCore);

			// because the addBan() made the ChatAvatar, removeBan() deletes it.
			set<ChatAvatar *>::iterator iter = m_moderatorAvatars.begin();

			for (; iter != m_moderatorAvatars.end(); ++iter)
			{
				ChatAvatar *avatar = (*iter);
				Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
				Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
				if (caseInsensitiveCompare(name, avatarName) &&
					caseInsensitiveCompare(address, avatarAddress, true))
				{
					break;
				}
			}

			if (iter != m_moderatorAvatars.end())
			{
				delete (*iter);
				m_moderatorAvatars.erase(iter);
			}
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addTempModerator(ChatAvatarCore *newAvatar)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< set<ChatAvatarCore*>::iterator, bool > result;

			result = this->m_tempModeratorAvatarsCore.insert(newAvatar);

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_tempModeratorAvatars.insert(avatar);

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	ChatAvatarCore *ChatRoomCore::removeTempModerator(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_tempModeratorAvatarsCore.begin();

		for (; iterCore != m_tempModeratorAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_tempModeratorAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore);
			m_tempModeratorAvatarsCore.erase(iterCore);

			// because the addBan() made the ChatAvatar, removeBan() deletes it.
			set<ChatAvatar *>::iterator iter = m_tempModeratorAvatars.begin();

			for (; iter != m_tempModeratorAvatars.end(); ++iter)
			{
				ChatAvatar *avatar = (*iter);
				Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
				Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
				if (caseInsensitiveCompare(name, avatarName) &&
					caseInsensitiveCompare(address, avatarAddress, true))
				{
					break;
				}
			}

			if (iter != m_tempModeratorAvatars.end())
			{
				delete (*iter);
				m_tempModeratorAvatars.erase(iter);
			}
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addInvite(ChatAvatarCore* newAvatar)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< set<ChatAvatarCore*>::iterator, bool > result;

			result = m_inviteAvatarsCore.insert(newAvatar);

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_inviteAvatars.insert(avatar);

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	ChatAvatarCore *ChatRoomCore::removeInvite(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_inviteAvatarsCore.begin();

		for (; iterCore != m_inviteAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_inviteAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore);
			m_inviteAvatarsCore.erase(iterCore);

			// because the addBan() made the ChatAvatar, removeBan() deletes it.
			set<ChatAvatar *>::iterator iter = m_inviteAvatars.begin();

			for (; iter != m_inviteAvatars.end(); ++iter)
			{
				ChatAvatar *avatar = (*iter);
				Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
				Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
				if (caseInsensitiveCompare(name, avatarName) &&
					caseInsensitiveCompare(address, avatarAddress, true))
				{
					break;
				}
			}

			if (iter != m_inviteAvatars.end())
			{
				delete (*iter);
				m_inviteAvatars.erase(iter);
			}
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	ChatAvatarCore *ChatRoomCore::addVoice(ChatAvatarCore* newAvatar)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		if (newAvatar)
		{
			pair< set<ChatAvatarCore*>::iterator, bool > result;

			result = m_voiceAvatarsCore.insert(newAvatar);

			// if add of ChatAvatarCore works, add a new ChatAvatar also.
			if (result.second == true)
			{
				ChatAvatar *avatar = newAvatar->getNewChatAvatar();

				m_voiceAvatars.insert(avatar);

				returnAvatar = newAvatar;
			}
		}

		return returnAvatar;
	}

	ChatAvatarCore *ChatRoomCore::removeVoice(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		set<ChatAvatarCore *>::iterator iterCore = m_voiceAvatarsCore.begin();

		for (; iterCore != m_voiceAvatarsCore.end(); ++iterCore)
		{
			ChatAvatarCore *avatarCore = (*iterCore);
			if (caseInsensitiveCompare(name, avatarCore->getName()) &&
				caseInsensitiveCompare(address, avatarCore->getAddress(), true))
			{
				break;
			}
		}

		if (iterCore != m_voiceAvatarsCore.end())
		{
			// remove ChatAvatarCore and ChatAvatar objects.
			returnAvatar = (*iterCore);
			m_voiceAvatarsCore.erase(iterCore);

			// because the addBan() made the ChatAvatar, removeBan() deletes it.
			set<ChatAvatar *>::iterator iter = m_voiceAvatars.begin();

			for (; iter != m_voiceAvatars.end(); ++iter)
			{
				ChatAvatar *avatar = (*iter);
				Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
				Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
				if (caseInsensitiveCompare(name, avatarName) &&
					caseInsensitiveCompare(address, avatarAddress, true))
				{
					break;
				}
			}

			if (iter != m_voiceAvatars.end())
			{
				delete (*iter);
				m_voiceAvatars.erase(iter);
			}
		}

		// pass back the ChatAvatarCore and let caller delete it (b/c he created it).
		return(returnAvatar);
	}

	// ------- ChatRoom class support functions

	unsigned ChatRoomCore::getAvatarCount() const
	{
		return m_inroomAvatarsCore.size();
	}

	unsigned ChatRoomCore::getBanCount() const
	{
		return m_banAvatarsCore.size();
	}

	unsigned ChatRoomCore::getInviteCount() const
	{
		return m_inviteAvatarsCore.size();
	}

	unsigned ChatRoomCore::getModeratorCount() const
	{
		return m_moderatorAvatarsCore.size();
	}

	unsigned ChatRoomCore::getTemporaryModeratorCount() const
	{
		return m_tempModeratorAvatarsCore.size();
	}

	unsigned ChatRoomCore::getVoiceCount() const
	{
		return m_voiceAvatarsCore.size();
	}

	AvatarIteratorCore ChatRoomCore::getFirstAvatar()
	{
		return (AvatarIteratorCore(&m_inroomAvatars, m_inroomAvatars.begin()));
	}

	AvatarIteratorCore ChatRoomCore::findAvatar(unsigned avatarID)
	{
		return (AvatarIteratorCore(&m_inroomAvatars, m_inroomAvatars.find(avatarID)));
	}

	AvatarIteratorCore ChatRoomCore::findAvatar(const String &name, const String &address)
	{
		map<unsigned, ChatAvatar *>::iterator iter;

		for (iter = m_inroomAvatars.begin(); iter != m_inroomAvatars.end(); ++iter)
		{
			if (String((*iter).second->getName().string_data, (*iter).second->getName().string_length) == name &&
				String((*iter).second->getAddress().string_data, (*iter).second->getAddress().string_length) == address)
			{
				break;
			}
		}

		return (AvatarIteratorCore(&m_inroomAvatars, iter));
	}

	ModeratorIteratorCore ChatRoomCore::getFirstModerator()
	{
		return (ModeratorIteratorCore(&m_moderatorAvatars, m_moderatorAvatars.begin()));
	}

	ModeratorIteratorCore ChatRoomCore::findModerator(const String &name, const String &address)
	{
		set<ChatAvatar *>::iterator iter;

		for (iter = m_moderatorAvatars.begin(); iter != m_moderatorAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		return (ModeratorIteratorCore(&m_moderatorAvatars, iter));
	}

	TemporaryModeratorIteratorCore ChatRoomCore::getFirstTemporaryModerator()
	{
		return (TemporaryModeratorIteratorCore(&m_tempModeratorAvatars, m_tempModeratorAvatars.begin()));
	}

	TemporaryModeratorIteratorCore ChatRoomCore::findTemporaryModerator(const String &name, const String &address)
	{
		set<ChatAvatar *>::iterator iter;

		for (iter = m_tempModeratorAvatars.begin(); iter != m_tempModeratorAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		return (TemporaryModeratorIteratorCore(&m_tempModeratorAvatars, iter));
	}

	BanIteratorCore ChatRoomCore::getFirstBanned()
	{
		return (BanIteratorCore(&m_banAvatars, m_banAvatars.begin()));
	}

	BanIteratorCore ChatRoomCore::findBanned(const String &name, const String &address)
	{
		set<ChatAvatar *>::iterator iter;

		for (iter = m_banAvatars.begin(); iter != m_banAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		return (BanIteratorCore(&m_banAvatars, iter));
	}

	InviteIteratorCore ChatRoomCore::getFirstInvited()
	{
		return (InviteIteratorCore(&m_inviteAvatars, m_inviteAvatars.begin()));
	}

	InviteIteratorCore ChatRoomCore::findInvited(const String &name, const String &address)
	{
		set<ChatAvatar *>::iterator iter;

		for (iter = m_inviteAvatars.begin(); iter != m_inviteAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		return (InviteIteratorCore(&m_inviteAvatars, iter));
	}

	VoiceIteratorCore ChatRoomCore::getFirstVoice()
	{
		return (VoiceIteratorCore(&m_voiceAvatars, m_voiceAvatars.begin()));
	}

	VoiceIteratorCore ChatRoomCore::findVoice(const String &name, const String &address)
	{
		set<ChatAvatar *>::iterator iter;

		for (iter = m_voiceAvatars.begin(); iter != m_voiceAvatars.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter);
			Plat_Unicode::String avatarName(avatar->getName().string_data, avatar->getName().string_length);
			Plat_Unicode::String avatarAddress(avatar->getAddress().string_data, avatar->getAddress().string_length);
			if (caseInsensitiveCompare(name, avatarName) &&
				caseInsensitiveCompare(address, avatarAddress, true))
			{
				break;
			}
		}

		return (VoiceIteratorCore(&m_voiceAvatars, iter));
	}

	void ChatRoomCore::serializeWithLocalAvatarsOnly(Base::ByteStream &msg)
	{
		// put creator name
		put(msg, m_creatorName);

		// put creator address
		put(msg, m_creatorAddress);

		// put creator ID
		put(msg, m_creatorID);

		// put room name
		put(msg, m_roomName);

		// put room topic
		put(msg, m_roomTopic);

		// put room password
		put(msg, m_roomPassword);

		// put room prefix
		put(msg, m_roomPrefix);

		// put room address
		put(msg, m_roomAddress);

		// put room attributes
		put(msg, m_roomAttributes);

		// put room max size
		put(msg, m_maxRoomSize);

		// put room ID
		put(msg, m_roomID);

		// put room creation time
		put(msg, m_createTime);

		// put in-room avatars
		unsigned avatarCount = m_inroomAvatarsCore.size();

		std::map<unsigned, ChatAvatarCore *>::iterator avatarIter;
		std::set<ChatAvatarCore *> avatarsToSend;

		for (avatarIter = m_inroomAvatarsCore.begin(); avatarIter != m_inroomAvatarsCore.end(); ++avatarIter)
		{
			// include only the avatars that are on this API
			if (this->getAvatar((*avatarIter).second->getAvatarID()) != nullptr)
			{
				avatarsToSend.insert((*avatarIter).second);
			}
		}

		// actually send avatars from this API *after* they have been counted
		avatarCount = avatarsToSend.size();
		put(msg, avatarCount);

		std::set<ChatAvatarCore *>::iterator sendIter;
		for (sendIter = avatarsToSend.begin(); sendIter != avatarsToSend.end(); ++sendIter)
		{
			(*sendIter)->serialize(msg);
		}

		// put administrator avatars
		unsigned administratorCount = m_adminAvatarsCore.size();
		put(msg, administratorCount);

		std::map<unsigned, ChatAvatarCore *>::iterator adminIter;
		for (adminIter = m_adminAvatarsCore.begin(); adminIter != m_adminAvatarsCore.end(); ++adminIter)
		{
			(*adminIter).second->serialize(msg);
		}

		// put moderator avatars
		unsigned moderatorCount = m_moderatorAvatarsCore.size();
		put(msg, moderatorCount);

		std::set<ChatAvatarCore *>::iterator moderatorIter;
		for (moderatorIter = m_moderatorAvatarsCore.begin(); moderatorIter != m_moderatorAvatarsCore.end(); ++moderatorIter)
		{
			(*moderatorIter)->serialize(msg);
		}

		// put temporary moderator avatars
		unsigned tempModeratorCount = m_tempModeratorAvatarsCore.size();
		put(msg, tempModeratorCount);

		std::set<ChatAvatarCore *>::iterator tempModeratorIter;
		for (tempModeratorIter = m_tempModeratorAvatarsCore.begin(); tempModeratorIter != m_tempModeratorAvatarsCore.end(); ++tempModeratorIter)
		{
			(*tempModeratorIter)->serialize(msg);
		}

		// put banned avatars
		unsigned bannedCount = m_banAvatarsCore.size();
		put(msg, bannedCount);

		std::set<ChatAvatarCore *>::iterator bannedIter;
		for (bannedIter = m_banAvatarsCore.begin(); bannedIter != m_banAvatarsCore.end(); ++bannedIter)
		{
			(*bannedIter)->serialize(msg);
		}

		// put invited avatars
		unsigned invitedCount = m_inviteAvatarsCore.size();
		put(msg, invitedCount);

		std::set<ChatAvatarCore *>::iterator invitesIter;
		for (invitesIter = m_inviteAvatarsCore.begin(); invitesIter != m_inviteAvatarsCore.end(); ++invitesIter)
		{
			(*invitesIter)->serialize(msg);
		}

		// put voice avatars
		unsigned voiceCount = m_voiceAvatarsCore.size();
		put(msg, voiceCount);

		std::set<ChatAvatarCore *>::iterator voicesIter;
		for (voicesIter = m_voiceAvatarsCore.begin(); voicesIter != m_voiceAvatarsCore.end(); ++voicesIter)
		{
			(*voicesIter)->serialize(msg);
		}

		put(msg, m_roomMessageID);
	}

	void ChatRoomCore::setCreator(unsigned creatorID, const Plat_Unicode::String &creatorName, const Plat_Unicode::String &creatorAddress)
	{
		m_creatorID = creatorID;
		m_creatorName = creatorName;
		m_creatorAddress = creatorAddress;
	}
};