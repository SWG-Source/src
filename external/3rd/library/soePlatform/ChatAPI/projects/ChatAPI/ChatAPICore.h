#if !defined (CHATAPICORE_H_)
#define CHATAPICORE_H_

#pragma warning (disable : 4786)
#include <map>
#include <set>
#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

#include <GenericAPI/GenericApiCore.h>

namespace ChatSystem 
{

	class ChatAvatar;
	class ChatAvatarCore;
	class ChatRoomCore;
	class ChatRoom;
	class ChatAPI;
	class ChatUnicodeString;

	class ChatAPICore : public GenericAPI::GenericAPICore
	{
	public:
		ChatAPICore(const char *registrar_host, short registrar_port, const char *server_host, short server_port);
		virtual ~ChatAPICore();

		void setRequestTimeout(unsigned requestTimeout) { m_requestTimeout = requestTimeout; }

		unsigned RequestCreateAvatar(const char *name);
		unsigned RequestDestroyAvatar(ChatAvatar *avatar);

		virtual void responseCallback(GenericAPI::GenericResponse *res);
		virtual void responseCallback(short type, Base::ByteStream::ReadIterator &iter);
		void clearRequestCount() { m_requestCount = 0; }

		virtual void OnConnect(const char *host, short port);
		virtual void OnDisconnect(const char *host, short port);

		void setAPI(ChatAPI *api) { m_api = api; }
		ChatAvatar   *getAvatar(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress);
		ChatAvatar   *getAvatar(unsigned avatarID);
		ChatRoom     *getRoom(const ChatUnicodeString &roomAddress);
		ChatRoom     *getRoom(unsigned roomID);

		static ChatUnicodeString getErrorString(unsigned resultCode);

		bool isUID(Plat_Unicode::String &uid) { return (m_uidSet.find(Plat_Unicode::toUpper(uid)) != m_uidSet.end()); }

		// calls GenericAPICore::processAPI() after doing any necessary work
		void processAPI();

	private:
		void cacheRoom(ChatRoomCore *roomCore);
		ChatRoomCore *decacheRoom(unsigned roomID);
		
		ChatRoomCore *getRoomCore(unsigned roomID);

		void cacheAvatar(ChatAvatarCore *avatarCore);
		ChatAvatarCore *decacheAvatar(unsigned avatarID);
		
		ChatAvatarCore *getAvatarCore(unsigned avatarID);

		bool areEqualChatStrings(const ChatUnicodeString &str1, const ChatUnicodeString &str2);

		void failoverReloginAvatars();
		void failoverReloginOneAvatar(ChatAvatarCore * avatarCore);

		void failoverRecreateRooms();

		std::map<unsigned, ChatAvatarCore *> m_avatarCoreCache;
		std::map<unsigned, ChatAvatar *> m_avatarCache;
		std::map<unsigned, ChatRoomCore *> m_roomCoreCache;
		std::map<unsigned, ChatRoom *> m_roomCache;

		std::set<Plat_Unicode::String> m_uidSet;

		bool m_connected;
		bool m_sentVersion;
		bool m_inFailoverMode;
		unsigned m_failoverAvatarResRemain;
		unsigned m_failoverRoomResRemain;
		unsigned m_requestCount;

		bool m_setToRegistrar;
		std::string m_registrarHost;
		std::string m_defaultServerHost;
		std::string m_assignedServerHost;
		short m_registrarPort;
		short m_defaultServerPort;
		short m_assignedServerPort;

		time_t m_timeSinceLastDisconnect;
		bool m_rcvdRegistrarResponse;

		static const char * const ms_errorStringsEnglish[];
		static unsigned ms_numErrorStrings;

		ChatAPI *m_api;
		
	protected:
		bool m_shouldSendVersion;
	};
};
#endif

