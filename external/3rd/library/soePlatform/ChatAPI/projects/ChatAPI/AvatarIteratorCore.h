#if !defined (AVATARITERATORCORE_H_)
#define AVATARITERATORCORE_H_

#pragma warning (disable : 4786)
#include <map>
#include <set>

namespace ChatSystem
{

	class ChatAvatar;

	class AvatarIteratorCore
	{
	public:
		AvatarIteratorCore();
		AvatarIteratorCore(std::map<unsigned, ChatAvatar *> *mapIn, std::map<unsigned, ChatAvatar *>::iterator iter);
		~AvatarIteratorCore();

		AvatarIteratorCore & operator=(const AvatarIteratorCore& rhs);

		ChatAvatar *getCurAvatar();

		// navigation functions return true while iterator points to a map node,
		//   else false if reached the end.
		bool increment();
		bool decrement();

		// have we reached the end?
		bool outOfBounds();

	private:
		std::map<unsigned, ChatAvatar *> *m_map;
		std::map<unsigned, ChatAvatar *>::iterator m_mapIter;
	};

	class ModeratorIteratorCore
	{
	public:
		ModeratorIteratorCore();
		ModeratorIteratorCore(std::set<ChatAvatar *> *setIn, std::set<ChatAvatar *>::iterator iter);
		~ModeratorIteratorCore();

		ModeratorIteratorCore & operator=(const ModeratorIteratorCore& rhs);

		ChatAvatar *getCurAvatar();

		// navigation functions return true while iterator points to a map node,
		//   else false if reached the end.
		bool increment();
		bool decrement();

		// have we reached the end?
		bool outOfBounds();

	private:
		std::set<ChatAvatar *> *m_set;
		std::set<ChatAvatar *>::iterator m_setIter;
	};

	class TemporaryModeratorIteratorCore
	{
	public:
		TemporaryModeratorIteratorCore();
		TemporaryModeratorIteratorCore(std::set<ChatAvatar *> *setIn, std::set<ChatAvatar *>::iterator iter);
		~TemporaryModeratorIteratorCore();

		TemporaryModeratorIteratorCore & operator=(const TemporaryModeratorIteratorCore& rhs);

		ChatAvatar *getCurAvatar();

		// navigation functions return true while iterator points to a map node,
		//   else false if reached the end.
		bool increment();
		bool decrement();

		// have we reached the end?
		bool outOfBounds();

	private:
		std::set<ChatAvatar *> *m_set;
		std::set<ChatAvatar *>::iterator m_setIter;
	};

	class VoiceIteratorCore
	{
	public:
		VoiceIteratorCore();
		VoiceIteratorCore(std::set<ChatAvatar *> *setIn, std::set<ChatAvatar *>::iterator iter);
		~VoiceIteratorCore();

		VoiceIteratorCore & operator=(const VoiceIteratorCore& rhs);

		ChatAvatar *getCurAvatar();

		// navigation functions return true while iterator points to a map node,
		//   else false if reached the end.
		bool increment();
		bool decrement();

		// have we reached the end?
		bool outOfBounds();

	private:
		std::set<ChatAvatar *> *m_set;
		std::set<ChatAvatar *>::iterator m_setIter;
	};

	class InviteIteratorCore
	{
	public:
		InviteIteratorCore();
		InviteIteratorCore(std::set<ChatAvatar *> *setIn, std::set<ChatAvatar *>::iterator iter);
		~InviteIteratorCore();

		InviteIteratorCore & operator=(const InviteIteratorCore& rhs);

		ChatAvatar *getCurAvatar();

		// navigation functions return true while iterator points to a map node,
		//   else false if reached the end.
		bool increment();
		bool decrement();

		// have we reached the end?
		bool outOfBounds();

	private:
		std::set<ChatAvatar *> *m_set;
		std::set<ChatAvatar *>::iterator m_setIter;
	};

	class BanIteratorCore
	{
	public:
		BanIteratorCore();
		BanIteratorCore(std::set<ChatAvatar *> *setIn, std::set<ChatAvatar *>::iterator iter);
		~BanIteratorCore();

		BanIteratorCore & operator=(const BanIteratorCore& rhs);

		ChatAvatar *getCurAvatar();

		// navigation functions return true while iterator points to a map node,
		//   else false if reached the end.
		bool increment();
		bool decrement();

		// have we reached the end?
		bool outOfBounds();

	private:
		std::set<ChatAvatar *> *m_set;
		std::set<ChatAvatar *>::iterator m_setIter;
	};

};
#endif

