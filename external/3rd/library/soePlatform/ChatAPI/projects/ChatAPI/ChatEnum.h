#if !defined (CHATENUM_H_)
#define CHATENUM_H_

#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

#pragma warning ( disable : 4786 )

inline static void _chatdebug_(const char *fmt, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
#endif
}

// Use AVATAR_PRIORITY_NOFORCELOGOUT on RequestAvatarLogin if you want to override the
// "last login wins" rule, switching that given avatar's session to use "first login wins"
#define AVATAR_PRIORITY_NOFORCELOGOUT	((int) 0x80000000) /*-2147483648*/

#define ASSERT_VALID_STRING_LENGTH(X) if (X ==0) return;

namespace ChatSystem 
{

	enum eResultCodes
	{
		CHATRESULT_SUCCESS,

		CHATRESULT_TIMEOUT = 1,
		CHATRESULT_DUPLICATELOGIN,
		CHATRESULT_SRCAVATARDOESNTEXIST,
		CHATRESULT_DESTAVATARDOESNTEXIST,
		CHATRESULT_ADDRESSDOESNTEXIST,		// 5
		CHATRESULT_ADDRESSNOTROOM,
		CHATRESULT_ADDRESSNOTAID,
		CHATRESULT_FRIENDNOTFOUND,
		CHATRESULT_ROOM_UNKNOWNFAILURE,
		CHATRESULT_ROOM_SRCNOTINROOM,		// 10
		CHATRESULT_ROOM_DESTNOTINROOM,
		CHATRESULT_ROOM_BANNEDAVATAR,
		CHATRESULT_ROOM_PRIVATEROOM,
		CHATRESULT_ROOM_MODERATEDROOM,
		CHATRESULT_ROOM_NOTINROOM,			// 15
		CHATRESULT_ROOM_NOPRIVILEGES,
		CHATRESULT_DATABASE,
		CHATRESULT_CANNOTGETAVATARID,
		CHATRESULT_CANNOTGETNODEID,
		CHATRESULT_CANNOTGETPMSGID,			// 20
		CHATRESULT_PMSGNOTFOUND,
		CHATRESULT_ROOMMAXAVATARSREACHED,
		CHATRESULT_IGNORING,
		CHATRESULT_ROOM_ALREADYEXISTS,
		CHATRESULT_NOTHINGTOCONFIRM,		// 25
		CHATRESULT_DUPLICATEFRIEND,
		CHATRESULT_IGNORENOTFOUND,
		CHATRESULT_DUPLICATEIGNORE,
		CHATRESULT_DBFAIL,
		CHATRESULT_ROOM_DESTAVATARNOTMODERATOR,	// 30
		CHATRESULT_ROOM_DESTAVATARNOTINVITED,
		CHATRESULT_ROOM_DESTAVATARNOTBANNED,
		CHATRESULT_ROOM_DUPLICATEBAN,
		CHATRESULT_ROOM_DUPLICATEMODERATOR,
		CHATRESULT_ROOM_DUPLICATEINVITE,	// 35
		CHATRESULT_ROOM_ALREADYINROOM,
		CHATRESULT_ROOM_PARENTNONPERSISTENT,
		CHATRESULT_ROOM_PARENTBADNODETYPE,
		CHATRESULT_NOFANCLUBHANDLE,
		CHATRESULT_AIDALREADYEXISTS,		// 40
		CHATRESULT_UIDALREADYEXISTS,
		CHATRESULT_WRONGCHATSERVERFORREQUEST,
		CHATRESULT_SUCCESSBADDATA,
		CHATRESULT_NULLNAMELOGIN,
		CHATRESULT_SERVER_IDENTITY_EMPTY,	// 45
		CHATRESULT_SERVER_IDENTITY_TAKEN,
		CHATRESULT_REMOTESERVERDOWN,
		CHATRESULT_NODEIDCONFLICT,
		CHATRESULT_INVALIDNODENAME,
		CHATRESULT_INSUFFICIENTGMPRIVS,		// 50
		CHATRESULT_SNOOPALREADYADDED,
		CHATRESULT_NOTSNOOPING,
		CHATRESULT_ROOM_DESTAVATARNOTTEMPORARYMODERATOR,
		CHATRESULT_ROOM_DESTAVATARNOTVOICE,
		CHATRESULT_ROOM_DUPLICATETEMPORARYMODERATOR, // 55
		CHATRESULT_ROOM_DUPLICATEVOICE,
		CHATRESULT_AVATARMUSTBELOGGEDOUT,
		CHATRESULT_NOTHINGTODO,
		CHATRESULT_TRANSFERNAMENULL,
		CHATRESULT_TRANSFERUSERIDZERO,				// 60
		CHATRESULT_TRANSFERADDRESSNULL,
		CHATRESULT_OUTOFIDS,
		CHATRESULT_ROOM_LOCALROOM,
		CHATRESULT_ROOM_GAMEROOM,
		CHATRESULT_ROOM_DESTAVATARNOTENTERING,		// 65
		CHATRESULT_INSUFFICIENTPRIORITY,
		CHATRESULT_ROOM_WAITINGFORENTRY,
		CHATRESULT_INBOXLIMITEXCEEDED,
		CHATRESULT_DUPLICATEDESTINATION,
		CHATRESULT_CATEGORYLIMITEXCEEDED,			// 70
		CHATRESULT_MESSAGE_FILTER_FAILURE,
		CHATRESULT_INVALID_INPUT,

	};

