// VoiceChatMiscMessages.h
// temporary file to hold various messages that may or may not exist for long
// Steven Wyckoff

#ifndef _INCLUDED_VoiceChatMiscMessages_H
#define _INCLUDED_VoiceChatMiscMessages_H

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

/////////////////////////////////////////////////////////////////////////
// VoiceChatAddClientToRoom
// TODO: move this to its own file
/////////////////////////////////////////////////////////////////////////

class VoiceChatAddClientToChannel : public GameNetworkMessage
{
public:

	static const char* const cms_name;

	explicit VoiceChatAddClientToChannel(NetworkId const & clientId, std::string const & clientName, std::string const & roomName, bool forceShortlist);
	explicit VoiceChatAddClientToChannel(Archive::ReadIterator &);
	~VoiceChatAddClientToChannel();

	inline NetworkId const & getClientId() const {return m_clientId.get();}
	inline std::string const & getClientName() const {return m_clientName.get();}
	inline std::string const & getChannelName() const {return m_channelName.get();}
	inline bool getForceShortlist() const {return m_forceShortlist.get();}

private:
	VoiceChatAddClientToChannel & operator = (const VoiceChatAddClientToChannel & rhs);
	VoiceChatAddClientToChannel(const VoiceChatAddClientToChannel & source);