	// Status for PersistentHeader objects as found in PersistentMessage.h
	enum PersistentStatus
	{
		PERSISTENT_NEW = 1,
		PERSISTENT_UNREAD,
		PERSISTENT_READ,
		PERSISTENT_TRASH,
		PERSISTENT_DELETED
	};

	// Bit flags for persistent message alterations. These are used in RequestAlterPersistentMessage
	enum PersistentAlterationTypes
	{
		ALTER_SENDER		  = 1,
		ALTER_SUBJECT         = 2,
		ALTER_MSG             = 4,
		ALTER_OOB             = 8,
		ALTER_SENT_TIME       = 16,
		ALTER_CATEGORY        = 32
	};

	// Types of Snoop messages for OnReceiveSnoopMessage() callback in ChatAPI.h
	enum SnoopTypes
	{
		SNOOP_INSTANTMESSAGE,
		SNOOP_ROOMMESSAGE,
		SNOOP_PERSISTENTMESSAGE,
		SNOOP_END
	};

#define CHATUNICODESTRING_COMP_OPER_DECL(__oper__)							\
		inline bool operator __oper__(const ChatUnicodeString &src) const	\
		{																	\
			return (m_wideString __oper__ src.m_wideString);				\
		}

#define string_data		data()
#define string_length	length()

	class ChatUnicodeString {

	private:
		Plat_Unicode::String m_wideString;
		Plat_Unicode::String getEmptyString();

		std::string m_cString;

		// const unsigned short *string_data;
		// unsigned string_length;

	public:

		ChatUnicodeString();
		ChatUnicodeString(const unsigned short *data, unsigned length);
		ChatUnicodeString(const char *nData, unsigned length);
		ChatUnicodeString(const Plat_Unicode::String& src);
		ChatUnicodeString(const std::string& nSrc);
		ChatUnicodeString(const char *nStrSrc);
		ChatUnicodeString(const ChatUnicodeString& src);

		ChatUnicodeString& operator=(const std::string& nSrc);
		ChatUnicodeString& operator=(const Plat_Unicode::String& src);
		ChatUnicodeString& operator=(const char *nStrSrc);
		ChatUnicodeString& operator=(const ChatUnicodeString& src);

		ChatUnicodeString& operator+=(const ChatUnicodeString& src);
		ChatUnicodeString operator+(const ChatUnicodeString& src);
		CHATUNICODESTRING_COMP_OPER_DECL(<)
		CHATUNICODESTRING_COMP_OPER_DECL(<=)
		CHATUNICODESTRING_COMP_OPER_DECL(==)
		CHATUNICODESTRING_COMP_OPER_DECL(!=)
		CHATUNICODESTRING_COMP_OPER_DECL(>=)
		CHATUNICODESTRING_COMP_OPER_DECL(>)
		inline unsigned length() const { return (unsigned)m_wideString.length(); }
		inline const unsigned short * data() const { return m_wideString.data(); }
		const char    * c_str() const;
		
	};
	

	// class for OnGetSnoopList() callback to summarize a name/address pair
	class AvatarSnoopPair
	{
	public:
		AvatarSnoopPair(const Plat_Unicode::String &name, const Plat_Unicode::String &address);
		~AvatarSnoopPair() {}

		const ChatUnicodeString &getAvatarName() const { return m_name; }
		const ChatUnicodeString &getAvatarAddress() const { return m_address; }

	private:
		ChatUnicodeString m_name;
		ChatUnicodeString m_address;
	};

};

#endif