	Archive::AutoVariable<NetworkId> m_clientId;
	Archive::AutoVariable<std::string> m_clientName;
	Archive::AutoVariable<std::string> m_channelName;
	Archive::AutoVariable<bool> m_forceShortlist;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatRemoveClientFromChannel : public GameNetworkMessage
{
public:

	static const char* const cms_name;

	explicit VoiceChatRemoveClientFromChannel(NetworkId const & clientId, std::string const & clientName, std::string const & roomName);
	explicit VoiceChatRemoveClientFromChannel(Archive::ReadIterator &);
	~VoiceChatRemoveClientFromChannel();

	inline NetworkId const & getClientId() const {return m_clientId.get();}
	inline std::string const & getClientName() const {return m_clientName.get();}
	inline std::string const & getChannelName() const {return m_channelName.get();}

private:
	VoiceChatRemoveClientFromChannel & operator = (const VoiceChatRemoveClientFromChannel & rhs);
	VoiceChatRemoveClientFromChannel(const VoiceChatRemoveClientFromChannel & source);

	Archive::AutoVariable<NetworkId> m_clientId;
	Archive::AutoVariable<std::string> m_clientName;
	Archive::AutoVariable<std::string> m_channelName;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatGetChannel : public GameNetworkMessage
{
public:
	static const char* const cms_name;

	explicit VoiceChatGetChannel(std::string const & _roomName, std::string const & _password, bool _public, bool _persistant, uint32 _limit);
	explicit VoiceChatGetChannel(Archive::ReadIterator &);
	~VoiceChatGetChannel();

	std::string const & getRoomName() const;
	std::string const & getPassword() const;
	uint32 getLimit() const;
	bool getIsPublic() const;
	bool getIsPersistant() const;

private:
	VoiceChatGetChannel & operator = (const VoiceChatGetChannel & rhs);
	VoiceChatGetChannel(const VoiceChatGetChannel & source);

	Archive::AutoVariable<std::string> m_roomName;
	Archive::AutoVariable<std::string> m_password;
	Archive::AutoVariable<uint32> m_limit;
	Archive::AutoVariable<bool> m_isPublic;
	Archive::AutoVariable<bool> m_persistant;
};

//-----------------------------------------------------------------------

inline std::string const & VoiceChatGetChannel::getRoomName() const
{
	return m_roomName.get();
}

//-----------------------------------------------------------------------

inline std::string const & VoiceChatGetChannel::getPassword() const
{
	return m_password.get();
}

//-----------------------------------------------------------------------

inline uint32 VoiceChatGetChannel::getLimit() const
{
	return m_limit.get();
}

//-----------------------------------------------------------------------

inline bool VoiceChatGetChannel::getIsPublic() const
{
	return m_isPublic.get();
}

//-----------------------------------------------------------------------

inline bool VoiceChatGetChannel::getIsPersistant() const
{
	return m_persistant.get();
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatOnGetChannel : public GameNetworkMessage
{
public:
	static const char* const cms_name;

	explicit VoiceChatOnGetChannel(std::string const & _roomName, std::string const & _uri, std::string const & _password, bool _public, bool _persistant, bool _success);
	explicit VoiceChatOnGetChannel(Archive::ReadIterator &);
	~VoiceChatOnGetChannel();

	std::string const & getRoomName() const;
	std::string const & getUri() const;
	std::string const & getPassword() const;
	bool getIsPublic() const;
	bool getIsPersistant() const;
	bool getWasSuccessful() const;

private:
	VoiceChatOnGetChannel & operator = (const VoiceChatOnGetChannel & rhs);
	VoiceChatOnGetChannel(const VoiceChatOnGetChannel & source);

	Archive::AutoVariable<std::string> m_roomName;
	Archive::AutoVariable<std::string> m_uri;
	Archive::AutoVariable<std::string> m_password;
	Archive::AutoVariable<bool> m_isPublic;
	Archive::AutoVariable<bool> m_persistant;
	Archive::AutoVariable<bool> m_success;
};

//-----------------------------------------------------------------------

inline std::string const & VoiceChatOnGetChannel::getRoomName() const
{
	return m_roomName.get();
}

//-----------------------------------------------------------------------

inline std::string const & VoiceChatOnGetChannel::getUri() const
{
	return m_uri.get();
}

//-----------------------------------------------------------------------

inline std::string const & VoiceChatOnGetChannel::getPassword() const
{
	return m_password.get();
}

//-----------------------------------------------------------------------
inline bool VoiceChatOnGetChannel::getIsPublic() const
{
	return m_isPublic.get();
}

inline bool VoiceChatOnGetChannel::getIsPersistant() const
{
	return m_persistant.get();
}

inline bool VoiceChatOnGetChannel::getWasSuccessful() const
{
	return m_success.get();
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatDeleteChannel : public GameNetworkMessage
{
public:
	static const char* const cms_name;

	explicit VoiceChatDeleteChannel(std::string const & _roomName);
	explicit VoiceChatDeleteChannel(Archive::ReadIterator &);
	~VoiceChatDeleteChannel();

	std::string const & getRoomName() const { return m_roomName.get(); }

private:
	VoiceChatDeleteChannel & operator = (const VoiceChatDeleteChannel & rhs);
	VoiceChatDeleteChannel(const VoiceChatDeleteChannel & source);

	Archive::AutoVariable<std::string> m_roomName;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

enum VoiceChannelCommandType
{
	COMMAND_MUTE = 0,
	COMMAND_UNMUTE,
	COMMAND_KICK,
	COMMAND_HANGUP,
	COMMAND_INVITE,
	COMMAND_BAN,
	COMMAND_UNBAN,
	COMMAND_CHAN_LOCK,
	COMMAND_CHAN_UNLOCK,
	COMMAND_MUTE_ALL,
	COMMAND_UNMUTE_ALL,
	COMMAND_ADD_MODERATOR,
	COMMAND_DELETE_MODERATOR,
	COMMAND_ADD_ACL,
	COMMAND_DELETE_ACL,
	COMMAND_END
};

class VoiceChatChannelCommand : public GameNetworkMessage
{
public:
	static const char* const cms_name;

	explicit VoiceChatChannelCommand(std::string const & _srcUserName, std::string const & _destUserName, std::string const & _channelName, uint32 _command, uint32 _banTimeout);
	explicit VoiceChatChannelCommand(Archive::ReadIterator &);
	~VoiceChatChannelCommand();

	std::string const & getSourceUserName() const { return m_srcUserName.get(); }
	std::string const & getTargetUserName() const { return m_destUserName.get(); }
	std::string const & getChannelName() const { return m_channelName.get(); }
	uint32 getCommandType() const {return m_command.get(); }
	uint32 getBanTimeout() const {return m_banTimeout.get(); }

private:
	VoiceChatChannelCommand & operator = (const VoiceChatChannelCommand & rhs);
	VoiceChatChannelCommand(const VoiceChatChannelCommand & source);

	Archive::AutoVariable<std::string> m_srcUserName;
	Archive::AutoVariable<std::string> m_destUserName;
	Archive::AutoVariable<std::string> m_channelName;
	Archive::AutoVariable<uint32> m_command;
	Archive::AutoVariable<uint32> m_banTimeout;
};


class VoiceChatOnChannelCommand : public GameNetworkMessage
{
public:
	static const char* const cms_name;

	explicit VoiceChatOnChannelCommand(std::string const & _srcUserName, std::string const & _destUserName, std::string const & _channelName, uint32 _command, uint32 _banTimeout, uint32 _result);
	explicit VoiceChatOnChannelCommand(Archive::ReadIterator &);
	~VoiceChatOnChannelCommand();

	std::string const & getSourceUserName() const { return m_srcUserName.get(); }
	std::string const & getTargetUserName() const { return m_destUserName.get(); }
	std::string const & getChannelName() const { return m_channelName.get(); }
	uint32 getCommandType() const {return m_command.get(); }
	uint32 getBanTimeout() const {return m_banTimeout.get(); }
	uint32 getResult() const {return m_result.get(); }

private:
	VoiceChatOnChannelCommand & operator = (const VoiceChatOnChannelCommand & rhs);
	VoiceChatOnChannelCommand(const VoiceChatOnChannelCommand & source);

	Archive::AutoVariable<std::string> m_srcUserName;
	Archive::AutoVariable<std::string> m_destUserName;
	Archive::AutoVariable<std::string> m_channelName;
	Archive::AutoVariable<uint32> m_command;
	Archive::AutoVariable<uint32> m_banTimeout;
	Archive::AutoVariable<uint32> m_result;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatRequestPersonalChannel : public GameNetworkMessage
{
public:
	static const char * const cms_name;

	explicit VoiceChatRequestPersonalChannel (NetworkId const & owner, bool createIt);
	explicit VoiceChatRequestPersonalChannel (Archive::ReadIterator &);
	~ VoiceChatRequestPersonalChannel();

	inline NetworkId const & getOwner() {return m_owner.get();}
	inline bool getShouldCreate() {return m_pleaseCreateIt.get();}

private:
	VoiceChatRequestPersonalChannel & operator = (const VoiceChatRequestPersonalChannel & rhs);
	VoiceChatRequestPersonalChannel(const VoiceChatOnChannelCommand & source);

	Archive::AutoVariable<NetworkId> m_owner;
	Archive::AutoVariable<bool> m_pleaseCreateIt;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatInvite : public GameNetworkMessage
{
public:
	static const char * const cms_name;

	explicit VoiceChatInvite (NetworkId const & requester, std::string channelName, NetworkId const & inviteeId, std::string const & inviteeName);
	explicit VoiceChatInvite (NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName, std::string const & uri);
	explicit VoiceChatInvite (Archive::ReadIterator &);
	~VoiceChatInvite();

	inline NetworkId const & getRequester() const {return m_requester.get();}
	inline std::string const & getChannelName() const {return m_channelName.get();}
	inline NetworkId const & getInviteeId() const {return m_inviteeId.get();}
	inline std::string const & getInviteeName() const {return m_inviteeName.get();}
	inline std::string const & getChannelUri() const {return m_channelUri.get();}

	inline void setChannelUri(std::string const & uri) {m_channelUri.set(uri);}

private:
	VoiceChatInvite & operator = (const VoiceChatInvite & rhs);
	VoiceChatInvite (VoiceChatInvite const & source);

	Archive::AutoVariable<NetworkId> m_requester;
	Archive::AutoVariable<std::string> m_channelName;
	Archive::AutoVariable<NetworkId> m_inviteeId;
	Archive::AutoVariable<std::string> m_inviteeName;
	Archive::AutoVariable<std::string> m_channelUri;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatKick : public GameNetworkMessage
{
public:
	static const char * const cms_name;

	explicit VoiceChatKick (NetworkId const & requester, std::string channelName, NetworkId const & kickeeId, std::string const & kickeeName);
	explicit VoiceChatKick (NetworkId const & sourceId, NetworkId const & targetId, std::string channelName);
	explicit VoiceChatKick (Archive::ReadIterator &);
	~VoiceChatKick();

	inline NetworkId const & getRequester() const {return m_requester.get();}
	inline std::string const & getChannelName() const {return m_channelName.get();}
	inline NetworkId const & getKickeeId() const {return m_kickeeId.get();}
	inline std::string const & getKickeeName() const {return m_kickeeName.get();}

private:
	VoiceChatKick & operator = (const VoiceChatKick & rhs);
	VoiceChatKick (VoiceChatKick const & source);

	Archive::AutoVariable<NetworkId> m_requester;
	Archive::AutoVariable<std::string> m_channelName;
	Archive::AutoVariable<NetworkId> m_kickeeId;
	Archive::AutoVariable<std::string> m_kickeeName;
};


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatStatus : public GameNetworkMessage
{
public:
	static const char * const cms_name;

	enum StatusCode
	{
		SC_VoiceEnabled,
		SC_VoiceStatusUnknown,
		SC_VoiceServerDisabled,
		SC_VoiceClientIsTrial,
	};

	explicit VoiceChatStatus (const uint32 statusCode);
	explicit VoiceChatStatus (Archive::ReadIterator &);
	~VoiceChatStatus();

	inline uint32 getStatus() const { return m_status.get(); }

private:
	VoiceChatStatus & operator = (const VoiceChatStatus & rhs);
	VoiceChatStatus (VoiceChatStatus const & source);

	Archive::AutoVariable<uint32> m_status;
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

class VoiceChatRequestChannelInfo : public GameNetworkMessage
{
public:
	static const char * const cms_name;

	explicit VoiceChatRequestChannelInfo (NetworkId const & requester, std::string const & channelName);
	explicit VoiceChatRequestChannelInfo (Archive::ReadIterator &);
	~VoiceChatRequestChannelInfo();

	inline NetworkId const & getRequester() const {return m_requester.get();}
	inline std::string const & getChannelName() const {return m_channelName.get();}

private:
	VoiceChatRequestChannelInfo & operator = (const VoiceChatRequestChannelInfo & rhs);
	VoiceChatRequestChannelInfo (VoiceChatRequestChannelInfo const & source);

	Archive::AutoVariable<NetworkId> m_requester;
	Archive::AutoVariable<std::string> m_channelName;
};

#endif // _INCLUDED_VoiceChatMiscMessages_H
